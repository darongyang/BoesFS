//  黑白名单混用
	//  policy只有一个bitmap
	//  0: deny ; 1: allow
//  原先使用黑白名单的主要原因之一在于为了在未命中时给出相应默认动作. 
//  现在将默认动作交由用户定义
	//  措施如下
	//  原先存放 effect_model的位置 变成了 default_action
	//  effect_allow : 默认放行
	//  effect_deny : 默认禁止

/*
 * This module has the kernel code for Boesfs
 */

#define KBUILD_MODNAME "boesfs"

#include <uapi/linux/bpf.h>
#include <linux/version.h>
#include <uapi/asm-generic/errno-base.h>
#include "acl_prog_new.h"

#define TOTAL_MODEL 5
#define TOTAL_RULE 32

/******************  MODEL **************/
//  0 : request_definition ; 1: policy_definition ; 2: policy_effect ; 3: matchers
struct bpf_map_def SEC("maps") model_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(line_t),
	.max_entries = TOTAL_MODEL,
};
//  request_definition :  args[0]:第0个request参数 ; arg[1]:第1个request参数 ...
//  policy_definition : 同上
//  policy_effect : 0 / 1
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
	.max_entries = TOTAL_RULE,
};

struct bpf_map_def SEC("maps") dir_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = PATH_MAX_SUPP,
	.value_size = sizeof(policy_t),
	.max_entries = TOTAL_RULE,
};

static int result_missed(default_action_t default_action)
{
	return default_action == EFFECT_ALLOW ? 0 : -1;
}

//  p : 查找到的规则
// op : 要进行判断的操作
static int result_hit(policy_t *p, boesfs_op_t op)
{
	if (ISVALID(p->rule_map, op))
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

//  传入key : object name. 客体名称 , 在policy中查找是否存在该object的规则
//  p, bob, data2, write
//  深度优先原则
//     - 白名单规则判断
//         - 规则命中：观察其allow，1，则放行；0，则拦截
//         - 规则不命中：取决于default_action
//     - 流程如下
//     - 查找file map，找到，规则命中。
//         - 进行白名单判断
//     - 查找file map，没找到，规则可能未命中
//         - 查找dir map：在dir map中由深到浅查找path的所有父目录（遵循深度优先原则）
//             - 如果找到，则规则命中。放行 or 拦截。
//             - 如果没找到，则规则未命中，default_action。
static int matcher_sub_obj_act(boesfs_op_t op, char *obj_name, default_action_t default_action)
{
	// DBG("matcher_sub_obj_act %s %d\n", obj_name, op);
	policy_t *p = bpf_map_lookup_elem(&file_map, obj_name);
	if (p != NULL)
	{
		// DBG("%s found in file map\n", obj_name);
		return result_hit(p, op);
	}
	else
	{
		p = bpf_boesfs_acl_dfs_path(&dir_map, obj_name);
		if (p == NULL)
		{
			// DBG("%s not found in dir map\n", obj_name);
			return result_missed(default_action);
		}
		else
		{
			// DBG("%s found in dir map allow = %d , deny = %d\n", obj_name, p->allow, p->deny);
			return result_hit(p, op);
		}
	}
	return -1;
}

//  matcher 1 : 12 . subject_name , object_name
static int matcher_sub_obj(boesfs_op_t op, const char *obj_name, int default_action)
{
	// DBG("matcher_sub_obj %s %d\n", obj_name, op);
	policy_t *p = bpf_map_lookup_elem(&file_map, (void *)obj_name);
	if (p != NULL)
	{
		// DBG("%s found in file map\n", obj_name);
		return result_hit(p,op);
	}
	else
	{
		p = bpf_boesfs_acl_dfs_path(&dir_map, (void*)obj_name);
		if (p == NULL)
		{
			// DBG("%s not found in dir map\n", obj_name);
			return result_missed(default_action);
		}
		else
		{
			// DBG("%s found in dir map\n", obj_name);
			return result_hit(p,op);
		}
	}
}

//  matcher2
//  针对subject, 查找操作13
static int matcher_sub_act(boesfs_op_t op, int effect_model)
{
	// DBG("matcher_sub_act %d\n", op);
	//  查找subject的权限bitmap
	int idx = EXTRA_IDX;
	policy_t *p = bpf_map_lookup_elem(&model_map, &idx);
	if (p == NULL)
	{
		return effect_model == EFFECT_DENY ? 0 : -1;
	}
	else
	{
		return result_hit(p,op);
	}
}

//  init handlers and model var
static int init(struct boesfs_args *ctx, matcher_t *match_model, default_action_t *default_action)
{
	//  init matcher
	model_map_idx_t idx = MATCHER_IDX;
	line_t *line_matcher = bpf_map_lookup_elem(&model_map, (void *)&idx);
	if (line_matcher == NULL)
	{
		return -1;
	}
	*match_model = line_matcher->args[0];

	//  init policy effect
	idx = POL_EFF_IDX;
	line_t *line_effect = bpf_map_lookup_elem(&model_map, (void *)&idx);
	if (line_effect == NULL)
	{ //  程序未检查 map_lookup_elem() 的返回值是否为空就开始使用 : R0 invalid mem access 'map_value_or_null'
		return -1;
	}
	*default_action = line_effect->args[0];

	return 0;
}

int SEC("boesfs") boesfs_handler(struct boesfs_args *ctx)
{
	boesfs_op_t op = ctx->op;
	matcher_t match_model;
	default_action_t default_action;
	if (init(ctx, &match_model, &default_action) == -1)
	{
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
		return matcher_sub_obj_act(op, arg.path, default_action);
	case SUB_OBJ:
		return matcher_sub_obj(op, arg.path, default_action);
	case SUB_ACT:
		return matcher_sub_act(op, default_action);
	default:
		return -1;
	}
	return -1;
}

char _license[] SEC("license") = "GPL";
uint32_t _version SEC("version") = LINUX_VERSION_CODE;
