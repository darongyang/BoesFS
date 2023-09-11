/*
 * This module has the kernel code for Boesfs
 */

#define KBUILD_MODNAME "boesfs"

#include <uapi/linux/bpf.h>
#include <linux/version.h>
#include <uapi/asm-generic/errno-base.h>
#include "acl_prog.h"


/******************  MODEL **************/
//  0 : request_definition ; 1: policy_definition ; 2: policy_effect ; 3: matchers
struct bpf_map_def SEC("maps") model_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(uint32_t),
	.value_size = sizeof(line_t),
	.max_entries = TOTAL_MODEL,
};
//  request_definition :  args[0]:第0个request参数 ; arg[1]:第1个request参数 ...
//  policy_definition : 同上
//  policy_effect : 0 / 1
//  e = some(where (p.eft == deny))		X
//  e = some(where (p.eft == allow ))	0
//  e = !some(where (p.eft == deny))	1
//  e = !some(where(p.eft == allow))	X
//  matchers :
//  0 : 1 2 3
//  1 : 1 2
//  2 : 1 3
//  3 : 2 3

/*************** Policy **************/
struct bpf_map_def SEC("maps") file_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = PATH_MAX_SUPP,
	.value_size = sizeof(policy_t),
	.max_entries = TOTAL_OBJ,
};

struct bpf_map_def SEC("maps") dir_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(uint32_t),
	.value_size = sizeof(dir_entry_t),
	.max_entries = TOTAL_OBJ,
};

struct bpf_map_def SEC("maps") args_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(uint32_t),
	.value_size = sizeof(uint32_t),
	.max_entries = TOTAL_RULE,
};


/**
 * @brief 检查传入的请求是否满足参数匹配
 * 
 * @param op 请求类型
 * @param obj_name 访问的客体
 * @param ctx 请求结构体
 * @return int 0:匹配成功, -1:匹配失败
 */
static int boesfs_args_verify(boesfs_op_t op, char * obj_name, struct boesfs_args *ctx){
	// 计算hash(op, obj_name)得到key
	uint32_t key = bpf_get_prandom_u32(ctx, OP_HASH); // 4

	DBG("op_hash :%x\n", bpf_get_prandom_u32(ctx, OP_HASH));

	key += bpf_get_prandom_u32(ctx, OBJ_HASH);

	DBG("obj_hash :%x\n", bpf_get_prandom_u32(ctx, OBJ_HASH));

	key = (uint32_t)HASH(key);

	DBG("hash_key (to search args_map):%x %u\n", key, key);

	// 利用key查args_map得到value
	uint32_t *value = bpf_map_lookup_elem(&args_map, &key);

	if (value == NULL)
	{
		DBG("Fail to search in args map\n");
		return 0;
	}
	else
	{
		DBG("Success to search in args map\n");
	}

	uint32_t value2 = 0;
	value2 = *value;

	DBG("hash_value (in args_map):%x\n", value2);

	// // 取value的后三位 args位图
	uint32_t args_bits = GETLOW3(value2);
	DBG("args_bits :%x\n", args_bits);
	uint32_t args_hash = 0; // 8


	// 字节码不做非法检查 由agent完成
	if(ARG1_VALID(args_bits)){
		args_hash += bpf_get_prandom_u32(ctx, ARG1_HASH);
		DBG("args1 compare! args1_hash:%x\n", bpf_get_prandom_u32(ctx, ARG1_HASH));
	}
		
	
	if(ARG2_VALID(args_bits)){
		args_hash += bpf_get_prandom_u32(ctx, ARG2_HASH);
		DBG("args2 compare! args2_hash:%x\n", bpf_get_prandom_u32(ctx, ARG2_HASH));
	}

	if(ARG3_VALID(args_bits)){
		args_hash += bpf_get_prandom_u32(ctx, ARG3_HASH);
		DBG("args3 compare! args3_hash:%x\n", bpf_get_prandom_u32(ctx, ARG3_HASH));
	}

	args_hash = (uint32_t)HASH(args_hash);
	DBG("args_hash: %x\n", args_hash);

	// 取前29位
	args_hash = GETHIGH29(args_hash);
	DBG("args_hash_high29: %x\n", args_hash);

	// // 判断是否通过
	if(args_hash == GETHIGH29(value2))
	{
		DBG("value in args_map hit hit\n");
		return 0;
	}
	else
	{
		DBG("value in args_map not hit\n");
		return -1;
	}
	
}

