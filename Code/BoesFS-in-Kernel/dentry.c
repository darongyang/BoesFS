/*
 * Copyright (c) 1998-2022 Erez Zadok
 * Copyright (c) 2009	   Shrikar Archak
 * Copyright (c) 2003-2022 Stony Brook University
 * Copyright (c) 2003-2022 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "boesfs.h"

/*
 * returns: -ERRNO if error (returned to user)
 *          0: tell VFS to invalidate dentry
 *          1: dentry is valid
 */
static int boesfs_d_revalidate(struct dentry *dentry, unsigned int flags)
{
	struct path lower_path;
	struct dentry *lower_dentry;
	int err = 1;

	if (flags & LOOKUP_RCU)
		return -ECHILD;

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	if (!(lower_dentry->d_flags & DCACHE_OP_REVALIDATE))
		goto out;
	
	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	char * file_path = get_absolute_path_from_path_struct(&lower_path, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from path in vfs d_revalidate error!");
		return err;
	}
	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_LOOKUP2;
	memcpy(args.elfpath, BOESFS_SB(dentry->d_sb)->elfpath, SINGLE_PATH_MAX);
	// lookup args
	args.args.lookup2_args.flags = (int)flags;
	memcpy(args.args.lookup2_args.path, file_path, strlen(file_path) + 1);

	//  dir tree root
	args.dir_tree_root = BOESFS_SB(dentry->d_sb)->dir_tree_root;
	
	err = boesfs_run_prog(BOESFS_SB(dentry->d_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(dentry->d_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(dentry->d_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(dentry->d_sb)->logpath), err);
	}
	// kill
	if(err < 0 && BOESFS_SB(dentry->d_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(dentry->d_sb)->deny > BOESFS_SB(dentry->d_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(dentry->d_sb)->pid);
	}
	vfree((void *)path_buf);
	if (err < 0)
		goto out;
		
	/* VFS functions args hook END */

	err = lower_dentry->d_op->d_revalidate(lower_dentry, flags);
out:
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static void boesfs_d_release(struct dentry *dentry)
{
	/* release and reset the lower paths */
	boesfs_put_reset_lower_path(dentry);
	free_dentry_private_data(dentry);
	return;
}

const struct dentry_operations boesfs_dops = {
	.d_revalidate	= boesfs_d_revalidate,
	.d_release	= boesfs_d_release,
};
