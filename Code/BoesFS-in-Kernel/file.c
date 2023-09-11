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

static ssize_t boesfs_read(struct file *file, char __user *buf,
			   size_t count, loff_t *ppos)
{
	int err;
	struct file *lower_file;
	struct dentry *dentry = file->f_path.dentry;

	lower_file = boesfs_lower_file(file);

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}

	char * file_path = get_absolute_path_from_file_struct(file, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs read error!");
		return err;
	}
	

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_READ;
	memcpy(args.elfpath, BOESFS_SB(file->f_inode->i_sb)->elfpath, SINGLE_PATH_MAX);
	// read args
	args.args.read_args.len = (int)count;
	args.args.read_args.offset = (int)*ppos;
	memcpy(args.args.read_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(file->f_inode->i_sb)->dir_tree_root;
	
	err = boesfs_run_prog(BOESFS_SB(file->f_inode->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(file->f_inode->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(file->f_inode->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char*)(BOESFS_SB(file->f_inode->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(file->f_inode->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(file->f_inode->i_sb)->deny > BOESFS_SB(file->f_inode->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(file->f_inode->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;

	/* VFS functions args hook END */

	err = vfs_read(lower_file, buf, count, ppos);
	/* update our inode atime upon a successful lower read */
	if (err >= 0)
		fsstack_copy_attr_atime(d_inode(dentry),
					file_inode(lower_file));

	return err;
}

static ssize_t boesfs_write(struct file *file, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	int err;

	struct file *lower_file;
	struct dentry *dentry = file->f_path.dentry;

	lower_file = boesfs_lower_file(file);

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}

	char * file_path = get_absolute_path_from_file_struct(file, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs write error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_WRITE;
	memcpy(args.elfpath, BOESFS_SB(file->f_inode->i_sb)->elfpath, SINGLE_PATH_MAX);
	// write args
	args.args.write_args.len = (int)count;
	args.args.write_args.offset = (int)*ppos;
	memcpy(args.args.write_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(file->f_inode->i_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(file->f_inode->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(file->f_inode->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(file->f_inode->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(file->f_inode->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(file->f_inode->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(file->f_inode->i_sb)->deny > BOESFS_SB(file->f_inode->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(file->f_inode->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;
	/* VFS functions args hook END */

	err = vfs_write(lower_file, buf, count, ppos);
	/* update our inode times+sizes upon a successful lower write */
	if (err >= 0) {
		fsstack_copy_inode_size(d_inode(dentry),
					file_inode(lower_file));
		fsstack_copy_attr_times(d_inode(dentry),
					file_inode(lower_file));
	}

	return err;
}

static int boesfs_readdir(struct file *file, struct dir_context *ctx)
{
	int err;
	struct file *lower_file = NULL;
	struct dentry *dentry = file->f_path.dentry;

	lower_file = boesfs_lower_file(file);

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}

	char * file_path = get_absolute_path_from_file_struct(file, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs write error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_ITERATE;
	memcpy(args.elfpath, BOESFS_SB(file->f_inode->i_sb)->elfpath, SINGLE_PATH_MAX);
	// iterate args
	memcpy(args.args.iterate_args.path, file_path, strlen(file_path) + 1);

	//  dir tree root
	args.dir_tree_root = BOESFS_SB(file->f_inode->i_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(file->f_inode->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(file->f_inode->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(file->f_inode->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(file->f_inode->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(file->f_inode->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(file->f_inode->i_sb)->deny > BOESFS_SB(file->f_inode->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(file->f_inode->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;

	/* VFS functions args hook END */


	err = iterate_dir(lower_file, ctx);
	file->f_pos = lower_file->f_pos;
	if (err >= 0)		/* copy the atime */
		fsstack_copy_attr_atime(d_inode(dentry),
					file_inode(lower_file));
	return err;
}

static long boesfs_unlocked_ioctl(struct file *file, unsigned int cmd,
				  unsigned long arg)
{
	long err = -ENOTTY;
	struct file *lower_file;

	lower_file = boesfs_lower_file(file);

	/* XXX: use vfs_ioctl if/when VFS exports it */
	if (!lower_file || !lower_file->f_op)
		goto out;
	if (lower_file->f_op->unlocked_ioctl)
		err = lower_file->f_op->unlocked_ioctl(lower_file, cmd, arg);

	/* some ioctls can change inode attributes (EXT2_IOC_SETFLAGS) */
	if (!err)
		fsstack_copy_attr_all(file_inode(file),
				      file_inode(lower_file));
out:
	return err;
}

#ifdef CONFIG_COMPAT
static long boesfs_compat_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	long err = -ENOTTY;
	struct file *lower_file;

	lower_file = boesfs_lower_file(file);

	/* XXX: use vfs_ioctl if/when VFS exports it */
	if (!lower_file || !lower_file->f_op)
		goto out;
	if (lower_file->f_op->compat_ioctl)
		err = lower_file->f_op->compat_ioctl(lower_file, cmd, arg);

out:
	return err;
}
#endif

static int boesfs_mmap(struct file *file, struct vm_area_struct *vma)
{
	int err = 0;
	bool willwrite;
	struct file *lower_file;
	const struct vm_operations_struct *saved_vm_ops = NULL;

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}

	char * file_path = get_absolute_path_from_file_struct(file, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs read error!");
		return err;
	}
	

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_MMAP;
	memcpy(args.elfpath, BOESFS_SB(file->f_inode->i_sb)->elfpath, SINGLE_PATH_MAX);
	// mmap args
	args.args.mmap_args.startaddr = vma->vm_start;
	args.args.mmap_args.endaddr = vma->vm_end;;
	memcpy(args.args.mmap_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(file->f_inode->i_sb)->dir_tree_root;
	
	err = boesfs_run_prog(BOESFS_SB(file->f_inode->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(file->f_inode->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(file->f_inode->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char*)(BOESFS_SB(file->f_inode->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(file->f_inode->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(file->f_inode->i_sb)->deny > BOESFS_SB(file->f_inode->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(file->f_inode->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;

	/* VFS functions args hook END */

	/* this might be deferred to mmap's writepage */
	willwrite = ((vma->vm_flags | VM_SHARED | VM_WRITE) == vma->vm_flags);

	/*
	 * File systems which do not implement ->writepage may use
	 * generic_file_readonly_mmap as their ->mmap op.  If you call
	 * generic_file_readonly_mmap with VM_WRITE, you'd get an -EINVAL.
	 * But we cannot call the lower ->mmap op, so we can't tell that
	 * writeable mappings won't work.  Therefore, our only choice is to
	 * check if the lower file system supports the ->writepage, and if
	 * not, return EINVAL (the same error that
	 * generic_file_readonly_mmap returns in that case).
	 */
	lower_file = boesfs_lower_file(file);
	if (willwrite && !lower_file->f_mapping->a_ops->writepage) {
		err = -EINVAL;
		printk(KERN_ERR "boesfs: lower file system does not "
		       "support writeable mmap\n");
		goto out;
	}

	/*
	 * find and save lower vm_ops.
	 *
	 * XXX: the VFS should have a cleaner way of finding the lower vm_ops
	 */
	if (!BOESFS_F(file)->lower_vm_ops) {
		err = lower_file->f_op->mmap(lower_file, vma);
		if (err) {
			printk(KERN_ERR "boesfs: lower mmap failed %d\n", err);
			goto out;
		}
		saved_vm_ops = vma->vm_ops; /* save: came from lower ->mmap */
	}

	/*
	 * Next 3 lines are all I need from generic_file_mmap.  I definitely
	 * don't want its test for ->readpage which returns -ENOEXEC.
	 */
	file_accessed(file);
	vma->vm_ops = &boesfs_vm_ops;

	file->f_mapping->a_ops = &boesfs_aops; /* set our aops */
	if (!BOESFS_F(file)->lower_vm_ops) /* save for our ->fault */
		BOESFS_F(file)->lower_vm_ops = saved_vm_ops;

out:
	return err;
}

static int boesfs_open(struct inode *inode, struct file *file)
{
	int err = 0;
	struct file *lower_file = NULL;
	struct path lower_path;

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}

	char * file_path = get_absolute_path_from_file_struct(file, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs open error!");
		return err;
	}

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_OPEN;
	memcpy(args.elfpath, BOESFS_SB(file->f_inode->i_sb)->elfpath, SINGLE_PATH_MAX);
	// open args
	memcpy(args.args.open_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(file->f_inode->i_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(file->f_inode->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(file->f_inode->i_sb)->deny += 1;
	// log
	if(BOESFS_SB(file->f_inode->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char *)(BOESFS_SB(file->f_inode->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(file->f_inode->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(file->f_inode->i_sb)->deny > BOESFS_SB(file->f_inode->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(file->f_inode->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;
	/* VFS functions args hook END */



	/* don't open unhashed/deleted files */
	if (d_unhashed(file->f_path.dentry)) {
		err = -ENOENT;
		goto out_err;
	}

	file->private_data =
		kzalloc(sizeof(struct boesfs_file_info), GFP_KERNEL);
	if (!BOESFS_F(file)) {
		err = -ENOMEM;
		goto out_err;
	}

	/* open lower object and link boesfs's file struct to lower's */
	boesfs_get_lower_path(file->f_path.dentry, &lower_path);
	lower_file = dentry_open(&lower_path, file->f_flags, current_cred());
	path_put(&lower_path);
	if (IS_ERR(lower_file)) {
		err = PTR_ERR(lower_file);
		lower_file = boesfs_lower_file(file);
		if (lower_file) {
			boesfs_set_lower_file(file, NULL);
			fput(lower_file); /* fput calls dput for lower_dentry */
		}
	} else {
		boesfs_set_lower_file(file, lower_file);
	}

	if (err)
		kfree(BOESFS_F(file));
	else
		fsstack_copy_attr_all(inode, boesfs_lower_inode(inode));
out_err:
	return err;
}

static int boesfs_flush(struct file *file, fl_owner_t id)
{
	int err = 0;
	struct file *lower_file = NULL;

	lower_file = boesfs_lower_file(file);
	if (lower_file && lower_file->f_op && lower_file->f_op->flush) {
		filemap_write_and_wait(file->f_mapping);
		err = lower_file->f_op->flush(lower_file, id);
	}

	return err;
}

/* release all lower object references & free the file info structure */
static int boesfs_file_release(struct inode *inode, struct file *file)
{
	struct file *lower_file;

	lower_file = boesfs_lower_file(file);
	if (lower_file) {
		boesfs_set_lower_file(file, NULL);
		fput(lower_file);
	}

	kfree(BOESFS_F(file));
	return 0;
}

static int boesfs_fsync(struct file *file, loff_t start, loff_t end,
			int datasync)
{
	int err;
	struct file *lower_file;
	struct path lower_path;
	struct dentry *dentry = file->f_path.dentry;

	// printk("test : %d\n", datasync);

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}

	char * file_path = get_absolute_path_from_file_struct(file, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs read error!");
		return err;
	}
	

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_FSYNC;
	memcpy(args.elfpath, BOESFS_SB(file->f_inode->i_sb)->elfpath, SINGLE_PATH_MAX);
	// fsync args
	args.args.fsync_args.len = datasync;
	memcpy(args.args.fsync_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(file->f_inode->i_sb)->dir_tree_root;
	
	err = boesfs_run_prog(BOESFS_SB(file->f_inode->i_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(file->f_inode->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(file->f_inode->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char*)(BOESFS_SB(file->f_inode->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(file->f_inode->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(file->f_inode->i_sb)->deny > BOESFS_SB(file->f_inode->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(file->f_inode->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;

	/* VFS functions args hook END */

	err = __generic_file_fsync(file, start, end, datasync);
	if (err)
		goto out;
	lower_file = boesfs_lower_file(file);
	boesfs_get_lower_path(dentry, &lower_path);
	err = vfs_fsync_range(lower_file, start, end, datasync);
	boesfs_put_lower_path(dentry, &lower_path);
out:
	return err;
}

static int boesfs_fasync(int fd, struct file *file, int flag)
{
	int err = 0;
	struct file *lower_file = NULL;

	lower_file = boesfs_lower_file(file);
	if (lower_file->f_op && lower_file->f_op->fasync)
		err = lower_file->f_op->fasync(fd, lower_file, flag);

	return err;
}

/*
 * Boesfs cannot use generic_file_llseek as ->llseek, because it would
 * only set the offset of the upper file.  So we have to implement our
 * own method to set both the upper and lower file offsets
 * consistently.
 */
static loff_t boesfs_file_llseek(struct file *file, loff_t offset, int whence)
{
	int err;
	struct file *lower_file;

	/* VFS functions args hook START */
	void *path_buf = vmalloc(PATH_MAX);
	if (IS_ERR(path_buf)) {
		printk("Error: Failed to alloc memory for file path\n");
		return err;
	}

	char * file_path = get_absolute_path_from_file_struct(file, path_buf);
	if(file_path == NULL){
		printk("ERROR: get path from file in vfs read error!");
		return err;
	}
	

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_LLSEEK;
	memcpy(args.elfpath, BOESFS_SB(file->f_inode->i_sb)->elfpath, SINGLE_PATH_MAX);
	// llseek args
	args.args.llseek_args.offset = (int)offset;
	args.args.llseek_args.whence = (int)whence;
	memcpy(args.args.llseek_args.path, file_path, strlen(file_path) + 1);
	//  dir tree root
	args.dir_tree_root = BOESFS_SB(file->f_inode->i_sb)->dir_tree_root;
	
	err = boesfs_run_prog(BOESFS_SB(file->f_inode->i_sb)->prog_pos, &args);

	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(file->f_inode->i_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(file->f_inode->i_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char*)(BOESFS_SB(file->f_inode->i_sb)->logpath), err);
	}

	// kill
	if(err < 0 && BOESFS_SB(file->f_inode->i_sb)->kill!=0){
		// 比较kill 和 deny并杀死进程
		if(BOESFS_SB(file->f_inode->i_sb)->deny > BOESFS_SB(file->f_inode->i_sb)->kill)
			boesfs_kill((pid_t)BOESFS_SB(file->f_inode->i_sb)->pid);
	}

	vfree((void *)path_buf);

	if (err < 0)
		return err;

	/* VFS functions args hook END */

	err = generic_file_llseek(file, offset, whence);
	if (err < 0)
		goto out;

	lower_file = boesfs_lower_file(file);
	err = generic_file_llseek(lower_file, offset, whence);

out:
	return err;
}

/*
 * Boesfs read_iter, redirect modified iocb to lower read_iter
 */
ssize_t
boesfs_read_iter(struct kiocb *iocb, struct iov_iter *iter)
{
	int err;
	struct file *file = iocb->ki_filp, *lower_file;

	lower_file = boesfs_lower_file(file);
	if (!lower_file->f_op->read_iter) {
		err = -EINVAL;
		goto out;
	}

	get_file(lower_file); /* prevent lower_file from being released */
	iocb->ki_filp = lower_file;
	err = lower_file->f_op->read_iter(iocb, iter);
	iocb->ki_filp = file;
	fput(lower_file);
	/* update upper inode atime as needed */
	if (err >= 0 || err == -EIOCBQUEUED)
		fsstack_copy_attr_atime(d_inode(file->f_path.dentry),
					file_inode(lower_file));
out:
	return err;
}

/*
 * Boesfs write_iter, redirect modified iocb to lower write_iter
 */
ssize_t
boesfs_write_iter(struct kiocb *iocb, struct iov_iter *iter)
{
	int err;
	struct file *file = iocb->ki_filp, *lower_file;

	lower_file = boesfs_lower_file(file);
	if (!lower_file->f_op->write_iter) {
		err = -EINVAL;
		goto out;
	}

	get_file(lower_file); /* prevent lower_file from being released */
	iocb->ki_filp = lower_file;
	err = lower_file->f_op->write_iter(iocb, iter);
	iocb->ki_filp = file;
	fput(lower_file);
	/* update upper inode times/sizes as needed */
	if (err >= 0 || err == -EIOCBQUEUED) {
		fsstack_copy_inode_size(d_inode(file->f_path.dentry),
					file_inode(lower_file));
		fsstack_copy_attr_times(d_inode(file->f_path.dentry),
					file_inode(lower_file));
	}
out:
	return err;
}

const struct file_operations boesfs_main_fops = {
	.llseek		= boesfs_file_llseek,
	.read		= boesfs_read,
	.write		= boesfs_write,
	.unlocked_ioctl	= boesfs_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= boesfs_compat_ioctl,
#endif
	.mmap		= boesfs_mmap,
	.open		= boesfs_open,
	.flush		= boesfs_flush,
	.release	= boesfs_file_release,
	.fsync		= boesfs_fsync,
	.fasync		= boesfs_fasync,
	.read_iter	= boesfs_read_iter,
	.write_iter	= boesfs_write_iter,
};

/* trimmed directory options */
const struct file_operations boesfs_dir_fops = {
	.llseek		= boesfs_file_llseek,
	.read		= generic_read_dir,
	.iterate	= boesfs_readdir,
	.unlocked_ioctl	= boesfs_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= boesfs_compat_ioctl,
#endif
	.open		= boesfs_open,
	.release	= boesfs_file_release,
	.flush		= boesfs_flush,
	.fsync		= boesfs_fsync,
	.fasync		= boesfs_fasync,
};
