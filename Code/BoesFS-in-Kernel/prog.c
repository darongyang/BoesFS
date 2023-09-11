#include "boesfs.h"
#include <linux/filter.h>
#include <linux/bpf.h>

/**
 * @brief 设置超级块的prog指针
 * @param 
 *        prog_fd: Agent 传来的prog fd
 *        sb: 待设置的超级块
 * @return -1: 失败 0: 成功
 */
int boesfs_set_prog_pos(struct super_block *sb, uint32_t prog_fd){
    struct boesfs_sb_info * sb_info;
    struct bpf_prog * new_prog, *old_prog;

    // 非法检查
    if(!sb){
        printk("ERROR: sb is NULL!\n");
        return ERR;
    }

    sb_info = BOESFS_SB(sb);

    if(!sb_info){
        printk("ERROR: sb_info is NULL!\n");
        return ERR;
    }

    if(prog_fd <= 0){
        printk("ERROR: prog_fd is not valid!\n");
        return ERR;
    }

    new_prog = bpf_prog_get_type(prog_fd, BPF_PROG_TYPE_BOESFS);
    // prog = bpf_prog_get_type(prog_fd, BPF_PROG_TYPE_SANDFS);

    if (IS_ERR(new_prog)){
        printk("ERROR: get prog from fd fail!\n");
        return ERR;
    }

    old_prog = sb_info->prog_pos;
    sb_info->prog_pos = new_prog;

    if (old_prog)
		bpf_prog_put(old_prog);

    printk("INFO: set prog to super block is success! \n");

    return 0;
}


/**
 * @brief 运行bpf prog, eBPF触发点函数
 * @param rawdata: Agent mount时传来的数据
 * @return -1: 不符合，>0:符合，并返回fd
 */
int boesfs_run_prog(struct bpf_prog * prog_pos, struct boesfs_args * check_args)
{
    struct bpf_prog * run_prog;
    int ret;

    if(!prog_pos){
        printk("ERROR: run_prog is NULL!\n");
        return ERR;
    }

    // run_prog = prog_pos;
    run_prog = READ_ONCE(prog_pos); // 一定要READ ONCE?

    rcu_read_lock(); 
    ret = BPF_PROG_RUN(run_prog, check_args);
    rcu_read_unlock();

    return ret;
}