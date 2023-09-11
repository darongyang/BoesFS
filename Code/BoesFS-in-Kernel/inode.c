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

static int boesfs_create(struct inode *dir, struct dentry *dentry,
			 umode_t mode, bool want_excl)
{
	int err;
	struct dentry *lower_dentry;
	struct dentry *lower_parent_dentry = NULL;
	struct path lower_path;


	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	char * file_path = get_absolute_path_from_dentry_struct(dentry, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from dentry in vfs mknod error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_CREATE;
	memcpy(args.elfpath, BOESFS_SB(dir->i_sb)->elfpath, SINGLE_PATH_MAX);
	// unlink args
	args.args.create_args.mode = (int)mode;
	memcpy(args.args.create_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(dir->i_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(dir->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(dir->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(dir->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(dir->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(dir->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(dir->i_sb)->deny > BOESFS_SB(dir->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(dir->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;
	
	/* VFS functions args hook END */

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_parent_dentry = lock_parent(lower_dentry);

	err = vfs_create(d_inode(lower_parent_dentry), lower_dentry, mode,
			 want_excl);
	if (err)
		goto out;
	err = boesfs_interpose(dentry, dir->i_sb, &lower_path);
	if (err)
		goto out;
	fsstack_copy_attr_times(dir, boesfs_lower_inode(dir));
	fsstack_copy_inode_size(dir, d_inode(lower_parent_dentry));

out:
	unlock_dir(lower_parent_dentry);
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static int boesfs_link(struct dentry *old_dentry, struct inode *dir,
		       struct dentry *new_dentry)
{
	struct dentry *lower_old_dentry;
	struct dentry *lower_new_dentry;
	struct dentry *lower_dir_dentry;
	u64 file_size_save;
	int err;
	struct path lower_old_path, lower_new_path;

	/* VFS functions args hook START */
	void *path_buf1 = vmalloc(PATH_MAX);
	void *path_buf2 = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf1)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	if (IS_ERR(path_buf2)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	
	char * file_path1 = get_absolute_path_from_dentry_struct(old_dentry, path_buf1);
	char * file_path2 = get_absolute_path_from_dentry_struct(new_dentry, path_buf2);

	if(file_path1 == NULL){
		printk("ERROR: get path from dentry in vfs link error!");
		return err;
	}
	if(file_path2 == NULL){
		printk("ERROR: get path from dentry in vfs link error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_LINK;
	memcpy(args.elfpath, BOESFS_SB(dir->i_sb)->elfpath, SINGLE_PATH_MAX);
	// link args
	memcpy(args.args.link_args.path, file_path1, strlen(file_path1) + 1);
	memcpy(args.args.link_args.linkpath, file_path2, strlen(file_path2) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(dir->i_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(dir->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(dir->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(dir->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(dir->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(dir->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(dir->i_sb)->deny > BOESFS_SB(dir->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(dir->i_sb)->pid);
	}

	vfree((void *)path_buf1);
	vfree((void *)path_buf2);

	if (err < 0)
		return err;
	
	/* VFS functions args hook END */

	file_size_save = i_size_read(d_inode(old_dentry));
	boesfs_get_lower_path(old_dentry, &lower_old_path);
	boesfs_get_lower_path(new_dentry, &lower_new_path);
	lower_old_dentry = lower_old_path.dentry;
	lower_new_dentry = lower_new_path.dentry;
	lower_dir_dentry = lock_parent(lower_new_dentry);

	err = vfs_link(lower_old_dentry, d_inode(lower_dir_dentry),
		       lower_new_dentry, NULL);
	if (err || !d_inode(lower_new_dentry))
		goto out;

	err = boesfs_interpose(new_dentry, dir->i_sb, &lower_new_path);
	if (err)
		goto out;
	fsstack_copy_attr_times(dir, d_inode(lower_new_dentry));
	fsstack_copy_inode_size(dir, d_inode(lower_new_dentry));
	set_nlink(d_inode(old_dentry),
		  boesfs_lower_inode(d_inode(old_dentry))->i_nlink);
	i_size_write(d_inode(new_dentry), file_size_save);
out:
	unlock_dir(lower_dir_dentry);
	boesfs_put_lower_path(old_dentry, &lower_old_path);
	boesfs_put_lower_path(new_dentry, &lower_new_path);
	return err;
}

static int boesfs_unlink(struct inode *dir, struct dentry *dentry)
{
	int err;
	struct dentry *lower_dentry;
	struct inode *lower_dir_inode = boesfs_lower_inode(dir);
	struct dentry *lower_dir_dentry;
	struct path lower_path;


	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	char * file_path = get_absolute_path_from_dentry_struct(dentry, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs unlink error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_UNLINK;
	memcpy(args.elfpath, BOESFS_SB(dir->i_sb)->elfpath, SINGLE_PATH_MAX);
	// unlink args
	memcpy(args.args.unlink_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(dir->i_sb)->dir_tree_root;
	
	err = boesfs_run_prog(BOESFS_SB(dir->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(dir->i_sb)->deny += 1;

	// log
	if(BOESFS_SB(dir->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(dir->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(dir->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(dir->i_sb)->deny > BOESFS_SB(dir->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(dir->i_sb)->pid);
	}

	if (err < 0)
		printk("error\n");

	vfree((void *)path_buf);

	if (err < 0)
		return err;
	/* VFS functions args hook END */

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;

	dget(lower_dentry);
	lower_dir_dentry = lock_parent(lower_dentry);

	err = vfs_unlink(lower_dir_inode, lower_dentry, NULL);

	/*
	 * Note: unlinking on top of NFS can cause silly-renamed files.
	 * Trying to delete such files results in EBUSY from NFS
	 * below.  Silly-renamed files will get deleted by NFS later on, so
	 * we just need to detect them here and treat such EBUSY errors as
	 * if the upper file was successfully deleted.
	 */
	if (err == -EBUSY && lower_dentry->d_flags & DCACHE_NFSFS_RENAMED)
		err = 0;
	if (err)
		goto out;
	fsstack_copy_attr_times(dir, lower_dir_inode);
	fsstack_copy_inode_size(dir, lower_dir_inode);
	set_nlink(d_inode(dentry),
		  boesfs_lower_inode(d_inode(dentry))->i_nlink);
	d_inode(dentry)->i_ctime = dir->i_ctime;
	d_drop(dentry); /* this is needed, else LTP fails (VFS won't do it) */
out:
	unlock_dir(lower_dir_dentry);
	dput(lower_dentry);
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static int boesfs_symlink(struct inode *dir, struct dentry *dentry,
			  const char *symname)
{
	int err;
	struct dentry *lower_dentry;
	struct dentry *lower_parent_dentry = NULL;
	struct path lower_path;

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	
	char * file_path1 = symname;
	char * file_path2 = get_absolute_path_from_dentry_struct(dentry, path_buf);

	if(file_path2 == NULL){
		printk("ERROR: get path from dentry in vfs link error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_SYMLINK;
	memcpy(args.elfpath, BOESFS_SB(dir->i_sb)->elfpath, SINGLE_PATH_MAX);
	// link args
	memcpy(args.args.symlink_args.path, file_path1, strlen(file_path1) + 1);
	memcpy(args.args.symlink_args.linkpath, file_path2, strlen(file_path2) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(dir->i_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(dir->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(dir->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(dir->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(dir->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(dir->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(dir->i_sb)->deny > BOESFS_SB(dir->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(dir->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;
	
	/* VFS functions args hook END */

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_parent_dentry = lock_parent(lower_dentry);

	err = vfs_symlink(d_inode(lower_parent_dentry), lower_dentry, symname);
	if (err)
		goto out;
	err = boesfs_interpose(dentry, dir->i_sb, &lower_path);
	if (err)
		goto out;
	fsstack_copy_attr_times(dir, boesfs_lower_inode(dir));
	fsstack_copy_inode_size(dir, d_inode(lower_parent_dentry));

out:
	unlock_dir(lower_parent_dentry);
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static int boesfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	int err;
	struct dentry *lower_dentry;
	struct dentry *lower_parent_dentry = NULL;
	struct path lower_path;

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	char * file_path = get_absolute_path_from_dentry_struct(dentry, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs mkdir error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_MKDIR;
	memcpy(args.elfpath, BOESFS_SB(dir->i_sb)->elfpath, SINGLE_PATH_MAX);
	// unlink args
	args.args.mkdir_args.mode = (int)mode;
	memcpy(args.args.mkdir_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(dir->i_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(dir->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(dir->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(dir->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(dir->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(dir->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(dir->i_sb)->deny > BOESFS_SB(dir->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(dir->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;
	
	/* VFS functions args hook END */
	



	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_parent_dentry = lock_parent(lower_dentry);

	err = vfs_mkdir(d_inode(lower_parent_dentry), lower_dentry, mode);
	if (err)
		goto out;

	err = boesfs_interpose(dentry, dir->i_sb, &lower_path);
	if (err)
		goto out;

	fsstack_copy_attr_times(dir, boesfs_lower_inode(dir));
	fsstack_copy_inode_size(dir, d_inode(lower_parent_dentry));
	/* update number of links on parent directory */
	set_nlink(dir, boesfs_lower_inode(dir)->i_nlink);

out:
	unlock_dir(lower_parent_dentry);
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static int boesfs_rmdir(struct inode *dir, struct dentry *dentry)
{
	struct dentry *lower_dentry;
	struct dentry *lower_dir_dentry;
	int err;
	struct path lower_path;

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	char * file_path = get_absolute_path_from_dentry_struct(dentry, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs rmdir error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_RMDIR;
	memcpy(args.elfpath, BOESFS_SB(dir->i_sb)->elfpath, SINGLE_PATH_MAX);
	// unlink args
	memcpy(args.args.rmdir_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(dir->i_sb)->dir_tree_root;
	
	err = boesfs_run_prog(BOESFS_SB(dir->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(dir->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(dir->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(dir->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(dir->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(dir->i_sb)->deny > BOESFS_SB(dir->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(dir->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;

	/* VFS functions args hook END */

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_dir_dentry = lock_parent(lower_dentry);

	err = vfs_rmdir(d_inode(lower_dir_dentry), lower_dentry);
	if (err)
		goto out;

	d_drop(dentry);	/* drop our dentry on success (why not VFS's job?) */
	if (d_inode(dentry))
		clear_nlink(d_inode(dentry));
	fsstack_copy_attr_times(dir, d_inode(lower_dir_dentry));
	fsstack_copy_inode_size(dir, d_inode(lower_dir_dentry));
	set_nlink(dir, d_inode(lower_dir_dentry)->i_nlink);

out:
	unlock_dir(lower_dir_dentry);
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static int boesfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode,
			dev_t dev)
{
	int err;
	struct dentry *lower_dentry;
	struct dentry *lower_parent_dentry = NULL;
	struct path lower_path;


	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	char * file_path = get_absolute_path_from_dentry_struct(dentry, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from dentry in vfs mknod error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_MKNOD;
	memcpy(args.elfpath, BOESFS_SB(dir->i_sb)->elfpath, SINGLE_PATH_MAX);
	// unlink args
	args.args.mknod_args.mode = (int)mode;
	args.args.mknod_args.dev = (int)dev;
	memcpy(args.args.mknod_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(dir->i_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(dir->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(dir->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(dir->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(dir->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(dir->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(dir->i_sb)->deny > BOESFS_SB(dir->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(dir->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;
	
	/* VFS functions args hook END */

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_parent_dentry = lock_parent(lower_dentry);

	err = vfs_mknod(d_inode(lower_parent_dentry), lower_dentry, mode, dev);
	if (err)
		goto out;

	err = boesfs_interpose(dentry, dir->i_sb, &lower_path);
	if (err)
		goto out;
	fsstack_copy_attr_times(dir, boesfs_lower_inode(dir));
	fsstack_copy_inode_size(dir, d_inode(lower_parent_dentry));

out:
	unlock_dir(lower_parent_dentry);
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

/*
 * The locking rules in boesfs_rename are complex.  We could use a simpler
 * superblock-level name-space lock for renames and copy-ups.
 */
static int boesfs_rename(struct inode *old_dir, struct dentry *old_dentry,
			 struct inode *new_dir, struct dentry *new_dentry,
			 unsigned int flags)
{
	int err = 0;
	struct dentry *lower_old_dentry = NULL;
	struct dentry *lower_new_dentry = NULL;
	struct dentry *lower_old_dir_dentry = NULL;
	struct dentry *lower_new_dir_dentry = NULL;
	struct dentry *trap = NULL;
	struct path lower_old_path, lower_new_path;

	if (flags)
		return -EINVAL;

	/* VFS functions args hook START */
	void *path_buf1 = vmalloc(PATH_MAX);
	void *path_buf2 = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf1)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	if (IS_ERR(path_buf2)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	
	char * file_path1 = get_absolute_path_from_dentry_struct(old_dentry, path_buf1);
	char * file_path2 = get_absolute_path_from_dentry_struct(new_dentry, path_buf2);

	if(file_path1 == NULL){
		printk("ERROR: get path from dentry in vfs rename error!");
		return err;
	}
	if(file_path2 == NULL){
		printk("ERROR: get path from dentry in vfs rename error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_RENAME;
	memcpy(args.elfpath, BOESFS_SB(old_dir->i_sb)->elfpath, SINGLE_PATH_MAX);
	// link args
	memcpy(args.args.rename_args.path, file_path1, strlen(file_path1) + 1);
	memcpy(args.args.rename_args.newpath, file_path2, strlen(file_path2) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(old_dir->i_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(old_dir->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(old_dir->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(old_dir->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(old_dir->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(old_dir->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(old_dir->i_sb)->deny > BOESFS_SB(old_dir->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(old_dir->i_sb)->pid);
	}

	vfree((void *)path_buf1);
	vfree((void *)path_buf2);

	if (err < 0)
		return err;
	
	/* VFS functions args hook END */

	boesfs_get_lower_path(old_dentry, &lower_old_path);
	boesfs_get_lower_path(new_dentry, &lower_new_path);
	lower_old_dentry = lower_old_path.dentry;
	lower_new_dentry = lower_new_path.dentry;
	lower_old_dir_dentry = dget_parent(lower_old_dentry);
	lower_new_dir_dentry = dget_parent(lower_new_dentry);

	trap = lock_rename(lower_old_dir_dentry, lower_new_dir_dentry);
	/* source should not be ancestor of target */
	if (trap == lower_old_dentry) {
		err = -EINVAL;
		goto out;
	}
	/* target should not be ancestor of source */
	if (trap == lower_new_dentry) {
		err = -ENOTEMPTY;
		goto out;
	}

	err = vfs_rename(d_inode(lower_old_dir_dentry), lower_old_dentry,
			 d_inode(lower_new_dir_dentry), lower_new_dentry,
			 NULL, 0);
	if (err)
		goto out;

	fsstack_copy_attr_all(new_dir, d_inode(lower_new_dir_dentry));
	fsstack_copy_inode_size(new_dir, d_inode(lower_new_dir_dentry));
	if (new_dir != old_dir) {
		fsstack_copy_attr_all(old_dir,
				      d_inode(lower_old_dir_dentry));
		fsstack_copy_inode_size(old_dir,
					d_inode(lower_old_dir_dentry));
	}

out:
	unlock_rename(lower_old_dir_dentry, lower_new_dir_dentry);
	dput(lower_old_dir_dentry);
	dput(lower_new_dir_dentry);
	boesfs_put_lower_path(old_dentry, &lower_old_path);
	boesfs_put_lower_path(new_dentry, &lower_new_path);
	return err;
}

static const char *boesfs_get_link(struct dentry *dentry, struct inode *inode,
				   struct delayed_call *done)
{
	DEFINE_DELAYED_CALL(lower_done);
	struct dentry *lower_dentry;
	struct path lower_path;
	char *buf;
	const char *lower_link;

	if (!dentry)
		return ERR_PTR(-ECHILD);

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;

	/*
	 * get link from lower file system, but use a separate
	 * delayed_call callback.
	 */
	lower_link = vfs_get_link(lower_dentry, &lower_done);
	if (IS_ERR(lower_link)) {
		buf = ERR_CAST(lower_link);
		goto out;
	}

	/*
	 * we can't pass lower link up: have to make private copy and
	 * pass that.
	 */
	buf = kstrdup(lower_link, GFP_KERNEL);
	do_delayed_call(&lower_done);
	if (!buf) {
		buf = ERR_PTR(-ENOMEM);
		goto out;
	}

	fsstack_copy_attr_atime(d_inode(dentry), d_inode(lower_dentry));

	set_delayed_call(done, kfree_link, buf);
out:
	boesfs_put_lower_path(dentry, &lower_path);
	return buf;
}

static int boesfs_permission(struct inode *inode, int mask)
{
	struct inode *lower_inode;
	int err;

	lower_inode = boesfs_lower_inode(inode);
	err = inode_permission(lower_inode, mask);
	return err;
}

static int boesfs_setattr(struct dentry *dentry, struct iattr *ia)
{
	int err;
	struct dentry *lower_dentry;
	struct inode *inode;
	struct inode *lower_inode;
	struct path lower_path;
	struct iattr lower_ia;

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	char * file_path = get_absolute_path_from_dentry_struct(dentry, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from dentry in vfs setattr error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_SETATTR;
	memcpy(args.elfpath, BOESFS_SB(dentry->d_sb)->elfpath, SINGLE_PATH_MAX);
	// setattr args
	args.args.setattr_args.mode = (int)(ia->ia_mode);
	args.args.setattr_args.uid = (int)(ia->ia_uid.val);
	args.args.setattr_args.gid = (int)(ia->ia_gid.val);
	memcpy(args.args.setattr_args.path, file_path, strlen(file_path) + 1);
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
		return err;
	
	/* VFS functions args hook END */

	inode = d_inode(dentry);

	/*
	 * Check if user has permission to change inode.  We don't check if
	 * this user can change the lower inode: that should happen when
	 * calling notify_change on the lower inode.
	 */
	err = setattr_prepare(dentry, ia);
	if (err)
		goto out_err;

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_inode = boesfs_lower_inode(inode);

	/* prepare our own lower struct iattr (with the lower file) */
	memcpy(&lower_ia, ia, sizeof(lower_ia));
	if (ia->ia_valid & ATTR_FILE)
		lower_ia.ia_file = boesfs_lower_file(ia->ia_file);

	/*
	 * If shrinking, first truncate upper level to cancel writing dirty
	 * pages beyond the new eof; and also if its' maxbytes is more
	 * limiting (fail with -EFBIG before making any change to the lower
	 * level).  There is no need to vmtruncate the upper level
	 * afterwards in the other cases: we fsstack_copy_inode_size from
	 * the lower level.
	 */
	if (ia->ia_valid & ATTR_SIZE) {
		err = inode_newsize_ok(inode, ia->ia_size);
		if (err)
			goto out;
		truncate_setsize(inode, ia->ia_size);
	}

	/*
	 * mode change is for clearing setuid/setgid bits. Allow lower fs
	 * to interpret this in its own way.
	 */
	if (lower_ia.ia_valid & (ATTR_KILL_SUID | ATTR_KILL_SGID))
		lower_ia.ia_valid &= ~ATTR_MODE;

	/* notify the (possibly copied-up) lower inode */
	/*
	 * Note: we use d_inode(lower_dentry), because lower_inode may be
	 * unlinked (no inode->i_sb and i_ino==0.  This happens if someone
	 * tries to open(), unlink(), then ftruncate() a file.
	 */
	inode_lock(d_inode(lower_dentry));
	err = notify_change(lower_dentry, &lower_ia, /* note: lower_ia */
			    NULL);
	inode_unlock(d_inode(lower_dentry));
	if (err)
		goto out;

	/* get attributes from the lower inode */
	fsstack_copy_attr_all(inode, lower_inode);
	/*
	 * Not running fsstack_copy_inode_size(inode, lower_inode), because
	 * VFS should update our inode size, and notify_change on
	 * lower_inode should update its size.
	 */

out:
	boesfs_put_lower_path(dentry, &lower_path);
out_err:
	return err;
}

static int boesfs_getattr(const struct path *path, struct kstat *stat, 
                          u32 request_mask, unsigned int flags)
{
	int err;
    struct dentry *dentry = path->dentry;
	struct kstat lower_stat;
	struct path lower_path;

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}
	char * file_path = get_absolute_path_from_dentry_struct(dentry, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from dentry in vfs getattr error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_GETATTR;
	memcpy(args.elfpath, BOESFS_SB(dentry->d_sb)->elfpath, SINGLE_PATH_MAX);
	// getattr args
	memcpy(args.args.getattr_args.path, file_path, strlen(file_path) + 1);
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
		return err;
	
	/* VFS functions args hook END */

	boesfs_get_lower_path(dentry, &lower_path);
	err = vfs_getattr(&lower_path, &lower_stat, request_mask, flags);
	if (err)
		goto out;
	fsstack_copy_attr_all(d_inode(dentry),
			      d_inode(lower_path.dentry));
	generic_fillattr(d_inode(dentry), stat);
	stat->blocks = lower_stat.blocks;
out:
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static int
boesfs_setxattr(struct dentry *dentry, struct inode *inode, const char *name,
		const void *value, size_t size, int flags)
{
	int err; struct dentry *lower_dentry;
	struct path lower_path;

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	if (!(d_inode(lower_dentry)->i_opflags & IOP_XATTR)) {
		err = -EOPNOTSUPP;
		goto out;
	}
	err = vfs_setxattr(lower_dentry, name, value, size, flags);
	if (err)
		goto out;
	fsstack_copy_attr_all(d_inode(dentry),
			      d_inode(lower_path.dentry));
out:
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static ssize_t
boesfs_getxattr(struct dentry *dentry, struct inode *inode,
		const char *name, void *buffer, size_t size)
{
	int err;
	struct dentry *lower_dentry;
	struct inode *lower_inode;
	struct path lower_path;

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_inode = boesfs_lower_inode(inode);
	if (!(d_inode(lower_dentry)->i_opflags & IOP_XATTR)) {
		err = -EOPNOTSUPP;
		goto out;
	}
	err = vfs_getxattr(lower_dentry, name, buffer, size);
	if (err)
		goto out;
	fsstack_copy_attr_atime(d_inode(dentry),
				d_inode(lower_path.dentry));
out:
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static ssize_t
boesfs_listxattr(struct dentry *dentry, char *buffer, size_t buffer_size)
{
	int err;
	struct dentry *lower_dentry;
	struct path lower_path;

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	if (!(d_inode(lower_dentry)->i_opflags & IOP_XATTR)) {
		err = -EOPNOTSUPP;
		goto out;
	}
	err = vfs_listxattr(lower_dentry, buffer, buffer_size);
	if (err)
		goto out;
	fsstack_copy_attr_atime(d_inode(dentry),
				d_inode(lower_path.dentry));
out:
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

static int
boesfs_removexattr(struct dentry *dentry, struct inode *inode, const char *name)
{
	int err;
	struct dentry *lower_dentry;
	struct inode *lower_inode;
	struct path lower_path;

	boesfs_get_lower_path(dentry, &lower_path);
	lower_dentry = lower_path.dentry;
	lower_inode = boesfs_lower_inode(inode);
	if (!(lower_inode->i_opflags & IOP_XATTR)) {
		err = -EOPNOTSUPP;
		goto out;
	}
	err = vfs_removexattr(lower_dentry, name);
	if (err)
		goto out;
	fsstack_copy_attr_all(d_inode(dentry), lower_inode);
out:
	boesfs_put_lower_path(dentry, &lower_path);
	return err;
}

const struct inode_operations boesfs_symlink_iops = {
	.permission	= boesfs_permission,
	.setattr	= boesfs_setattr,
	.getattr	= boesfs_getattr,
	.get_link	= boesfs_get_link,
	.listxattr	= boesfs_listxattr,
};

const struct inode_operations boesfs_dir_iops = {
	.create		= boesfs_create,
	.lookup		= boesfs_lookup,
	.link		= boesfs_link,
	.unlink		= boesfs_unlink,
	.symlink	= boesfs_symlink,
	.mkdir		= boesfs_mkdir,
	.rmdir		= boesfs_rmdir,
	.mknod		= boesfs_mknod,
	.rename		= boesfs_rename,
	.permission	= boesfs_permission,
	.setattr	= boesfs_setattr,
	.getattr	= boesfs_getattr,
	.listxattr	= boesfs_listxattr,
};

const struct inode_operations boesfs_main_iops = {
	.permission	= boesfs_permission,
	.setattr	= boesfs_setattr,
	.getattr	= boesfs_getattr,
	.listxattr	= boesfs_listxattr,
};

static int boesfs_xattr_get(const struct xattr_handler *handler,
			    struct dentry *dentry, struct inode *inode,
			    const char *name, void *buffer, size_t size)
{
	return boesfs_getxattr(dentry, inode, name, buffer, size);
}

static int boesfs_xattr_set(const struct xattr_handler *handler,
			    struct dentry *dentry, struct inode *inode,
			    const char *name, const void *value, size_t size,
			    int flags)
{
	if (value)
		return boesfs_setxattr(dentry, inode, name, value, size, flags);

	BUG_ON(flags != XATTR_REPLACE);
	return boesfs_removexattr(dentry, inode, name);
}

const struct xattr_handler boesfs_xattr_handler = {
	.prefix = "",		/* match anything */
	.get = boesfs_xattr_get,
	.set = boesfs_xattr_set,
};

const struct xattr_handler *boesfs_xattr_handlers[] = {
	&boesfs_xattr_handler,
	NULL
};