//  规则命中时
//  p : 查找到的规则
//  op : 要进行判断的操作
static int result_hit(policy_t *p, boesfs_op_t op, policy_effect_t effect_model)
{
	if (effect_model == EFFECT_ALLOW)
	{
		if (ISVALID(p->allow, op))
		{
			DBG("%d is allowed\n", op);
			return 0;
		}
		else
		{
			DBG("%d is denied\n", op);
			return -1;
		}
	}
	else
	{
		if (ISVALID(p->deny, op))
		{
			DBG("%d is denied\n", op);
			return -1;
		}
		else
		{
			DBG("%d is allowed\n", op);
			return 0;
		}
	}
}

//  规则未命中时
static int result_missed(policy_effect_t effect_model)
{
	return effect_model == EFFECT_DENY ? 0 : -1;
}


static int matcher_sub_obj_act_args(boesfs_op_t op, char *obj_name, policy_effect_t effect_model,struct boesfs_args *ctx, int model)
{
	int arg_check = 0;
	// 参数检查
	if(model == OBJ_ACT_ARGS || model == SUB_OBJ_ACT_ARGS)
	{
		arg_check = -1;
		/* 先查args_map */
		arg_check = boesfs_args_verify(op, obj_name, ctx); // 参数匹配结果
	}
	else
	{
		arg_check = 0;
	}

	// 参数匹配没通过
	if(arg_check == -1) 
	{
		return result_missed(effect_model); // 规则一定失效
	} 
	else
	{
	// 参数匹配通过
		if (effect_model == EFFECT_ALLOW || effect_model == EFFECT_DENY)
		{
			//  search file map
			policy_t *p = bpf_map_lookup_elem(&file_map, obj_name);
			if (p != NULL)
			{
				DBG("%s found in file map\n", obj_name);
				return result_hit(p, op, effect_model); // 规则命中
			}
			else
			{
				DBG("%s not found in file map\n", obj_name);
				//  search dir tree
				policy_t rule = {			//  must on ebpf stack
					.valid = 0,
					.allow = 0,
					.deny = 0,
				};
				
				int ret = bpf_boesfs_acl_dfs_path(ctx, obj_name, &rule, 0);
				
				if(ret < 0)					//  bpf_boesfs_acl_dfs_path failed
					goto err;
							
				if (rule.valid != 1)		//  node not found or found but not valid
				{
					DBG("%s not found in dir map valid is %d\n", obj_name,rule.valid);
					return result_missed(effect_model);
				}
				else
				{
					DBG("%s found in dir map allow = %d , deny = %d\n", obj_name, rule.allow, rule.deny);
					return result_hit(&rule, op, effect_model);
				}
			}
		}
		else
		{
			DBG("matcher_sub_obj_act never reach!\n");
		}

	err:
		return -1;
	}	
}

//  matcher 1 : 12 . subject_name , object_name
static int matcher_sub_obj(boesfs_op_t op, char *obj_name, policy_effect_t effect_model,struct boesfs_args *ctx)
{
	// DBG("matcher_sub_obj %s %d\n", obj_name, op);
	policy_t *p = bpf_map_lookup_elem(&file_map, (void *)obj_name);
	if (p != NULL)
	{
		//  DBG("%s found in file map\n", obj_name);
		//  result_hit
		return effect_model == EFFECT_DENY ? -1 : 0;
	}
	else
	{
		//  search dir tree
		policy_t rule = {			//  must on ebpf stack
			.valid = 0,
			.allow = 0,
			.deny = 0,
		};
			
		int ret = bpf_boesfs_acl_dfs_path(ctx, obj_name, &rule, 1);
		if(ret < 0)
			goto err;
		
		if (rule.valid != 1)
		{
			// DBG("%s not found in dir map\n", obj_name);
			return result_missed(effect_model);
		}
		else
		{
			//  DBG("%s found in dir map\n", obj_name);
			//  result hit
			return effect_model == EFFECT_DENY ? -1 : 0;
		}
	}

err:
	return -1;
}

