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
	int p_def_type[4], is_rbac = 0, is_args = 0;
	int model_key, map_idx, r_def_num = 0, p_def_num = 0;
	memset(p_def_type, 0, 4 * sizeof(int));
	memset(&r_def, 0, sizeof(line_t));
	memset(&p_def, 0, sizeof(line_t));
	memset(&p_eff, 0, sizeof(line_t));
	memset(&matchers, 0, sizeof(line_t));
	//储存规则
	map_idx = 0;
	// 打开规则文件
	// info("open model file\n");
	f = fopen(model_file,"r");
	if(!f)
	{
		snprintf(err, PATH_MAX_SUPP, "open %s error\n\n", model_file);
		return 1;
	}
	// 判断请求定义格式是否正确
	fgets(buf, 512, f);
	// info(buf);
	if(memcmp(buf, "[request_definition]", 20))
	{
		strcpy(err, "no [request_definition]\n");
		return 1;
	}
	fgets(buf, 512, f);
	// info(buf);
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
	// info(buf);
	fgets(buf, 512, f);
	// info(buf);
	if(memcmp(buf, "[policy_definition]", 19))
	{
		strcpy(err, "no [policy_definition]");
		return 1;
	}
	fgets(buf, 512, f);
	// info(buf);
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
			p_def_type[0] = 1;
		else if(!strcmp(p, "obj"))
			p_def_type[1] = 1;
		else if(!strcmp(p, "act"))
			p_def_type[2] = 1;
		else if(!strcmp(p, "args"))
			p_def_type[3] = 1;
		else
		{
			strcpy(err, "unknown policy defined");
			return 1;
		}
		p = strtok(NULL,",");
	}
	p_def.nums = p_def_num;
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
	// info(buf);
	fgets(buf, 512, f);
	// info(buf);
	if(!memcmp(buf, "[role_definition]", 17))
	{
		fgets(buf, 512, f);
		str_handle(buf);
		if(memcmp(buf, "g=_,_", 5))
		{
			strcpy(err, "invalid [role_definition]");
			return 1;
		}
		is_rbac = 1;
		fgets(buf, 3, f);
		fgets(buf, 512, f);
	}
	if(memcmp(buf, "[policy_effect]", 15))
	{
		strcpy(err, "no [policy_effect]");
		return 1;
	}
	// 读取策略结果
	fgets(buf, 512, f);
	// info(buf);
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
	// info(buf);
	fgets(buf, 512, f);
	// info(buf);
	if(memcmp(buf, "[matchers]", 10))
	{
		strcpy(err, "no [matchers]");
		return 1;
	}
	// 读取匹配逻辑
	
	fgets(buf, 512, f);
	// info(buf);
	str_handle(buf);
	if(!memcmp(buf, "m=r.sub==p.sub&&r.obj==p.obj&&r.act==p.act", 42) ||
		!memcmp(buf, "m=g(r.sub,p.sub)&&r.obj==p.obj&&r.act==p.act", 44) && is_rbac)
		matchers.args[0] = SUB_OBJ_ACT;
	else if(!memcmp(buf, "m=r.sub==p.sub&&r.obj==p.obj", 28))
		matchers.args[0] = SUB_OBJ;
	else if(!memcmp(buf, "m=r.sub==p.sub&&r.act==p.act", 28))
		matchers.args[0] = SUB_ACT;
	else if(!memcmp(buf, "m=r.obj==p.obj&&r.act==p.act", 28))
		matchers.args[0] = OBJ_ACT;
	else
	{
		strcpy(err, "matchers undifined");
		return 1;
	}
	if(p_def_type[4] && strstr(buf, "&&r.args==p.args"))
	{
		if(matchers.args[0] == SUB_OBJ_ACT)
			matchers.args[0] = SUB_OBJ_ACT_ARGS;
		else if(matchers.args[0] == OBJ_ACT)
			matchers.args[0] = OBJ_ACT_ARGS;
		else
		{
			strcpy(err, "matchers undifined");
			return 1;
		}
		is_args = 1;
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
	// info("open policy file\n");
	f = fopen(policy_file,"r");
	role_list *head = (role_list *)malloc(sizeof(role_list));
	head->next = NULL;
	if(!f)
	{
		snprintf(err, PATH_MAX_SUPP, "open %s error\n\n", policy_file);
		return 1;
	}
	if(is_rbac)
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
				return 1;
			}
			if(!strcmp(tmp_p, elf))
			{
				p = strtok(NULL, ",");
				role_list *tmp = (role_list *)malloc(sizeof(role_list));
				strncpy(tmp->role, p, 30);
				printf("%s\n", p);
				tmp->next = head->next;
				head->next = tmp;
			}
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
		// info(buf);
		// info("\n");
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
		memset(&p_val, 0, sizeof(policy_t));
		memset(&p_val_l, 0, sizeof(line_t));
		memset(policy_key, 0, PATH_MAX_SUPP);
		memset(&p_dir_val, 0, sizeof(dir_entry_t));
		p_val.valid = 1;
		p = strtok(buf, ",");
		p = strtok(NULL, ",");
		if(p_def_type[1] && p_def_type[0]) 
		{
			// sub, obj都有的时候比较sub与要执行的程序名，有则写入该规则
			if(matchers.args[0] == SUB_OBJ_ACT || matchers.args[0] == SUB_OBJ)
			{
				if(is_rbac)
				{
					/***
					 * 对比角色是否在链表中
					*/
					role_list *tmp_role = head;
					int flag = 0;
					while(tmp_role->next)
					{
						tmp_role = tmp_role->next;
						if(!strcmp(p, tmp_role->role))
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
						return 1;
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
		if(p_def_type[2] && matchers.args[0] != SUB_OBJ)
		{
			int op = check_op(p);
			if(op == -1)
			{
				snprintf(err, PATH_MAX_SUPP, "%s check unsupported\n\n", p);
				return 1;
			}
			if(is_args)
			{
				uint32_t args_count = 0;
				uint32_t args_val_hash = 0;
				char tmp_args[PATH_MAX_SUPP] = {0};
				char *args_p = NULL, *end_p = NULL;
				args_key = hash_uint32(hash_uint32(op) + hash_str(policy_key));
				// printf("add_key:%x\n", hash_uint32(op) + hash_str(policy_key));
				// printf("args_key:%x\n", args_key);
				end_p = strstr(p + strlen(p) + 1, ")");
				if(!end_p)
				{
					snprintf(err, PATH_MAX_SUPP, "args invalid: %s\n\n", p + strlen(p) + 1);
					return 1;
				}
				p = p + strlen(p) + 1;
				if(*p != '(')
				{
					snprintf(err, PATH_MAX_SUPP, "args check unsupported: %s\n\n", p);
					return 1;
				}
				strcpy(tmp_args, p + 1);
				args_p = strtok(tmp_args, ")");
				args_p = strtok(args_p, ",");
				while(args_p)
				{
					if(args_count >= 3)
					{
						snprintf(err, PATH_MAX_SUPP, "too many args: %s\n\n", p);
						return 1;
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
						printf("args%d: %x\n", args_count, s2d(args_p));
						printf("args%d_hash: %x\n", args_count, hash_uint32(s2d(args_p)));
						args_val_hash += hash_uint32(s2d(args_p));
					}
					else
						args_val_hash += hash_str(args_p);
					args_p = strtok(NULL, ",");
				}
				if(args_val != 0)
				{
					// args_val |= ((hash_uint32(args_val_hash) >> 3) & 0x1FFFFFFF) << 3;
					args_val |= hash_uint32(args_val_hash) & 0xFFFFFFF8;
					map_idx = 3;
					// printf("key:%x\n", args_key);
					// printf("args_bits:%x\n", args_val & 0x7);
					// printf("args_hash:%x\n", args_val & 0xFFFFFFF8);
					ebpf_data_update(ctx, (void *)&args_key, (void *)&args_val, map_idx, 1);

					// debug写map是否成功
					// int test;
					// uint32_t *testt = malloc(sizeof(uint32_t));;
					// test = ebpf_data_lookup(ctx, (void *)&args_key, (void *)testt, map_idx);
					// if(test){
					// 	printf("lookup fail\n");
					// }
					// else
					// {
					// 	printf("value:%x\n", *testt);
					// }
				}
				p = end_p + 2;
			}
			else
				p = p + strlen(p) + 1;
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
					return 1;
				}
			}
			p = strtok(NULL, ",");
			// 读取map查看是否已有相应的规则
			// printf("map_idx:%d\n", map_idx);
			result = ebpf_data_lookup(ctx, actual_key, tmp_val_head, map_idx);
			if(result)
			{
				// 若无对应规则，直接按照0来计算
				if(p_eff.args[0] == EFFECT_ALLOW && !memcmp(p, "allow", 5))
					((policy_t *)actual_val)->allow = SETVALID(0, op);
				else if(p_eff.args[0] == EFFECT_DENY && !memcmp(p, "deny", 4))
					((policy_t *)actual_val)->deny = SETVALID(0, op);
				else
					continue;
			}
			else
			{
				// 若已有对应规则，加上新规则
				if(p_eff.args[0] == EFFECT_ALLOW && !memcmp(p, "allow", 5))
					((policy_t *)actual_val)->allow = SETVALID(((policy_t *)tmp_val)->allow, op);
				else if(p_eff.args[0] == EFFECT_DENY && !memcmp(p, "deny", 4))
					((policy_t *)actual_val)->deny = SETVALID(((policy_t *)tmp_val)->deny, op);
				else
					continue;
			}
		}
		else
		{
			if (p_def_type[2])
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
				return 1;
			}
		}
		// 有act写入新规则，无act写入0
		// printf("%s_%d_%d\n", (char *)actual_key, ((policy_t *)actual_val)->allow, ((policy_t *)actual_val)->deny);
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
		return 1;
	}
	fclose(f);
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