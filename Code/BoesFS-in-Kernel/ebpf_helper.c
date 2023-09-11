#include <linux/filter.h>
#include <linux/bpf.h>
#include <linux/perf_event.h>
#include <linux/bpf_perf_event.h>
#include <linux/poison.h>
#include <uapi/asm-generic/errno-base.h>
#include "supporter.h"
#include "boesfs.h"
#include "ebpf_helper.h"

/**
 * BoesFS 的eBPF 帮助函数定义
 * 拷贝内存数据到bpf栈、acl dfs搜索、dir tree搜索优化
 */


/**
 * @brief 根据type，将上下文src拷贝到bpf prog的栈上(dst)
 * @param type
 * @param dst
 * @param src
 */
BPF_CALL_3(bpf_boesfs_read_args, int, type , void * , dst , void * , src)
{
	int ret = -EINVAL;
	int sz = sizeof (union_args_t);
	void *args = &(((boesfs_args_t*)src)->args);

	/* 
		参数分组：
		* read、write
		* lookup、lookup2
		* link、symlink
		* open、unlink、rmdir、getattr、iterate、statfs
		* mkdir、create
		* mknod
		rename
		setattr
		llseek
		fsync
	*/
	
	if(type == BOESFS_READ || type == BOESFS_WRITE){
		struct read_boesfs_args *p_dst = dst;
		struct read_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);
		if(unlikely(ret < 0))
			goto err;	
		ret = probe_kernel_read(&(p_dst->len),&(p_src->len),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->offset),&(p_src->offset),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
	} 
	
	else if(type == BOESFS_LINK || type == BOESFS_SYMLINK) {
		struct link_boesfs_args *p_dst = dst;
		struct link_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(p_dst->linkpath, p_src->linkpath,strlen(p_src->linkpath)+1);
		if(unlikely(ret < 0))
			goto err;
	} 

	else if(type == BOESFS_LOOKUP || type == BOESFS_LOOKUP2) {
		struct lookup_boesfs_args *p_dst = dst;
		struct lookup_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->flags),&(p_src->flags),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
	} 
	
	else if(type == BOESFS_MKDIR || type == BOESFS_CREATE) {
		struct mkdir_boesfs_args *p_dst = dst;
		struct mkdir_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->mode),&(p_src->mode),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
	} 
	
	else if(type == BOESFS_OPEN || type == BOESFS_UNLINK || type == BOESFS_RMDIR 
			|| type == BOESFS_GETATTR || type == BOESFS_ITERATE || type == BOESFS_STATFS) {
		struct open_boesfs_args *p_dst = dst;
		struct open_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);

		if(unlikely(ret < 0))
			goto err;
	} 

	else if(type == BOESFS_MKNOD) {
		struct mknod_boesfs_args *p_dst = dst;
		struct mknod_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->mode),&(p_src->mode),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->dev),&(p_src->dev),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
	} 

	else if(type == BOESFS_RENAME) {
		struct rename_boesfs_args *p_dst = dst;
		struct rename_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(p_dst->newpath, p_src->newpath,strlen(p_src->newpath)+1);
		if(unlikely(ret < 0))
			goto err;
	} 

	else if(type == BOESFS_SETATTR) {
		struct setattr_boesfs_args *p_dst = dst;
		struct setattr_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->mode),&(p_src->mode),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->uid),&(p_src->uid),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->gid),&(p_src->gid),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
	} 

	else if(type == BOESFS_LLSEEK) {
		struct llseek_boesfs_args *p_dst = dst;
		struct llseek_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->offset),&(p_src->offset),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->whence),&(p_src->whence),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
	} 

	else if(type == BOESFS_FSYNC) {
		struct fsync_boesfs_args *p_dst = dst;
		struct fsync_boesfs_args *p_src = args;
		ret = probe_kernel_read(p_dst->path, p_src->path,strlen(p_src->path)+1);
		if(unlikely(ret < 0))
			goto err;
		ret = probe_kernel_read(&(p_dst->len),&(p_src->len),sizeof(int));
		if(unlikely(ret < 0))
			goto err;
	} 
	
	else {
		printk(KERN_INFO "Invalid boesfs_op_t\n");
		goto err;
	}

	return ret;

	err:
		memset(dst,0,sz);
		return ret;
}

