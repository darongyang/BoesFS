#include "boesfs.h"
#include "supporter.h"
#include <asm/siginfo.h>
#include <linux/signal.h>
#include <linux/types.h>

// 实现传入给定pid，然后能kill掉该进程及其子进程
int boesfs_kill(pid_t pid){

    struct siginfo info;
    int ret;

	info.si_signo = SIGHUP;
	info.si_errno = 0;
	info.si_code = SI_KERNEL;
	info.si_pid = task_tgid_vnr(current);
	info.si_uid = from_kuid_munged(current_user_ns(), current_uid());

    if(pid>0){
        read_lock(k_tasklist_lock);
		ret = k___kill_pgrp_info(SIGHUP, &info, find_vpid(pid));
	    read_unlock(k_tasklist_lock); 
        return ret;
    }
    else
        return -1;
}
