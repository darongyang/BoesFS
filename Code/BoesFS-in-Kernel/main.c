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
#include "dir_tree.h"
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/bpf.h>

#define USER_MAX 10

/* 内核模块参数 */
static char *user[USER_MAX] = {"", "", "", "", "", "", "", "", "", ""};
static int limit[USER_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int user_num = 0;
static int limit_num = 0;
 
/* 传参声明 */
module_param_array(user , charp , &user_num , S_IRUGO);
module_param_array(limit , int , &limit_num , S_IRUGO);


/*
 * There is no need to lock the boesfs_super_info's rwsem as there is no
 * way anyone can have a reference to the superblock at this point in time.
 */
static int boesfs_read_super(struct super_block *sb, void *raw_data, int silent)
{
	int err = 0;
	struct super_block *lower_sb;
	struct path lower_path;
	struct boesfs_mount_data * mount_data = (struct boesfs_mount_data *)raw_data;
	char *dev_name = mount_data->dev_name;
	char * agent_info = mount_data->agent_info;
	struct inode *inode;

	// printk("raw_data: %p\n", raw_data);

	printk("mount_data->dev_name: %s\n", mount_data->dev_name);
	
	printk("mount_data->agent_info: %s\n", mount_data->agent_info);

	printk("agent_info: %s\n", agent_info);

	/* Agent data Relative */
	int prog_fd; // prog fd
	// int kill; // kill or not
	// int log; // log or not
	// int pid; // sandbox process pid
	// char logpath[SINGLE_PATH_MAX]; // log file path
	// char elfpath[SINGLE_PATH_MAX]; // elf path

	if (!dev_name) {
		printk(KERN_ERR
		       "boesfs: read_super: missing dev_name argument\n");
		err = -EINVAL;
		goto out;
	}

	/* BoesFS fill Super Block to Save Data from Agent */
	boesfs_get_data_value(agent_info, 0, &prog_fd);// 从prog_info得到prog_fd

	if (prog_fd == -1) {
		printk("Error: prog_fd=%d\n", prog_fd);
		goto out;
	}

	printk("INFO: prog_fd=%d\n", prog_fd);

	/* parse lower path */
	err = kern_path(dev_name, LOOKUP_FOLLOW | LOOKUP_DIRECTORY,
			&lower_path);
	if (err) {
		printk(KERN_ERR	"boesfs: error accessing "
		       "lower directory '%s'\n", dev_name);
		goto out;
	}

	/* allocate superblock private data */
	sb->s_fs_info = kzalloc(sizeof(struct boesfs_sb_info), GFP_KERNEL);
	if (!BOESFS_SB(sb)) {
		printk(KERN_CRIT "boesfs: read_super: out of memory\n");
		err = -ENOMEM;
		goto out_free;
	}

	if (boesfs_set_prog_pos(sb, prog_fd))
		printk("Error: Set prog pos failed\n");
	boesfs_get_data_value(agent_info, 1, &(BOESFS_SB(sb)->pid));
	boesfs_get_data_value(agent_info, 2, &(BOESFS_SB(sb)->kill));
	boesfs_get_data_value(agent_info, 3, &(BOESFS_SB(sb)->log));
	boesfs_get_data_value(agent_info, 4, BOESFS_SB(sb)->logpath);
	boesfs_get_data_value(agent_info, 5, BOESFS_SB(sb)->elfpath);

	/* set the lower superblock field of upper superblock */
	lower_sb = lower_path.dentry->d_sb;
	atomic_inc(&lower_sb->s_active);
	boesfs_set_lower_super(sb, lower_sb);

	/* inherit maxbytes from lower file system */
	sb->s_maxbytes = lower_sb->s_maxbytes;

	/*
	 * Our c/m/atime granularity is 1 ns because we may stack on file
	 * systems whose granularity is as good.
	 */
	sb->s_time_gran = 1;

	sb->s_op = &boesfs_sops;
	sb->s_xattr = boesfs_xattr_handlers;

	sb->s_export_op = &boesfs_export_ops; /* adding NFS support */

	/* get a new inode and allocate our root dentry */
	inode = boesfs_iget(sb, d_inode(lower_path.dentry));
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto out_sput;
	}
	sb->s_root = d_make_root(inode);
	if (!sb->s_root) {
		err = -ENOMEM;
		goto out_iput;
	}
	d_set_d_op(sb->s_root, &boesfs_dops);

	/* link the upper and lower dentries */
	sb->s_root->d_fsdata = NULL;
	err = new_dentry_private_data(sb->s_root);
	if (err)
		goto out_freeroot;

	/* if get here: cannot have error */

	/* set the lower dentries for s_root */
	boesfs_set_lower_path(sb->s_root, &lower_path);

	/*
	 * No need to call interpose because we already have a positive
	 * dentry, which was instantiated by d_make_root.  Just need to
	 * d_rehash it.
	 */
	d_rehash(sb->s_root);
	if (!silent)
		printk(KERN_INFO
		       "boesfs: mounted on top of %s type %s\n",
		       dev_name, lower_sb->s_type->name);
	
	/* init env for ebpf prog , such as dir_tree_root ... */
	struct boesfs_args init_ebpf_args;
	init_ebpf_args.op = BOESFS_INIT;
	init_ebpf_args.dir_tree_root = NULL;
	int ret = boesfs_run_prog(BOESFS_SB(sb)->prog_pos,&init_ebpf_args);
	// printk("mount : boesfs_run_prog BOESFS_INIT!\n");
	/* init failed */
	// printk("init_ebpf_args ret = %d , root = %p\n",ret,init_ebpf_args.dir_tree_root);
	if(ret < 0 || init_ebpf_args.dir_tree_root == NULL){
		err = -EINVAL;
		goto out_freeroot;
	} 
	/* record env on superblock for later*/
	BOESFS_SB(sb)->dir_tree_root = init_ebpf_args.dir_tree_root;
	// printk("mount : dir_tree_root init %p!\n",BOESFS_SB(sb)->dir_tree_root);


	goto out; /* all is well */

	/* no longer needed: free_dentry_private_data(sb->s_root); */
out_freeroot:
	dput(sb->s_root);
out_iput:
	iput(inode);
out_sput:
	/* drop refs we took earlier */
	atomic_dec(&lower_sb->s_active);
	kfree(BOESFS_SB(sb));
	sb->s_fs_info = NULL;
out_free:
	path_put(&lower_path);

out:
	return err;
}