static const struct bpf_func_proto bpf_boesfs_read_args_proto = {
	.func		= bpf_boesfs_read_args,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_ANYTHING,
	.arg2_type	= ARG_PTR_TO_MEM, //  ARG_PTR_TO_MEM, 
	.arg3_type  = ARG_PTR_TO_CTX, //  ARG_PTR_TO_CTX,
};



//  作用：去掉最后一个元素
//  /aasd//555b//asdasc/dadads//      ->      /aasd//555b//asdasc
//  会改变path 
static char* skipelem(char *path)
{
    char *ed = path;
    //  找到末尾.
    while(ed+1!=NULL && *(ed+1)!='\0')
        ++ed;
    //  去除末尾的 '/' 
    while(ed > path && *ed == '/'){
        *ed = '\0';
        --ed;
    }
    if(*ed == '\0'){
        return path;
    }
    //  去除本级目录
    while(ed > path && *ed != '/' && *ed != '\0'){
        *ed = '\0';
        --ed;
    }
    //  去掉末尾的 '/'
    if(ed > path){
        *ed = '\0';
        --ed;
    //  如果已经到path的最顶级目录. 令其必须是 '/'
    } else if(ed == path){
        *ed = '/';
        return path;
    } else {
        //  printk("never reach!\n");
    }
    return path;
}

//  constant = 0
	//  注意path指向的内容会被改变(去掉了最后一级目录). 
//  constant = 1
	//  path指向的内容不会被改变.
//  
/**
 * @brief 返回一个指向path的最深父目录的规则policy_t的指针
 * @param ctx
 * @param paths
 * @param dst
 * @param constant
 */
BPF_CALL_4(bpf_boesfs_acl_dfs_path,struct boesfs_args *, ctx,void *,path,struct policy *, dst,int,constant)
{
	int ret = 0;
	struct dir_node * root = ctx->dir_tree_root;
	struct dir_node * dfs_parent = NULL;
	char t_path[PATH_MAX_SUPP] = {0};
	char *parent = path;
	
	//  constant. 拷贝一个parent副本. 
	if (constant) {
		ret = probe_kernel_read(t_path, path ,strlen(path));
		parent = t_path;
		if(ret < 0)
			goto err;
	}

	//  去掉最后一级目录	
	skipelem(parent);


	//  找到最深父目录节点
	dfs_parent = query_dir_tree(root,parent);

	if(dfs_parent == NULL){
		// printk("parent %s not found\n",parent);
		goto out;
	} else {
		// printk("parent %s %d %d %d!\n",dfs_parent->val,dfs_parent->rule.valid,dfs_parent->rule.allow,dfs_parent->rule.deny);
	}
	
	//  copy rule from kernel mem to ebpf stack
	ret = probe_kernel_read(dst,&(dfs_parent->rule),sizeof(struct policy));

	if(ret < 0){
		printk("probe_kernel_read failed ret = %d!\n",ret);
		goto err;
	}

	// printk("dst %d %d %d\n",dst->valid,dst->allow,dst->deny);

out:
	return ret;

err:
	printk("bpf_boesfs_acl_dfs_path failed ret = %d\n",ret);
	return ret;
} 

static const struct bpf_func_proto bpf_boesfs_acl_dfs_path_proto = {
	.func		= bpf_boesfs_acl_dfs_path,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,						
	.arg1_type  = ARG_PTR_TO_CTX,					//  pointer to context	
	.arg2_type 	= ARG_PTR_TO_MEM,					//  pointer to stack
	.arg3_type  = ARG_PTR_TO_MEM,					//  pointer to stack
	.arg4_type  = ARG_ANYTHING						//  int
};



//  销毁dir tree
BPF_CALL_1(bpf_boesfs_destroy_dir_tree,struct boesfs_args *, ctx) 
{
	// printk("bpf_boesfs_destroy_dir_tree!\n");
	struct dir_node *root = ctx->dir_tree_root;
	int ret = 0;
	if(destroy_dir_tree(root) == FALSE){
		ret = -EINVAL;
	}
	return ret;
}

