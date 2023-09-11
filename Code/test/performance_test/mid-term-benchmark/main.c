#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include "supporter.h"
#include <linux/bpf.h>

/************************************************************************/
/* 本benchmark提取出了初赛时查询的实现                                    */
/* 随机规则目录1024条，随机查询目录2048条s                                 */
/* 模拟了1000000次查询                                                   */
/************************************************************************/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("shc");

#define QUERY_RULE 2048
#define TOTAL_RULE 1024
#define NUM_RUNS 1000000
#define PATH_MAX_SUPP 256

struct bpf_map * test_map = NULL;

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


//  依据深度优先原则，查找规则
	//  return null : p_map中 无path以及其父目录的 acl规则
	//  return value : p_map中 path的最深父目录的 acl规则
//  如果该思路可行，后续可能可以优化。
static void *bpf_boesfs_acl_dfs_path(struct bpf_map * p_map , void * path)
{
	char parent[PATH_MAX_SUPP] = {0};
	probe_kernel_read(parent, path ,strlen(path));
    int depth = 0;
	void *t = NULL;
	//  由深到浅，查询权限
	//  查询到结果 或者 到 '/' 后停止查询 且 至多查找10层
	while(parent[1] != '\0' && t == NULL && (++depth < 20))
	{
		skipelem(parent);
		t = k_htab_ops->map_lookup_elem(p_map,(void*)parent);
		// printk("parent %s t %p\n",parent,t);
	}
	return t;
}


static struct bpf_map* init_map(void)
{
    //  alloc
    union bpf_attr attr;
    attr.map_type = BPF_MAP_TYPE_HASH;
    attr.key_size = PATH_MAX_SUPP;
    attr.value_size = sizeof(policy_t);
    attr.max_entries = TOTAL_RULE;
    attr.map_flags = 0;
    if(k_htab_ops == NULL) {
        return NULL;
    }
    test_map = k_htab_ops->map_alloc(&attr);
    if(IS_ERR(test_map)){
        return test_map;
    }
    int i;
    //  insert 
    for(i=0;i<TOTAL_RULE;++i)
    {
        policy_t p;
        p.deny = 1;
        p.allow = 2;
        char buf[PATH_MAX_SUPP] = {0};
        probe_kernel_read(buf,paths[i],strlen(paths[i]));
        k_htab_ops->map_update_elem(test_map,buf,&p,BPF_ANY);
    }
    return test_map;
}


static int __init mid_module_init(void) {

    printk("hello mid-term benchmark\n");

    //  init sym
    get_all_symbol_addr();
    //  init map
    struct bpf_map * t = init_map();
    if(IS_ERR(t)){
        int ret = PTR_ERR(t);
        printk("t = %d\n",ret);
    }

    unsigned long flags;
    ktime_t start, end;
    s64 total_time = 0;
    int i;

    for (i = 0; i < NUM_RUNS; i++) {
        int idx = i%QUERY_RULE;
        char p[PATH_MAX_SUPP] = {0};
        probe_kernel_read(p,query_path[idx],strlen(query_path[idx]));
        start = ktime_get();
        bpf_boesfs_acl_dfs_path(test_map,p); // Run the test function
        end = ktime_get();
        total_time += ktime_to_ns(ktime_sub(end, start));
    }

    pr_info("Average runtime of my_test_function: %lld ns\n", total_time / NUM_RUNS);
    
    return 0;
}

static void __exit mid_module_exit(void) {
    printk("bye mid-term benchmark\n");
    k_htab_ops->map_free(test_map);
}

module_init(mid_module_init);
module_exit(mid_module_exit);
