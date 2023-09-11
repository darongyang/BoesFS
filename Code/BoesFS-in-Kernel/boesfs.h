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

#ifndef _BOESFS_H_
#define _BOESFS_H_

#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/seq_file.h>
#include <linux/statfs.h>
#include <linux/fs_stack.h>
#include <linux/magic.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/xattr.h>
#include <linux/exportfs.h>
#include <linux/types.h>

/* malloc */
#include <linux/vmalloc.h>

/* for log */
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/kernel.h>
#include <linux/kthread.h> /* kernel thread */
#include <linux/spinlock.h> /* spin_lock */

/* for LKM support */
#include <linux/kallsyms.h>
#include <linux/kprobes.h> /* kprobe */

/* args */
#include "ebpf_helper.h"


#define ERR -1

/* for log.c */
#define MESSAGE_MAX 128
#define LOG_PATH_MAX 64

/* for rlimit.c */
#define RLIMIT_MAX 4096
#define LIMIT_INFO_MAX 64

/* for parse.c */
// "fd=%d,pid=%d,kill=%d,log=%d,logpath=%s,elfpath=%s"
#define INT_NUM 4
#define PATH_NUM 2
#define DATA_TYPES_NUM (INT_NUM + PATH_NUM) 
#define SINGLE_INT_MAX 16
#define SINGLE_PATH_MAX 256
#define RAW_DATA_MAX (SINGLE_INT_MAX * INT_NUM + SINGLE_PATH_MAX * PATH_NUM)


/* the file system name */
#define BOESFS_NAME "boesfs"
#define BOESFS_SUPER_MAGIC 0xb550ca20
#define BOESFS_VERSION "4.11"

/* boesfs root inode number */
#define BOESFS_ROOT_INO     1

/* useful for tracking code reachability */
#define UDBG printk(KERN_DEFAULT "DBG:%s:%s:%d\n", __FILE__, __func__, __LINE__)

/* struct dir_node prototype */
struct dir_node;

/* operations vectors defined in specific files */
extern const struct file_operations boesfs_main_fops;
extern const struct file_operations boesfs_dir_fops;
extern const struct inode_operations boesfs_main_iops;
extern const struct inode_operations boesfs_dir_iops;
extern const struct inode_operations boesfs_symlink_iops;
extern const struct super_operations boesfs_sops;
extern const struct dentry_operations boesfs_dops;
extern const struct address_space_operations boesfs_aops, boesfs_dummy_aops;
extern const struct vm_operations_struct boesfs_vm_ops;
extern const struct export_operations boesfs_export_ops;
extern const struct xattr_handler *boesfs_xattr_handlers[];

extern int boesfs_init_inode_cache(void);
extern void boesfs_destroy_inode_cache(void);
extern int boesfs_init_dentry_cache(void);
extern void boesfs_destroy_dentry_cache(void);
extern int new_dentry_private_data(struct dentry *dentry);
extern void free_dentry_private_data(struct dentry *dentry);
extern struct dentry *boesfs_lookup(struct inode *dir, struct dentry *dentry,
				    unsigned int flags);
extern struct inode *boesfs_iget(struct super_block *sb,
				 struct inode *lower_inode);
extern int boesfs_interpose(struct dentry *dentry, struct super_block *sb,
			    struct path *lower_path);
extern int vfs_path_lookup(struct dentry *, struct vfsmount *, const char *,
                           unsigned int, struct path *);

/* BoesFS extern */ 

// prog.c
extern int boesfs_set_prog_pos(struct super_block *, uint32_t);
extern int boesfs_run_prog(struct bpf_prog *, struct boesfs_args *);

// bpf_helper.c
extern int boesfs_register_prog_type(void);
extern int boesfs_unregister_prog_type(void);

// get_path.c
extern char * get_absolute_path_from_path_struct(struct path *, void *);
extern char * get_absolute_path_from_file_struct(struct file *, void *);
extern char * get_absolute_path_from_dentry_struct(struct dentry *, void *);

// kill.c
extern int boesfs_kill(pid_t);

// log.c
extern int boesfs_log(struct boesfs_args *, char *, int);

// supporter.c
extern void get_all_symbol_addr(void);

// parse.c
extern int boesfs_get_data_value(char *, int, void *);

// rlimit.c
extern int set_user_limits(void *, int);

/* file private data */
struct boesfs_file_info {
	struct file *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
};

/* boesfs inode data in memory */
struct boesfs_inode_info {
	struct inode *lower_inode;
	struct inode vfs_inode;
};

/* boesfs dentry data in memory */
struct boesfs_dentry_info {
	spinlock_t lock;	/* protects lower_path */
	struct path lower_path;
};

/* boesfs super-block data in memory */
struct boesfs_sb_info {
	struct super_block *lower_sb;

