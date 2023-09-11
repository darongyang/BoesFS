#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/bpf.h>
#include <linux/version.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <pwd.h>


#include <ebpf.h>
#include <libbpf.h>
#include <bpf_load.h>
#include <acl_public.h>
#include <utils.h>


//#define DEBUG

#ifdef DEBUG
#define DBG(fmt, ...)   fprintf(stdout, fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif
#define ERROR(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

ebpf_ctxtext_t* ebpf_init(char *filename, int index)
{
	int i;
    ebpf_ctxtext_t *ctx;

	// // 原用户态程序中用于扩大子进程需要的资源的代码，未知作用，先注释
    // struct rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
	// uid_t uid = getuid();
	// if (!uid)
	// 	setrlimit(RLIMIT_MEMLOCK, &r)
	// 分配ebpf_ctxtext_t结构体的空间
	ctx = (ebpf_ctxtext_t* ) calloc(1, sizeof(ebpf_ctxtext_t));
	if (!ctx)
		goto err;
	//调用bpf_load里的函数加载bpf elf文件
    if (load_bpf_file(filename))
		goto err;
	//判断是否加载完
    if (!prog_fd[index] || !map_fd[0])
		goto err;
	//将加载后的文件描述符吸入结构体中
	ctx->ctrl_fd = prog_fd[index];
	for (i = 0; i < MAX_MAPS; i++)
		ctx->data_fd[i] = map_fd[i];
	
    return ctx;

err:
	if (ctx)
		free(ctx);
	return NULL;
}

void ebpf_fini(ebpf_ctxtext_t *ctx)
{
	int i;
	if (ctx->ctrl_fd)
		close(ctx->ctrl_fd);
	for (i = 0; i < MAX_MAPS; i++)
		if (ctx->data_fd[i])
			close(ctx->data_fd[i]);
	free(ctx);
	return;
}

int lookup_dir_map(ebpf_ctxtext_t *ctx, int count, char *key)
{
	int index, result;
	dir_entry_t dir;
	for(index = 0; index < count; index++)
	{
		result = ebpf_data_lookup(ctx, (void *)&index, (void *)&dir, 2);
		if(!result && !strcmp(key, dir.path))
			return index;
		
	}
	return -1;
}