struct dentry *boesfs_mount(struct file_system_type *fs_type, int flags,
			    const char *dev_name, void *raw_data)
{
	struct boesfs_mount_data mount_data;
	mount_data.dev_name = (void *)dev_name;
	mount_data.agent_info = (char *)raw_data;
	return mount_nodev(fs_type, flags, (void *)&mount_data,
			   boesfs_read_super);
}

void boesfs_shutdown_super(struct super_block *sb)
{
	//  destroy something when the prog exit , such as dir tree ...
	struct boesfs_args exit_ebpf_args;
	exit_ebpf_args.op = BOESFS_DESTROY;
	exit_ebpf_args.dir_tree_root = BOESFS_SB(sb)->dir_tree_root;
	boesfs_run_prog(BOESFS_SB(sb)->prog_pos,&exit_ebpf_args);

	bpf_prog_put(BOESFS_SB(sb)->prog_pos); // 释放掉超级块对prog的引用
	/* 后续看情况还要不要新增别的内容 */
	return generic_shutdown_super(sb);
}

static struct file_system_type boesfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= BOESFS_NAME,
	.mount		= boesfs_mount,
	.kill_sb	= boesfs_shutdown_super,
	.fs_flags	= FS_USERNS_MOUNT,
};
MODULE_ALIAS_FS(BOESFS_NAME);

static int __init init_boesfs_fs(void)
{
	int err;
	int i;

	pr_info("Registering boesfs " BOESFS_VERSION "\n");

	get_all_symbol_addr();

	err = boesfs_init_inode_cache();
	if (err)
		goto out;
	err = boesfs_init_dentry_cache();
	if (err)
		goto out;
	err = register_filesystem(&boesfs_fs_type);

	err = boesfs_register_prog_type(); // 注册BoesFS prog 类型

	dir_node_cache = kmem_cache_create("dir_node_cache", sizeof(struct dir_node), 0, SLAB_HWCACHE_ALIGN, NULL);
	if(dir_node_cache == NULL) {
		err = -ENOMEM;
		goto out;
	}

	/* 修改rlimit配置文件 */
    if(user_num != limit_num)
        printk("ERROR: user num != limit num");
    else if(user_num == limit_num && user_num != 0){
        for (i=0; i<user_num; i++){
            set_user_limits(user[i], limit[i]);
            printk("user=%s, limit=%d\n", user[i], limit[i]);
        }
            
    }

out:
	if (err) {
		boesfs_destroy_inode_cache();
		boesfs_destroy_dentry_cache();
	}
	return err;
}

static void __exit exit_boesfs_fs(void)
{
	printk("dir node cache slab destroy!!\n");
	kmem_cache_destroy(dir_node_cache);

	boesfs_unregister_prog_type(); // 取消注册
	boesfs_destroy_inode_cache();
	boesfs_destroy_dentry_cache();
	unregister_filesystem(&boesfs_fs_type);
	pr_info("Completed boesfs module unload\n");
}

MODULE_AUTHOR("Erez Zadok, Filesystems and Storage Lab, Stony Brook University"
	      " (https://www.fsl.cs.sunysb.edu/)");
MODULE_DESCRIPTION("Boesfs " BOESFS_VERSION
		   " (https://boesfs.filesystems.org/)");
MODULE_LICENSE("GPL");

module_init(init_boesfs_fs);
module_exit(exit_boesfs_fs);