static const struct bpf_func_proto bpf_boesfs_destroy_dir_tree_proto = {
	.func		= bpf_boesfs_destroy_dir_tree,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,							
	.arg1_type  = ARG_PTR_TO_CTX,						//  pointer to context	
};


BPF_CALL_3(bpf_boesfs_build_dir_tree,struct boesfs_args *, ctx,struct line* ,dir_tree_meta,void *,p_dir_map)
{
	// printk("bpf_boesfs_build_dir_tree start!\n");

	int ret = 0;
	if(dir_tree_meta == NULL){
		ret = -1;
		goto err;
	}
	struct dir_node * root = init_dir_tree_root();
	if(root == NULL){
		ret = -1;
		goto err;
	}

	//  build dir tree 成功.
	//  保存根节点. 稍后由字节码返回给boesfs沙盒层 保存在本prog mount的沙盒层的superblock下.
	ctx->dir_tree_root = root;
	
	//  获取dir enrty数量
	int dir_entries = dir_tree_meta->args[0];
	// printk("dir_entries = %d\n",dir_entries);
	//  build dir tree
	int i;
	for(i=0;i<dir_entries;++i){
		struct dir_entry *d = k_bpf_map_lookup_elem_proto->func((void*)p_dir_map,(void*)(&i),0,0,0);
		if(unlikely(d == NULL)) {
			printk(KERN_ERR "i = %d\n");
			break;
		}

		if(insert_dir_tree(root,d->path,d->rule) != TRUE){
			ret = -1;
			goto err;
		}
	}


out:
	return ret;

err: 
	return ret;
}


static const struct bpf_func_proto bpf_boesfs_build_dir_tree_proto = {
	.func		= bpf_boesfs_build_dir_tree,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,							
	.arg1_type  = ARG_PTR_TO_CTX,						//  pointer to context	
	.arg2_type 	= ARG_PTR_TO_MEM,						//  pointer to valid memory (stack, packet, map value).  
	.arg3_type  = ARG_CONST_MAP_PTR						//  const argument used as pointer to bpf_map
};


/**
 * @brief 将一个ulong做hash
 * 
 * @param integer 待哈希的ulong
 * @return uint32_t 哈希结果
 */
static uint32_t boesfs_hash_ulong(uint32_t integer){
	return HASH(integer);
}

/**
 * @brief 将一个str做hash
 * 
 * @param str 待哈希的ulong字符串
 * @return uint32_t 哈希结果
 */
static uint32_t boesfs_hash_string(char * str){
	uint32_t hash = 1;
	int len = strlen(str);
	int i = 0;
	// printk("test: len = %d\n", len);
	for(i = 0; i < len; i++){
		hash = hash * (uint32_t)str[i];
	}
	hash = boesfs_hash_ulong(hash);
	return hash;
}


/**
 * @brief boesfs_args部分参数做hash运算
 * @param ctx
 * @param mode 选择哪些参数做hash运算
 */