int read_acl_rbac(ebpf_ctxtext_t *ctx, char *model_file, char *policy_file, char *err, char *elf)
{
	char buf[512];
	FILE *f;
	line_t r_def, p_def, p_eff, matchers;
	char *p, *tmp;
	int p_def_type = 0, match_type = 0;
	int is_rbac_sub = 0, is_rbac_act_args = 0, is_args = 0;
	int model_key, map_idx, r_def_num = 0, p_def_num = 0;
	memset(&p_def_type, 0, sizeof(int));
	memset(&r_def, 0, sizeof(line_t));
	memset(&p_def, 0, sizeof(line_t));
	memset(&p_eff, 0, sizeof(line_t));
	memset(&matchers, 0, sizeof(line_t));
	//储存规则
	map_idx = 0;
	// 打开规则文件
	f = fopen(model_file,"r");
	if(!f)
	{
		snprintf(err, PATH_MAX_SUPP, "open %s error\n\n", model_file);
		return 1;
	}
	// 判断请求定义格式是否正确
	fgets(buf, 512, f);
	if(memcmp(buf, "[request_definition]", 20))
	{
		strcpy(err, "no [request_definition]\n");
		return 1;
	}
	fgets(buf, 512, f);
	str_handle(buf);
	if(memcmp(buf, "r=", 2))
	{
		strcpy(err, "no \"r=\"\n");
		return 1;
	}
	// 读取请求定义
	tmp = buf + 2;
	p = strtok(tmp, ",");
	while(p)
	{
		if(strlen(p) > 4)
		{
			strcpy(err, "too long def(r_def)");
			return 1;
		}
		memcpy(&r_def.args[r_def_num], p, strlen(p));
		r_def_num ++;
		p = strtok(NULL, ",");
	}
	r_def.nums = r_def_num;
	// 写入到map中
	model_key = REQ_DEF_IDX;
	if (ebpf_data_update(ctx, (void *)&model_key, (void *)&r_def, map_idx, 1))
	{
		strcpy(err, "failed to ebpf_data_update request definition");
		return 1;
	}
	// 判断策略定义格式是否正确
	fgets(buf, 3, f);
	fgets(buf, 512, f);
	if(memcmp(buf, "[policy_definition]", 19))
	{
		strcpy(err, "no [policy_definition]");
		return 1;
	}
	fgets(buf, 512, f);
	str_handle(buf);
	if(memcmp(buf, "p=", 2))
	{
		strcpy(err, "no p=");
		return 1;
	}
	// 读取策略定义
	tmp = buf + 2;
	p = strtok(tmp, ",");
	while(p)
	{
		if(strlen(p) > 4)
		{
			strcpy(err, "too long def(p_def)");
			return 1;
		}
		memcpy(&p_def.args[p_def_num], p, strlen(p));
		p_def_num ++;
		if(!strcmp(p, "sub"))
			p_def_type |= 1;
		else if(!strcmp(p, "obj"))
			p_def_type |= 2;
		else if(!strcmp(p, "act"))
			p_def_type |= 4;
		else if(!strcmp(p, "args"))
			p_def_type |= 8;
		else
		{
			strcpy(err, "unknown policy defined");
			return 1;
		}
		p = strtok(NULL,",");
	}
	p_def.nums = p_def_num;
	p_def.args[0] = p_def_type;
	if(p_def_num < 2)
	{
		strcpy(err, "unknown policy defined");
		return 1;
	}
	// 写入到map中
	model_key = POL_DEF_IDX;
	if (ebpf_data_update(ctx, (void *)&model_key, (void *)&p_def, map_idx, 1))
	{
		strcpy(err, "failed to ebpf_data_update policy definition");
		return 1;
	}
	// 判断策略结果格式是否正确
	fgets(buf, 3, f);
	fgets(buf, 512, f);
	if(!memcmp(buf, "[role_definition]", 17))
	{
		fgets(buf, 512, f);
		str_handle(buf);
		if(!memcmp(buf, "g=_,_", 5))
		{
			is_rbac_sub = 1;
			fgets(buf, 512, f);
			str_handle(buf);
		}
		else if(!memcmp(buf, "a=_,_;_,_", 9))
		{
			is_rbac_act_args = 1;
			fgets(buf, 512, f);
			str_handle(buf);
		}
		else if(strlen(buf) != 0)
		{
			strcpy(err, "[role_definition] error");
			return 1;
		}
		if(is_rbac_act_args && !is_rbac_sub && !memcmp(buf, "g=_,_", 5))
		{
			is_rbac_sub = 1;
			fgets(buf, 512, f);
			str_handle(buf);
		}
		else if(is_rbac_sub && !is_rbac_act_args && !memcmp(buf, "a=_,_;_,_", 9))
		{
			is_rbac_act_args = 1;
			fgets(buf, 512, f);
			str_handle(buf);
		}
		if(strlen(buf) != 0)
		{
			strcpy(err, "[role_definition] error");
			return 1;
		}
		fgets(buf, 512, f);
	}
	if(memcmp(buf, "[policy_effect]", 15))
	{
		strcpy(err, "no [policy_effect]");
		return 1;
	}
	// 读取策略结果
	fgets(buf, 512, f);
	str_handle(buf);
	if(!memcmp(buf, "e=some(where(p.eft==allow))", 27))
		p_eff.args[0] = EFFECT_ALLOW;
	else if(!memcmp(buf, "e=!some(where(p.eft==deny))", 27))
		p_eff.args[0] = EFFECT_DENY;
	else
	{
		strcpy(err, "p_eff undifined");
		return 1;
	}
	p_eff.nums = 1;
	// 写入到map中
	model_key = POL_EFF_IDX;
	if (ebpf_data_update(ctx, (void *)&model_key, (void *)&p_eff, map_idx, 1))
	{
		strcpy(err, "failed to ebpf_data_update policy effect");
		return 1;
	}
	// 判断匹配逻辑格式是否正确
	fgets(buf, 3, f);
	fgets(buf, 512, f);
	if(memcmp(buf, "[matchers]", 10))
	{
		strcpy(err, "no [matchers]");
		return 1;
	}
	// 读取匹配逻辑
	
	fgets(buf, 512, f);
	str_handle(buf);
	if(memcmp(buf, "m=", 2))
	{
		strcpy(err, "no m=");
		return 1;
	}
	if((p_def_type & 1) && strstr(buf, "r.sub==p.sub"))
		match_type |= 1;
	if((p_def_type & 1) && strstr(buf, "g(r.sub,p.sub)") && is_rbac_sub)
	{
		if(match_type & 1)
			match_type |= 16;
		match_type |= 1;
		is_rbac_sub = 2;
	}
	if((p_def_type & 2) && strstr(buf, "r.obj==p.obj"))
		match_type |= 2;
	if((p_def_type & 4) && strstr(buf, "r.act==p.act"))
		match_type |= 4;
	if((p_def_type & 8) && strstr(buf, "r.args==p.args"))
	{
		match_type |= 8;
		is_args = 1;
	}
	if((p_def_type & 4) && (p_def_type & 8) && is_rbac_act_args &&
		 strstr(buf, "a(r.act,p.act;r.args,p.args)"))
	{
		if(match_type & 4 || match_type & 8)
			match_type |= 16;
		match_type |= 12;
		is_args = 1;
		is_rbac_act_args = 2;
	}
	switch (match_type)
	{
		case 3:		matchers.args[0] = SUB_OBJ; break;
		case 5:		matchers.args[0] = SUB_ACT; break;
		case 6:		matchers.args[0] = OBJ_ACT; break;
		case 7:		matchers.args[0] = SUB_OBJ_ACT; break;
		case 14:	matchers.args[0] = OBJ_ACT_ARGS; break;
		case 15:	matchers.args[0] = SUB_OBJ_ACT_ARGS; break;
		default:	strcpy(err, "matchers undifined"); return 1;
	}
	matchers.nums = 1;
	// 写入到map中
	model_key = MATCHER_IDX;
	if (ebpf_data_update(ctx, (void *)&model_key, (void *)&matchers, map_idx, 1))
	{
		strcpy(err, "failed to ebpf_data_update policy effect");
		return 1;
	}
	// 关闭策略文件
	fclose(f);
	//储存策略

	/***
	 * 从这里开始读取策略文件
	*/

	model_key = SUB_ACT_RULE_IDX;
	int dir_count = 0, dir_idx;
	// 打开策略文件
	f = fopen(policy_file,"r");
	int free_error = 0;
	if(!f)
	{
		snprintf(err, PATH_MAX_SUPP, "open %s error\n\n", policy_file);
		return 1;
	}
	sub_list *sub_head = (sub_list *)malloc(sizeof(sub_list));
	act_args_list *aa_head = (act_args_list *)malloc(sizeof(act_args_list));
	sub_head->next = NULL;
	aa_head->next = NULL;
	if(is_rbac_sub == 2)
	{
		while (fgets(buf, 512, f) != NULL)
		{
			str_handle(buf);
			if(memcmp(buf, "g,", 2))
				continue;
			p = strtok(buf, ",");
			p = strtok(NULL, ",");
			char tmp_p[PATH_MAX_SUPP] = {0};
			if(get_filename(p, tmp_p))
			{
				snprintf(err, PATH_MAX_SUPP, "failed to get filename in acl\n\n");
				free_error = 1;
				goto free_head;
			}
			if(!strcmp(tmp_p, elf))
			{
				p = strtok(NULL, ",");
				sub_list *tmp_sub = (sub_list *)malloc(sizeof(sub_list));
				strncpy(tmp_sub->role, p, 30);
				tmp_sub->next = sub_head->next;
				sub_head->next = tmp_sub;
			}
		}
		/***
		 * 将符合现文件名的角色添加到链表中
		 * 更改读取位置到文件头
		*/
		fseek(f, 0, SEEK_SET);
	}
	
	if(is_rbac_act_args == 2)
	{
		while (fgets(buf, 512, f) != NULL)
		{
			str_handle(buf);
			if(memcmp(buf, "a,", 2))
				continue;
			p = strtok(buf, ",");
			p = strtok(NULL, ",");
			if(strlen(p) > 30)
				continue;
			act_args_list *tmp_aa = (act_args_list *)malloc(sizeof(act_args_list));
			strcpy(tmp_aa->role, p);
			p = strtok(NULL, ",");
			if(strlen(p) > 8)
			{
				free(tmp_aa);
				snprintf(err, PATH_MAX_SUPP, "%s check unsupported\n\n", p);
				free_error = 1;
				goto free_head;
			}
			int op = check_op(p);
			if(op == -1)
			{
				free(tmp_aa);
				snprintf(err, PATH_MAX_SUPP, "%s check unsupported\n\n", p);
				free_error = 1;
				goto free_head;
			}
			uint32_t count = 0, val_hash = 0;
			p = p + strlen(p) + 1;
			if(*(p + strlen(p) - 1) != ')' || *p != '(')
			{
				free(tmp_aa);
				snprintf(err, PATH_MAX_SUPP, "args error:%s\n\n", p);
				free_error = 1;
				goto free_head;
			}
			*(p + strlen(p) - 1) = '\0';
			p = p + 1;
			p = strtok(p, ",");
			while(p)
			{
				if(count >= 3)
					break;
				if(*p == '*')
				{
					count += 1;
					p = strtok(NULL, ",");
					continue;
				}
				tmp_aa->args_hash |= 1 << count ;
				count += 1;
				if(strspn(p, "0123456789") == strlen(p)){
					val_hash += hash_uint32(s2d(p));
				}
				else
					val_hash += hash_str(p);
				p = strtok(NULL, ",");
			}
			if(tmp_aa->args_hash != 0)
				tmp_aa->args_hash |= hash_uint32(val_hash) & 0xFFFFFFF8;
			tmp_aa->op = op;
			tmp_aa->next = aa_head->next;
			aa_head->next = tmp_aa;
		}
		/***
		 * 将符合现用户名的角色添加到链表中
		 * 这里暂时通过用户名来实现rbac，改成文件名也很方便
		 * 更改读取位置到文件头
		*/
		fseek(f, 0, SEEK_SET);
	}
	// 读取策略
	while(fgets(buf, 512, f) != NULL)
	{
		int result;
		str_handle(buf);
		if(memcmp(buf, "p,", 2))
			continue;
		char policy_key[PATH_MAX_SUPP];
		void *actual_key = NULL;
		void *actual_val_head = NULL;
		void *actual_val = NULL;
		void *tmp_val_head = NULL;
		void *tmp_val = NULL;
		policy_t p_val;
		line_t p_val_l;
		dir_entry_t p_dir_val;
		uint32_t args_key = 0, args_val = 0;
		uint32_t args_op_val = 0;
		memset(&p_val, 0, sizeof(policy_t));
		memset(&p_val_l, 0, sizeof(line_t));
		memset(policy_key, 0, PATH_MAX_SUPP);
		memset(&p_dir_val, 0, sizeof(dir_entry_t));
		p_val.valid = 1;
		p = strtok(buf, ",");
		p = strtok(NULL, ",");
		if(p_def_type & 2 && p_def_type & 1) 
		{
			// sub, obj都有的时候比较sub与要执行的程序名，有则写入该规则
			if(matchers.args[0] == SUB_OBJ_ACT ||
				matchers.args[0] == SUB_OBJ ||
				matchers.args[0] == SUB_OBJ_ACT_ARGS)
			{
				if(is_rbac_sub == 2)
				{
					/***
					 * 对比角色是否在链表中
					*/
					sub_list *tmp_sub = sub_head;
					int flag = 0;
					while(tmp_sub->next)
					{
						tmp_sub = tmp_sub->next;
						if(!strcmp(p, tmp_sub->role))
						{
							flag = 1;
							break;
						}
					}
					if(flag)
					{
						p = strtok(NULL, ",");
						strcpy(policy_key, p);
					}
					else
						continue;
				}
				else
				{
					char tmp_p[PATH_MAX_SUPP] = {0};
					if(get_filename(p, tmp_p))
					{
						snprintf(err, PATH_MAX_SUPP, "failed to get filename in acl\n\n");
						free_error = 1;
						goto free_head;
					}
					if(!strcmp(tmp_p, elf))
					{
						p = strtok(NULL, ",");
						strcpy(policy_key, p);
					}
					else
						continue;
				}
			}
			else
			{
				p = strtok(NULL, ",");
				strcpy(policy_key, p);
			}
		}
		else
			// sub, obj只有一个的情况下将其作为key
			strcpy(policy_key, p);
		if(strlen(policy_key) != 1 && policy_key[strlen(policy_key) - 1] == '/')
			policy_key[strlen(policy_key) - 1] = '\0';
		p = strtok(NULL, ",");
		// 在有act的情况下判断操作类型
		if(p_def_type & 4 && matchers.args[0] != SUB_OBJ)
		{
			int op;
			if(is_rbac_act_args == 2)
			{
				if(strlen(p) > 30)
					continue;
				act_args_list *tmp_aa = aa_head;
				int flag = 0;
				while(tmp_aa->next)
				{
					tmp_aa = tmp_aa->next;
					if(!strcmp(p, tmp_aa->role))
					{
						args_op_val = SETVALID(args_op_val, tmp_aa->op);
						if(tmp_aa->args_hash == 0)
							continue;
						flag = 1;
						args_key = hash_uint32(hash_uint32(tmp_aa->op) + hash_str(policy_key));
						args_val = tmp_aa->args_hash;
						map_idx = 3;
						ebpf_data_update(ctx, (void *)&args_key, (void *)&args_val, map_idx, 1);
					}
				}
				if(!flag)
					continue;
				p = p + strlen(p) + 1;
			}
			else
			{
				if(strlen(p) > 8)
					continue;
				op = check_op(p);
				if(op == -1)
				{
					snprintf(err, PATH_MAX_SUPP, "%s check unsupported\n\n", p);
					free_error = 1;
					goto free_head;
				}
				if(is_args)
				{
					uint32_t args_count = 0;
					uint32_t args_val_hash = 0;
					char tmp_args[PATH_MAX_SUPP] = {0};
					char *args_p = NULL, *end_p = NULL;
					args_key = hash_uint32(hash_uint32(op) + hash_str(policy_key));
					end_p = strstr(p + strlen(p) + 1, ")");
					if(!end_p)
					{
						snprintf(err, PATH_MAX_SUPP, "args invalid: %s\n\n", p + strlen(p) + 1);
						free_error = 1;
						goto free_head;
					}
					p = p + strlen(p) + 1;
					if(*p != '(')
					{
						snprintf(err, PATH_MAX_SUPP, "args check unsupported: %s\n\n", p);
						free_error = 1;
						goto free_head;
					}
					strcpy(tmp_args, p + 1);
					args_p = strtok(tmp_args, ")");
					args_p = strtok(args_p, ",");
					while(args_p)
					{
						if(args_count >= 3)
						{
							snprintf(err, PATH_MAX_SUPP, "too many args: %s\n\n", p);
							free_error = 1;
							goto free_head;
						}
						if(*args_p == '*')
						{
							args_count += 1;
							args_p = strtok(NULL, ",");
							continue;
						}
						args_val |= 1 << args_count ;
						args_count += 1;
						if(strspn(args_p, "0123456789") == strlen(args_p)){
							args_val_hash += hash_uint32(s2d(args_p));
						}
						else
							args_val_hash += hash_str(args_p);
						args_p = strtok(NULL, ",");
					}
					if(args_val != 0)
					{
						args_val |= hash_uint32(args_val_hash) & 0xFFFFFFF8;
						map_idx = 3;
						ebpf_data_update(ctx, (void *)&args_key, (void *)&args_val, map_idx, 1);
					}
					p = end_p + 2;
				}
				else
					p = p + strlen(p) + 1;
			}
			// 判断是文件限制还是目录限制
			p = strtok(p, ",");
			if(matchers.args[0] == SUB_ACT)
			{
				map_idx = 0;
				actual_key = (void *)&model_key;
				actual_val_head = (void *)&p_val_l;
				actual_val = (void *)&(p_val_l.args);
				tmp_val_head = malloc(sizeof(line_t));
				tmp_val = (void *) (((line_t *)tmp_val_head)->args);
			}
			else
			{
				if(!memcmp(p, "file", 4))
				{
					map_idx = 1;
					actual_key = (void *)policy_key;
					actual_val_head = (void *)&p_val;
					actual_val = (void *)&p_val;
					tmp_val_head = malloc(sizeof(policy_t));
					tmp_val = tmp_val_head;
				}
				else if(!memcmp(p, "dir", 3))
				{
					map_idx = 2;
					actual_val_head = (void *)&p_dir_val;
					actual_val = (void *)&(p_dir_val.rule);
					strcpy(p_dir_val.path, policy_key);
					p_dir_val.rule.valid = 1;
					dir_idx = lookup_dir_map(ctx, dir_count, policy_key);
					if(dir_idx == -1)
					{
						dir_idx = dir_count;
						dir_count++;
					}
					actual_key = (void *)&dir_idx;
					tmp_val_head = malloc(sizeof(dir_entry_t));
					tmp_val = (void *) &(((dir_entry_t *)tmp_val_head)->rule);
				}
				else
				{
					snprintf(err, PATH_MAX_SUPP, "%s check unsupported\n\n", p);
					free_error = 1;
					goto free_head;
				}
			}
			p = strtok(NULL, ",");
			// 读取map查看是否已有相应的规则
			result = ebpf_data_lookup(ctx, actual_key, tmp_val_head, map_idx);
			if(p_eff.args[0] == EFFECT_ALLOW && !memcmp(p, "allow", 5))
			{
				uint32_t tmp_bitmap = 0;
				if(!result)
					tmp_bitmap = ((policy_t *)tmp_val)->allow;
				if(is_rbac_act_args == 2)
					((policy_t *)actual_val)->allow = tmp_bitmap | args_op_val;
				else
					((policy_t *)actual_val)->allow = SETVALID(tmp_bitmap, op);
			}
			else if(p_eff.args[0] == EFFECT_DENY && !memcmp(p, "deny", 4))
			{
				uint32_t tmp_bitmap = 0;
				if(!result)
					tmp_bitmap = ((policy_t *)tmp_val)->deny;
				if(is_rbac_act_args == 2)
					((policy_t *)actual_val)->deny = tmp_bitmap | args_op_val;
				else
					((policy_t *)actual_val)->deny = SETVALID(tmp_bitmap, op);
			}
			else
				continue;
		}
		else
		{
			if (p_def_type & 4)
				p = strtok(NULL, ",");
			if(!memcmp(p, "file", 4))
			{
				map_idx = 1;
				actual_key = (void *)policy_key;
				actual_val_head = (void *)&p_val;
				actual_val = (void *)&p_val;
			}
			else if(!memcmp(p, "dir", 3))
			{
				map_idx = 2;
				actual_val_head = (void *)&p_dir_val;
				actual_val = (void *)&(p_dir_val.rule);
				strcpy(p_dir_val.path, policy_key);
				p_dir_val.rule.valid = 1;
				dir_idx = lookup_dir_map(ctx, dir_count, policy_key);
				if(dir_idx == -1)
				{
					dir_idx = dir_count;
					dir_count++;
				}
				actual_key = (void *)&dir_idx;
			}
			else
			{
				snprintf(err, PATH_MAX_SUPP, "%s check unsupported\n\n", p);
				free_error = 1;
				goto free_head;
			}
		}
		// 有act写入新规则，无act写入0
		ebpf_data_update(ctx, actual_key, actual_val_head, map_idx, 1);
	}
	map_idx = 0;
	model_key = DIR_ENTRY_IDX;
	line_t d_entry;
	memset(&d_entry, 0, sizeof(line_t));
	d_entry.nums = 1;
	d_entry.args[0] = dir_count;
	if (ebpf_data_update(ctx, (void *)&model_key, (void *)&d_entry, map_idx, 1))
	{
		strcpy(err, "failed to ebpf_data_update d_entry nums\n");
		free_error = 1;
		goto free_head;
	}
free_head:
	fclose(f);
	sub_list *free_sub = sub_head->next, *next_sub;
	while(free_sub)
	{
		next_sub = free_sub->next;
		free(free_sub);
		free_sub = next_sub;
	}
	free(sub_head);
	act_args_list *free_aa = aa_head->next, *next_aa;
	while(free_aa)
	{
		next_aa = free_aa->next;
		free(free_aa);
		free_aa = next_aa;
	}
	free(aa_head);
	if(free_error)
		return 1;
	return 0;
}

