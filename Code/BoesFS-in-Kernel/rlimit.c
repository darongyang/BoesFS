/* 内核模块实现rlimit配置 */

#include "boesfs.h"
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

static struct task_struct *limit_write_kthread1 = NULL;
static struct task_struct *limit_write_kthread2 = NULL;

int write_func(void * info){
    char limit_info[LIMIT_INFO_MAX];
    int len;
    struct file *file = NULL;
    mm_segment_t fs_bak;
    loff_t pos;

    if(info == NULL)
        return ERR;

    memcpy(limit_info, info, strlen(info) + 1);

    
    len = strlen(limit_info);
    file = filp_open("/etc/security/limits.conf", O_RDWR|O_APPEND|O_CREAT,0644);

    if(IS_ERR(file)){
        printk("ERROR: Open /etc/security/limits.conf fail!");
        return ERR;
    }

    fs_bak = get_fs(); 
    set_fs(KERNEL_DS);

    pos = file->f_pos;
    vfs_write(file, limit_info, len, &pos);

    filp_close(file,NULL);
    set_fs(fs_bak);

    if(info != NULL)
        kfree(info);

    do_exit(0);
    return 0;
}


/**
 * @brief 内核写资源限制配置文件，修改memlock资源限制
 * 
 * @param user 用户名
 * @param limit memlock资源限制大小
 * @return int 0表示成功,-1表示失败
 */
int set_user_limits(void * user, int limit){
    char *info1;
    char *info2;
    int pos;

    if(user == NULL)
        return ERR;

    if(limit > RLIMIT_MAX)
        return ERR;

    if((strlen(user) + strlen("hard") + strlen("memlock") + strlen("4096") + 3*strlen("\t") + strlen("\n")) 
        > LIMIT_INFO_MAX)
        return ERR;

    info1 = (char *)kmalloc(LIMIT_INFO_MAX,GFP_KERNEL);
    info2 = (char *)kmalloc(LIMIT_INFO_MAX,GFP_KERNEL);

    sprintf(info1, "%s\thard\tmemlock\t%d\n", user, limit);


    limit_write_kthread1 = kthread_run(write_func, info1, "kthread-boesfs-rlimit1"); 

    sprintf(info2, "%s\tsoft\tmemlock\t%d\n", user, limit);


    limit_write_kthread2 = kthread_run(write_func, info2, "kthread-boesfs-rlimit2"); 

    if (!limit_write_kthread1) {
		printk("kthread_run fail");
		return ERR;
	}
    if (!limit_write_kthread2) {
		printk("kthread_run fail");
		return ERR;
	}
    return 0;
}