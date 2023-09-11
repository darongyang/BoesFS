/*
 * Copyright (c) 1998-2017 Erez Zadok
 * Copyright (c) 2009	   Shrikar Archak
 * Copyright (c) 2003-2017 Stony Brook University
 * Copyright (c) 2003-2017 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "wrapfs.h"

/*
 * returns: -ERRNO if error (returned to user)
 *          0: tell VFS to invalidate dentry
 *          1: dentry is valid
 */
// 检查wrapfs的某个dentry是否有效，取决于底层fs的dentry是否有效,调用底层的d_revalidate方法判断
static int wrapfs_d_revalidate(struct dentry *dentry, unsigned int flags)
{
	struct path lower_path;
	struct dentry *lower_dentry;
	int err = 1;

	if (flags & LOOKUP_RCU) // 带有RCU标志，则error
		return -ECHILD;

	wrapfs_get_lower_path(dentry, &lower_path); // 获得对应底层fs的path，引用加一
	lower_dentry = lower_path.dentry; // 进而获得底层fs的dentry
	if (!(lower_dentry->d_flags & DCACHE_OP_REVALIDATE)) // 以前缓存过？标记在了d_flags里？
		goto out;
	err = lower_dentry->d_op->d_revalidate(lower_dentry, flags); // 
out:
	wrapfs_put_lower_path(dentry, &lower_path); // 用完了，减少path的引用
	return err;
}

// 释放wrapsfs的dentry（底层fs置空path + 减少引用），释放数据fsdata，置空指针
static void wrapfs_d_release(struct dentry *dentry)
{
	/* release and reset the lower paths */
	wrapfs_put_reset_lower_path(dentry);
	wrapfs_free_dentry_private_data(dentry);
	return;
}

const struct dentry_operations wrapfs_dops = {
	.d_revalidate	= wrapfs_d_revalidate,
	.d_release	= wrapfs_d_release,
};