int ebpf_data_next(ebpf_ctxtext_t *ctxtext, void *key, void *next, int idx)
{
	DBG("ebpf_next_data fd: %d\n", ctxtext->data_fd[idx]);
	return bpf_map_get_next_key(ctxtext->data_fd[idx], &key, &next);
}

/* Data handling abstractions */
int ebpf_data_lookup(ebpf_ctxtext_t *ctxtext, void *key, void *val, int idx)
{
	DBG("ebpf_data_lookup fd: %d\n", ctxtext->data_fd[idx]);
	return bpf_map_lookup_elem(ctxtext->data_fd[idx], key, val);
}

int ebpf_data_update(ebpf_ctxtext_t *ctxtext, void *key, void *val, int idx,
		int overwrite)
{
	unsigned long long flags = BPF_NOEXIST;
	if (overwrite)
		flags = BPF_ANY;
	DBG("ebpf_data_update fd: %d\n", ctxtext->data_fd[idx]);
	return bpf_map_update_elem(ctxtext->data_fd[idx], key, val, flags);
}

int ebpf_data_delete(ebpf_ctxtext_t *ctxtext, void *key, int idx)
{
	DBG("ebpf_data_delete fd: %d\n", ctxtext->data_fd[idx]);
	return bpf_map_delete_elem(ctxtext->data_fd[idx], key);
}