BPF_CALL_2(bpf_boesfs_args_hash, struct boesfs_args *, ctx, int, mode)
{
	int op = ctx->op; // 操作类型
	uint32_t hash;

	if(mode == OBJ_HASH){
		struct open_boesfs_args *args = &((ctx->args).open_args);
		hash = boesfs_hash_string(args->path);
	}
	else if(mode == OP_HASH){
		hash = boesfs_hash_ulong((uint32_t)op);
	}
	else if(mode == ARG1_HASH){
		if(op == BOESFS_READ || op == BOESFS_WRITE)
		{
			struct read_boesfs_args *args = &((ctx->args).read_args);
			hash = boesfs_hash_ulong((uint32_t)args->len);
		}
		else if (op == BOESFS_LOOKUP || op == BOESFS_LOOKUP2)
		{
			struct lookup_boesfs_args *args = &((ctx->args).lookup_args);
			hash = boesfs_hash_ulong((uint32_t)args->flags);
		}
		else if (op == BOESFS_LINK || op == BOESFS_SYMLINK)
		{
			struct link_boesfs_args *args = &((ctx->args).link_args);
			hash = boesfs_hash_string(args->linkpath);
		}
		else if (op == BOESFS_MKDIR || op == BOESFS_CREATE)
		{
			struct mkdir_boesfs_args *args = &((ctx->args).mkdir_args);
			hash = boesfs_hash_ulong((uint32_t)args->mode);
		}
		else if (op == BOESFS_MKNOD)
		{
			struct mknod_boesfs_args *args = &((ctx->args).mknod_args);
			hash = boesfs_hash_ulong((uint32_t)args->mode);
		}
		else if (op == BOESFS_RENAME)
		{
			struct rename_boesfs_args *args = &((ctx->args).rename_args);
			hash = boesfs_hash_string(args->newpath);
		}
		else if (op == BOESFS_SETATTR)
		{
			struct setattr_boesfs_args *args = &((ctx->args).setattr_args);
			hash = boesfs_hash_ulong((uint32_t)args->mode);
		}
		else if (op == BOESFS_LLSEEK)
		{
			struct llseek_boesfs_args *args = &((ctx->args).llseek_args);
			hash = boesfs_hash_ulong((uint32_t)args->offset);
		}
		else if (op == BOESFS_MMAP)
		{
			struct mmap_boesfs_args *args = &((ctx->args).mmap_args);
			hash = boesfs_hash_ulong((uint32_t)args->startaddr);
		}
		else if (op == BOESFS_FSYNC)
		{
			struct fsync_boesfs_args *args = &((ctx->args).fsync_args);
			hash = boesfs_hash_ulong((uint32_t)args->len);
		}
		else
		{
			hash = 0;
		}
	}
	else if(mode == ARG2_HASH){
		if(op == BOESFS_READ || op == BOESFS_WRITE)
		{
			struct read_boesfs_args *args = &((ctx->args).read_args);
			hash = boesfs_hash_ulong((uint32_t)args->offset);
			// printk("read arg2:%lx",(uint32_t)args->offset);
		}
		else if (op == BOESFS_MKNOD)
		{
			struct mknod_boesfs_args *args = &((ctx->args).mknod_args);
			hash = boesfs_hash_ulong((uint32_t)args->dev);
		}
		else if (op == BOESFS_SETATTR)
		{
			struct setattr_boesfs_args *args = &((ctx->args).setattr_args);
			hash = boesfs_hash_ulong((uint32_t)args->uid);
		}
		else if (op == BOESFS_LLSEEK)
		{
			struct llseek_boesfs_args *args = &((ctx->args).llseek_args);
			hash = boesfs_hash_ulong((uint32_t)args->whence);
		}
		else if (op == BOESFS_MMAP)
		{
			struct mmap_boesfs_args *args = &((ctx->args).mmap_args);
			hash = boesfs_hash_ulong((uint32_t)args->endaddr);
		}
		else
		{
			hash = 0;
		}
	}
	else if(mode == ARG3_HASH){
		if(op == BOESFS_SETATTR)
		{
				struct setattr_boesfs_args *args = &((ctx->args).setattr_args);
				hash = boesfs_hash_ulong((uint32_t)args->uid);
		}
		else
		{
			hash = 0;
		}
	}
	// printk("hash :%lx\n",(uint32_t)hash);
	return hash;
}

static const struct bpf_func_proto bpf_boesfs_args_hash_proto = {
	.func		= bpf_boesfs_args_hash,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,							
	.arg1_type  = ARG_PTR_TO_CTX,						//  pointer to context	
	.arg2_type	= ARG_ANYTHING,
};




/**
 * BoesFS 的eBPF 验证器定义
 * func转换、valid验证
 */


