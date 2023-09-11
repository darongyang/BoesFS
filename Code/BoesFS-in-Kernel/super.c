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
 * The inode cache is used with alloc_inode for both our inode info and the
 * vfs inode.
 */
static struct kmem_cache *boesfs_inode_cachep;

/* final actions when unmounting a file system */
static void boesfs_put_super(struct super_block *sb)
{
	struct boesfs_sb_info *spd;
	struct super_block *s;

	spd = BOESFS_SB(sb);
	if (!spd)
		return;

	/* decrement lower super references */
	s = boesfs_lower_super(sb);
	boesfs_set_lower_super(sb, NULL);
	atomic_dec(&s->s_active);

	kfree(spd);
	sb->s_fs_info = NULL;
}

static int boesfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
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
		printk("ERROR: get path from dentry in vfs statfs error!");
		return err;
	}
	

	// boesfs args
	struct boesfs_args args;
	args.op = BOESFS_STATFS;
	memcpy(args.elfpath, BOESFS_SB(dentry->d_sb)->elfpath, SINGLE_PATH_MAX);
	// statfs args
	memcpy(args.args.statfs_args.path, file_path, strlen(file_path) + 1);

	//  dir tree root
	args.dir_tree_root = BOESFS_SB(dentry->d_sb)->dir_tree_root;

	err = boesfs_run_prog(BOESFS_SB(dentry->d_sb)->prog_pos, &args);
	
	// 记录拒绝次数
	if(err < 0)
		BOESFS_SB(dentry->d_sb)->deny += 1;
	
	// log
	if(BOESFS_SB(dentry->d_sb)->log==1){
		// 输出审计日志
		boesfs_log(&args, (char*)(BOESFS_SB(dentry->d_sb)->logpath), err);
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
	err = vfs_statfs(&lower_path, buf);
	boesfs_put_lower_path(dentry, &lower_path);

	/* set return buf to our f/s to avoid confusing user-level utils */
	buf->f_type = BOESFS_SUPER_MAGIC;

	return err;
}

/*
 * @flags: numeric mount options
 * @options: mount options string
 */
static int boesfs_remount_fs(struct super_block *sb, int *flags, char *options)
{
	int err = 0;

	/*
	 * The VFS will take care of "ro" and "rw" flags among others.  We
	 * can safely accept a few flags (RDONLY, MANDLOCK), and honor
	 * SILENT, but anything else left over is an error.
	 */
	if ((*flags & ~(MS_RDONLY | MS_MANDLOCK | MS_SILENT)) != 0) {
		printk(KERN_ERR
		       "boesfs: remount flags 0x%x unsupported\n", *flags);
		err = -EINVAL;
	}

	return err;
}

/*
 * Called by iput() when the inode reference count reached zero
 * and the inode is not hashed anywhere.  Used to clear anything
 * that needs to be, before the inode is completely destroyed and put
 * on the inode free list.
 */
static void boesfs_evict_inode(struct inode *inode)
{
	struct inode *lower_inode;

	truncate_inode_pages(&inode->i_data, 0);
	clear_inode(inode);
	/*
	 * Decrement a reference to a lower_inode, which was incremented
	 * by our read_inode when it was created initially.
	 */
	lower_inode = boesfs_lower_inode(inode);
	boesfs_set_lower_inode(inode, NULL);
	iput(lower_inode);
}

static struct inode *boesfs_alloc_inode(struct super_block *sb)
{
	struct boesfs_inode_info *i;

	i = kmem_cache_alloc(boesfs_inode_cachep, GFP_KERNEL);
	if (!i)
		return NULL;

	/* memset everything up to the inode to 0 */
	memset(i, 0, offsetof(struct boesfs_inode_info, vfs_inode));

	i->vfs_inode.i_version = 1;
	return &i->vfs_inode;
}

static void boesfs_destroy_inode(struct inode *inode)
{
	kmem_cache_free(boesfs_inode_cachep, BOESFS_I(inode));
}

/* boesfs inode cache constructor */
static void init_once(void *obj)
{
	struct boesfs_inode_info *i = obj;

	inode_init_once(&i->vfs_inode);
}

int boesfs_init_inode_cache(void)
{
	int err = 0;

	boesfs_inode_cachep =
		kmem_cache_create("boesfs_inode_cache",
				  sizeof(struct boesfs_inode_info), 0,
				  SLAB_RECLAIM_ACCOUNT, init_once);
	if (!boesfs_inode_cachep)
		err = -ENOMEM;
	return err;
}

/* boesfs inode cache destructor */
void boesfs_destroy_inode_cache(void)
{
	if (boesfs_inode_cachep)
		kmem_cache_destroy(boesfs_inode_cachep);
}

/*
 * Used only in nfs, to kill any pending RPC tasks, so that subsequent
 * code can actually succeed and won't leave tasks that need handling.
 */
static void boesfs_umount_begin(struct super_block *sb)
{
	struct super_block *lower_sb;

	lower_sb = boesfs_lower_super(sb);
	if (lower_sb && lower_sb->s_op && lower_sb->s_op->umount_begin)
		lower_sb->s_op->umount_begin(lower_sb);
}

const struct super_operations boesfs_sops = {
	.put_super	= boesfs_put_super,
	.statfs		= boesfs_statfs,
	.remount_fs	= boesfs_remount_fs,
	.evict_inode	= boesfs_evict_inode,
	.umount_begin	= boesfs_umount_begin,
	.show_options	= generic_show_options,
	.alloc_inode	= boesfs_alloc_inode,
	.destroy_inode	= boesfs_destroy_inode,
	.drop_inode	= generic_delete_inode,
};

/* NFS support */

static struct inode *boesfs_nfs_get_inode(struct super_block *sb, u64 ino,
					  u32 generation)
{
	struct super_block *lower_sb;
	struct inode *inode;
	struct inode *lower_inode;

	lower_sb = boesfs_lower_super(sb);
	lower_inode = ilookup(lower_sb, ino);
	inode = boesfs_iget(sb, lower_inode);
	return inode;
}

static struct dentry *boesfs_fh_to_dentry(struct super_block *sb,
					  struct fid *fid, int fh_len,
					  int fh_type)
{
	return generic_fh_to_dentry(sb, fid, fh_len, fh_type,
				    boesfs_nfs_get_inode);
}

static struct dentry *boesfs_fh_to_parent(struct super_block *sb,
					  struct fid *fid, int fh_len,
					  int fh_type)
{
	return generic_fh_to_parent(sb, fid, fh_len, fh_type,
				    boesfs_nfs_get_inode);
}

/*
 * all other funcs are default as defined in exportfs/expfs.c
 */

const struct export_operations boesfs_export_ops = {
	.fh_to_dentry	   = boesfs_fh_to_dentry,
	.fh_to_parent	   = boesfs_fh_to_parent
};