	// "fd=%d,pid=%d,kill=%d,log=%d,logpath=%s,elfpath=%s"
	struct bpf_prog *prog_pos; // prog的指针
	int pid; // 被沙盒的进程pid
	int kill; // 是否开启终止选项
	int deny; // 拒绝次数
	int log; // 是否开启日志选项
	struct dir_node* dir_tree_root;	//  dir tree 根节点
	char logpath[SINGLE_PATH_MAX]; 	// 日志文件路径
	char elfpath[SINGLE_PATH_MAX]; 	// 被沙盒的进程名
};

/*
 * inode to private data
 *
 * Since we use containers and the struct inode is _inside_ the
 * boesfs_inode_info structure, BOESFS_I will always (given a non-NULL
 * inode pointer), return a valid non-NULL pointer.
 */
static inline struct boesfs_inode_info *BOESFS_I(const struct inode *inode)
{
	return container_of(inode, struct boesfs_inode_info, vfs_inode);
}

/* dentry to private data */
#define BOESFS_D(dent) ((struct boesfs_dentry_info *)(dent)->d_fsdata)

/* superblock to private data */
#define BOESFS_SB(super) ((struct boesfs_sb_info *)(super)->s_fs_info)

/* file to private Data */
#define BOESFS_F(file) ((struct boesfs_file_info *)((file)->private_data))

/* file to lower file */
static inline struct file *boesfs_lower_file(const struct file *f)
{
	return BOESFS_F(f)->lower_file;
}

static inline void boesfs_set_lower_file(struct file *f, struct file *val)
{
	BOESFS_F(f)->lower_file = val;
}

/* inode to lower inode. */
static inline struct inode *boesfs_lower_inode(const struct inode *i)
{
	return BOESFS_I(i)->lower_inode;
}

static inline void boesfs_set_lower_inode(struct inode *i, struct inode *val)
{
	BOESFS_I(i)->lower_inode = val;
}

/* superblock to lower superblock */
static inline struct super_block *boesfs_lower_super(
	const struct super_block *sb)
{
	return BOESFS_SB(sb)->lower_sb;
}

static inline void boesfs_set_lower_super(struct super_block *sb,
					  struct super_block *val)
{
	BOESFS_SB(sb)->lower_sb = val;
}

/* path based (dentry/mnt) macros */
static inline void pathcpy(struct path *dst, const struct path *src)
{
	dst->dentry = src->dentry;
	dst->mnt = src->mnt;
}
/* Returns struct path.  Caller must path_put it. */
static inline void boesfs_get_lower_path(const struct dentry *dent,
					 struct path *lower_path)
{
	spin_lock(&BOESFS_D(dent)->lock);
	pathcpy(lower_path, &BOESFS_D(dent)->lower_path);
	path_get(lower_path);
	spin_unlock(&BOESFS_D(dent)->lock);
	return;
}
static inline void boesfs_put_lower_path(const struct dentry *dent,
					 struct path *lower_path)
{
	path_put(lower_path);
	return;
}
static inline void boesfs_set_lower_path(const struct dentry *dent,
					 struct path *lower_path)
{
	spin_lock(&BOESFS_D(dent)->lock);
	pathcpy(&BOESFS_D(dent)->lower_path, lower_path);
	spin_unlock(&BOESFS_D(dent)->lock);
	return;
}
static inline void boesfs_reset_lower_path(const struct dentry *dent)
{
	spin_lock(&BOESFS_D(dent)->lock);
	BOESFS_D(dent)->lower_path.dentry = NULL;
	BOESFS_D(dent)->lower_path.mnt = NULL;
	spin_unlock(&BOESFS_D(dent)->lock);
	return;
}
static inline void boesfs_put_reset_lower_path(const struct dentry *dent)
{
	struct path lower_path;
	spin_lock(&BOESFS_D(dent)->lock);
	pathcpy(&lower_path, &BOESFS_D(dent)->lower_path);
	BOESFS_D(dent)->lower_path.dentry = NULL;
	BOESFS_D(dent)->lower_path.mnt = NULL;
	spin_unlock(&BOESFS_D(dent)->lock);
	path_put(&lower_path);
	return;
}

/* locking helpers */
static inline struct dentry *lock_parent(struct dentry *dentry)
{
	struct dentry *dir = dget_parent(dentry);
	inode_lock_nested(d_inode(dir), I_MUTEX_PARENT);
	return dir;
}

static inline void unlock_dir(struct dentry *dir)
{
	inode_unlock(d_inode(dir));
	dput(dir);
}

/* Here Define The BoesFS Mount Data */
struct boesfs_mount_data {
	void *dev_name;
	char *agent_info;
};

/* log struct */
typedef struct log_info{
    char message[MESSAGE_MAX];
	char logfile[PATH_MAX];
}log_info;

#endif	/* not _BOESFS_H_ */