//  matcher2
//  针对subject, 查找操作13
static int matcher_sub_act(boesfs_op_t op, policy_effect_t effect_model,struct boesfs_args *ctx)
{
	// DBG("matcher_sub_act %d\n", op);
	//  查找subject的权限bitmap
	int idx = SUB_ACT_RULE_IDX;
	struct line *p = bpf_map_lookup_elem(&model_map, &idx);
	if (p == NULL)
	{
		return result_missed(effect_model);
	}
	else
	{
		if (effect_model == EFFECT_ALLOW)
		{
			if (ISVALID(p->args[0], op))
				return 0;
			else
				return -1;
		}
		else
		{
			if (ISVALID(p->args[1], op))
				return -1;
			else
				return 0;
		}
	}
}

//  init handlers and model var
static int init_args(struct boesfs_args *ctx, matcher_t *match_model, policy_effect_t *effect_model)
{
	//  init matcher
	model_map_idx_t idx = MATCHER_IDX;
	line_t *line_matcher = bpf_map_lookup_elem(&model_map, (void *)&idx);
	if (line_matcher == NULL) {
		//  程序未检查 map_lookup_elem() 的返回值是否为空就开始使用 : R0 invalid mem access 'map_value_or_null'
		return -1;
	}
	*match_model = line_matcher->args[0];

	//  init policy effect
	idx = POL_EFF_IDX;
	line_t *line_effect = bpf_map_lookup_elem(&model_map, (void *)&idx);
	if (line_effect == NULL){ 
		return -1;
	}
	*effect_model = line_effect->args[0];

	return 0;	
}

//  初始化整棵dir tree
static int init_dir_tree(struct boesfs_args *ctx)
{
	int ret = 0;
	if(ctx->dir_tree_root != NULL){
		DBG("ctx->dir_tree_root should be NULL before init %p!\n",ctx->dir_tree_root);
		ret = -1;
		goto err;
	}

	int idx = DIR_ENTRY_IDX;
	line_t *dir_tree_meta = bpf_map_lookup_elem(&model_map,(void*)&idx);
	if(dir_tree_meta == NULL){
		DBG("DIR_ENTRY_IDX isn't filled!\n");
		ret = -1;
		goto err;
	}

	//  number of dir rule 
	DBG("dir nums : %d\n",dir_tree_meta->args[0]);

	// ret = bpf_boesfs_build_dir_tree(ctx,dir_tree_meta);
	ret = bpf_get_smp_processor_id(ctx,dir_tree_meta,&dir_map);
	if(ret < 0){
		DBG("bpf_get_smp_processor_id failed! , ret = %d\n",ret);
		goto err;
	} else {
		DBG("succeed to init dir tree ret = %d!\n",ret);
	}

out:
	return ret;

err:
	return ret;
}


static int destory_dir_tree(struct boesfs_args *ctx) {
	int ret = bpf_get_current_comm(ctx);
	if(ret < 0)
		goto err;
out:
	return ret;
err:
	DBG("failed to destroy dir tree!\n");
	return ret;
}

int SEC("boesfs") boesfs_handler(struct boesfs_args *ctx)
{

	int op = ctx->op;
	if(op == BOESFS_INIT) {
		return init_dir_tree(ctx);
	} else if (op == BOESFS_DESTROY) {
		return destory_dir_tree(ctx);		
	} 
	else {
		matcher_t match_model;
		policy_effect_t effect_model;
		if (init_args(ctx, &match_model, &effect_model) == -1){
			return -1;
		}
		struct open_boesfs_args arg = {
			.path = {0}
		};
		bpf_boesfs_read_args(BOESFS_OPEN, &arg, ctx);

		// DBG("boesfs handler match_model = %d , effect_model = %d!\n", match_model, effect_model);

		switch (match_model)
		{
		case OBJ_ACT:
		case SUB_OBJ_ACT:
		case OBJ_ACT_ARGS:
		case SUB_OBJ_ACT_ARGS:
			return matcher_sub_obj_act_args(op, arg.path, effect_model, ctx, match_model);
		case SUB_OBJ:
			return matcher_sub_obj(op, arg.path, effect_model,ctx);
		case SUB_ACT:
			return matcher_sub_act(op, effect_model,ctx);
		default:
			return -1;
		}
	}

}

char _license[] SEC("license") = "GPL";
uint32_t _version SEC("version") = LINUX_VERSION_CODE;