//  设定boesfs bpf prog的helper function
static
const struct bpf_func_proto *boesfs_prog_func_proto(enum bpf_func_id func_id)
{
	switch (func_id) {
	case BPF_FUNC_map_lookup_elem:
		return k_bpf_map_lookup_elem_proto;
	case BPF_FUNC_map_update_elem:
		return k_bpf_map_update_elem_proto;
	case BPF_FUNC_map_delete_elem:
		return k_bpf_map_delete_elem_proto;
	case BPF_FUNC_probe_read:
		return k_bpf_probe_read_proto;
	case BPF_FUNC_ktime_get_ns:
		return k_bpf_ktime_get_ns_proto;
	case BPF_FUNC_get_current_pid_tgid:
		return k_bpf_get_current_pid_tgid_proto;
	case BPF_FUNC_get_current_task:
		return k_bpf_get_current_task_proto;
	case BPF_FUNC_get_current_uid_gid:
		return k_bpf_get_current_uid_gid_proto;
	case BPF_FUNC_get_current_comm:
		// return k_bpf_get_current_comm_proto;
		return &bpf_boesfs_destroy_dir_tree_proto;
	case BPF_FUNC_trace_printk:
		return k_bpf_get_trace_printk_proto();
	//  for test
	case BPF_FUNC_get_smp_processor_id:
		// return k_bpf_get_smp_processor_id_proto;
		return &bpf_boesfs_build_dir_tree_proto;
	case BPF_FUNC_get_prandom_u32:
		// return k_bpf_get_prandom_u32_proto;
		return &bpf_boesfs_args_hash_proto;
	case BPF_FUNC_perf_event_output:
		return k_bpf_perf_event_output_proto;
	case BPF_FUNC_probe_read_str:
		return k_bpf_probe_read_str_proto;
	// boesfs related
	case BPF_FUNC_boesfs_read_args:
		return &bpf_boesfs_read_args_proto;
	case BPF_FUNC_boesfs_acl_dfs_path:
		return &bpf_boesfs_acl_dfs_path_proto;
	// case BPF_FUNC_boesfs_test_func:
		// return &bpf_boesfs_test_func_proto;
	default:
		return NULL;
	}
}

//  the callback is used to customize verifier to restrict eBPF program access to only certain fields within ctx structure with specified size and alignment.
static bool boesfs_prog_is_valid_access(int off, int size, enum bpf_access_type type,
					enum bpf_reg_type *reg_type)
{
	if (off < 0 || off >= sizeof(boesfs_args_t))
		return false;
	//  只读
	if (type != BPF_READ)
		return false;
	//  对齐
	if (off % size != 0)
		return false;
	//  bpf prog对于传入的ctx的访问范围不能超过struct boesfs_args
	if (off + size > sizeof(boesfs_args_t))
		return false;
	return true;
}

//  boesfs_prog_func_proto : 用于修正func id -> func addr
//  is valid access	: 检测bpf prog对于ctx的访问是否越界
static const struct bpf_verifier_ops boesfs_prog_ops = {
	.get_func_proto  = boesfs_prog_func_proto,
	.is_valid_access = boesfs_prog_is_valid_access,
};



/**
 * BoesFS 的eBPF类型定义
 * 类型、注册、取消注册
 */

//  注册BPF_PROG_TYPE_BOESFS类型
static struct bpf_prog_type_list boesfs_type = {
	.ops	= &boesfs_prog_ops,
	.type	= BPF_PROG_TYPE_BOESFS,
};


int __init boesfs_register_prog_type(void) 
{
	printk("Boesfs Type Prog Registering\n");
	//  头插法	
	struct list_head *new = &(boesfs_type.list_node);

	// printk("new(boesfs_tl.list_node) %p\n",new);
	// printk("prev(bpf_prog_types) %p\n",k_bpf_prog_head);
	// printk("next %p\n",k_bpf_prog_head->next);
	
	// on Linux 4.11
	new->next = k_bpf_prog_head->next;
	new->prev = k_bpf_prog_head;
	k_bpf_prog_head->next = new;

	// on Linux 4.10 and below
	// bpf_register_prog_type(&boesfs_type);

	printk("Boesfs Type Prog Registering finished\n");
	return 0;
}

int __exit boesfs_unregister_prog_type(void)
{
	printk("Boesfs Type Prog Unloading\n");

	struct list_head *del = &(boesfs_type.list_node);
	struct list_head *prev = del->prev;
	WRITE_ONCE(prev->next,del->next);
	del->next = LIST_POISON1;
	del->prev = LIST_POISON2;
  	
	printk("Boesfs Type Prog Unloading finished\n");
	return 0;	
}


