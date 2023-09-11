#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include "supporter.h"
#include <linux/bpf.h>
#include "dir_tree.h"
#include "ebpf_helper.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("shc");

/************************************************************************/
/* 本benchmark提取出了决赛时查询的实现                                    */
/* 随机规则目录1024条，随机查询目录2048条s                                 */
/* 模拟了1000000次查询                                                   */
/************************************************************************/


#define QUERY_RULE 2048
#define TOTAL_RULE 1024
#define NUM_RUNS 1000000
#define PATH_MAX_SUPP 256
static struct bpf_map *dir_map = NULL;
static struct boesfs_args arg;


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
        //  printf("never reach!\n");
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
static int bpf_boesfs_acl_dfs_path(struct boesfs_args * ctx,void * path,struct policy * dst,int constant)
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
		goto out;
	}
	
	//  copy rule from kernel mem to ebpf stack
	ret = probe_kernel_read(dst,&(dfs_parent->rule),sizeof(struct policy));

	if(ret < 0){
		printk("probe_kernel_read failed ret = %d!\n",ret);
		goto err;
	}

out:
	return ret;
err:
	printk("bpf_boesfs_acl_dfs_path failed ret = %d\n",ret);
	return ret;
} 


//  销毁dir tree
static int bpf_boesfs_destroy_dir_tree(struct boesfs_args * ctx) 
{
	struct dir_node *root = ctx->dir_tree_root;
	int ret = 0;
	if(destroy_dir_tree(root) == FALSE){
		ret = -EINVAL;
	}
	return ret;
}


static int bpf_boesfs_build_dir_tree(struct boesfs_args * ctx,struct bpf_map *p_dir_map)
{
	int ret = 0;
	struct dir_node * root = init_dir_tree_root();
	if(root == NULL){
		ret = -1;
		goto err;
	}

	//  build dir tree 成功.
	//  保存根节点. 稍后由字节码返回给boesfs沙盒层 保存在本prog mount的沙盒层的superblock下.
	ctx->dir_tree_root = root;
	
	//  build dir tree
	int i;
	for(i=0;i<TOTAL_RULE;++i){
		struct dir_entry *d = k_array_ops->map_lookup_elem(p_dir_map,(&i));
		if(unlikely(d == NULL)) {
			printk(KERN_ERR "i = %d\n",i);
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


static struct bpf_map* init_map(void)
{
    //  alloc
    union bpf_attr attr;
    attr.map_type = BPF_MAP_TYPE_ARRAY;
    attr.key_size = sizeof(uint32_t);
    attr.value_size = sizeof(dir_entry_t),
    attr.max_entries = TOTAL_RULE;
    attr.map_flags = 0;
    if(k_array_ops == NULL) {
        return NULL;
    }
    dir_map = k_array_ops->map_alloc(&attr);
    if(IS_ERR(dir_map)){
        return dir_map;
    }
    int i;
    //  insert
    for(i=0;i<TOTAL_RULE;++i)
    {
        policy_t p;
        p.deny = 1;
        p.allow = 2;
        dir_entry_t det;
        memset(&det,0,sizeof(det));
        probe_kernel_read(det.path,paths[i],strlen(paths[0]));
        det.rule.valid = 1;
        det.rule.allow = 0;
        det.rule.deny = 9;
        k_array_ops->map_update_elem(dir_map,&i,&det,BPF_ANY);
    }
    return dir_map;
}


static int __init final_module_init(void) {
    printk("hello final benchmark\n");

    int err = 0;
    //  init sym
    get_all_symbol_addr();
    //  init map
    struct bpf_map * t = init_map();
    if(IS_ERR(t)){
        int ret = PTR_ERR(t);
        printk("t = %d\n",ret);
    }
    //  init slab
    dir_node_cache = kmem_cache_create("dir_node_cache", sizeof(struct dir_node), 0, SLAB_HWCACHE_ALIGN, NULL);
	if(dir_node_cache == NULL) {
		err = -ENOMEM;
		goto out;
	}

    //  build
    bpf_boesfs_build_dir_tree(&arg,dir_map);

    unsigned long flags;
    ktime_t start, end;
    s64 total_time = 0;
    int i;

    for (i = 0; i < NUM_RUNS; i++) {
        int idx = i%QUERY_RULE;
        char buf_path[PATH_MAX_SUPP] = {0};
        struct policy p;
        memset(&p,0,sizeof(p));
        probe_kernel_read(buf_path,query_path[idx],strlen(query_path[idx]));

        start = ktime_get();
        bpf_boesfs_acl_dfs_path(&arg,buf_path,&p,0);        //  run the test api
        end = ktime_get();
        total_time += ktime_to_ns(ktime_sub(end, start));
    }

    pr_info("Average runtime of my_test_function: %lld ns\n", total_time / NUM_RUNS);
out:
    return err;
}

static void __exit final_module_exit(void) {
    printk("bye final benchmark\n");
    bpf_boesfs_destroy_dir_tree(&arg);
    k_array_ops->map_free(dir_map);
    kmem_cache_destroy(dir_node_cache);
}

module_init(final_module_init);
module_exit(final_module_exit);
