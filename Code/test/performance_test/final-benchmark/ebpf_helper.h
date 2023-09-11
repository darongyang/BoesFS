#ifndef _EBPF_HELPER_H_
#define _EBPF_HELPER_H_

#include "dir_tree.h"		//  for boesfs dir tree

#define PATH_MAX_SUPP 256

// Hooked VFS Functions Type
#define BOESFS_INIT		0
#define BOESFS_DESTROY 	1
#define BOESFS_READ 	2
#define BOESFS_WRITE 	3
#define BOESFS_LOOKUP	4
#define BOESFS_OPEN		5
#define BOESFS_MKDIR	6
#define BOESFS_UNLINK	7
#define BOESFS_RMDIR	8
#define BOESFS_MKNOD	9
#define BOESFS_CREATE	10
#define BOESFS_LINK		11
#define BOESFS_SYMLINK	12
#define BOESFS_RENAME	13
#define BOESFS_SETATTR	14
#define BOESFS_GETATTR	15
#define BOESFS_LLSEEK	16
#define BOESFS_ITERATE	17
#define BOESFS_MMAP		18
#define BOESFS_LOOKUP2	19
#define BOESFS_STATFS	20
#define BOESFS_FSYNC	21

//   BoesFS采集参数表示
//  对于每种操作(vfs_function),采集的参数都有一个结构体对应
typedef struct read_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int		len;
	int		offset;
}read_boesfs_args_t;

typedef struct write_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int		len;
	int		offset;
}write_boesfs_args_t;

typedef struct lookup_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int		flags;
}lookup_boesfs_args_t;

typedef struct open_boesfs_args{
	char	path[PATH_MAX_SUPP];
}open_boesfs_args_t;

typedef struct mkdir_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int		mode;
}mkdir_boesfs_args_t;

typedef struct unlink_boesfs_args{
	char	path[PATH_MAX_SUPP];
}unlink_boesfs_args_t;

typedef struct rmdir_boesfs_args{
	char	path[PATH_MAX_SUPP];
}rmdir_boesfs_args_t;

typedef struct mknod_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int		mode;
	int		dev;
}mknod_boesfs_args_t;

typedef struct create_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int		mode;
}create_boesfs_args_t;

typedef struct link_boesfs_args{
	char	path[PATH_MAX_SUPP];
	char	linkpath[PATH_MAX_SUPP];
}link_boesfs_args_t;

typedef struct symlink_boesfs_args{
	char	path[PATH_MAX_SUPP];
	char	linkpath[PATH_MAX_SUPP];
}symlink_boesfs_args_t;

typedef struct rename_boesfs_args{
	char	path[PATH_MAX_SUPP];
	char	newpath[PATH_MAX_SUPP];
}rename_boesfs_args_t;

typedef struct setattr_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int		mode;
	int 	uid;
	int 	gid;
}setattr_boesfs_args_t;

typedef struct getattr_boesfs_args{
	char	path[PATH_MAX_SUPP];
}getattr_boesfs_args_t;

typedef struct llseek_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int		offset;
	int 	whence;
}llseek_boesfs_args_t;

typedef struct iterate_boesfs_args{
	char	path[PATH_MAX_SUPP];
}iterate_boesfs_args_t;

typedef struct mmap_boesfs_args{
	char			path[PATH_MAX_SUPP];
	unsigned long	startaddr;
	unsigned long	endaddr;
}mmap_boesfs_args_t;

typedef struct lookup2_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int		flags;
}lookup2_boesfs_args_t;

typedef struct statfs_boesfs_args{
	char	path[PATH_MAX_SUPP];
}statfs_boesfs_args_t;

typedef struct fsync_boesfs_args{
	char	path[PATH_MAX_SUPP];
	int 	len;
}fsync_boesfs_args_t;

//  union统一起来. 便于is_valid_access检测内存访问越界.
typedef union args {
	lookup_boesfs_args_t 	lookup_args;
	unlink_boesfs_args_t 	unlink_args;
	read_boesfs_args_t   	read_args;
	open_boesfs_args_t   	open_args;
	write_boesfs_args_t  	write_args;
	mkdir_boesfs_args_t 	mkdir_args;
	rmdir_boesfs_args_t 	rmdir_args;
	mknod_boesfs_args_t 	mknod_args;
	create_boesfs_args_t 	create_args;
	link_boesfs_args_t 		link_args;
	symlink_boesfs_args_t 	symlink_args;
	rename_boesfs_args_t 	rename_args;
	setattr_boesfs_args_t 	setattr_args;
	getattr_boesfs_args_t 	getattr_args;
	llseek_boesfs_args_t 	llseek_args;
	iterate_boesfs_args_t 	iterate_args;
	mmap_boesfs_args_t 		mmap_args;
	lookup2_boesfs_args_t 	lookup2_args;
	statfs_boesfs_args_t 	statfs_args;
	fsync_boesfs_args_t 	fsync_args;
} union_args_t;

//  传递给bpf prog的上下文结构体
typedef struct boesfs_args {
    int op;									//  op
	union_args_t	args;					//  op args
	struct dir_node *dir_tree_root;			//  dir_tree_root
	char elfpath[PATH_MAX_SUPP];			//  useless
} boesfs_args_t;


#define LINE_ARG_MAX 10
typedef struct line{
    __u32 nums;         //  参数个数
    __u32 args[LINE_ARG_MAX]; 
}line_t;


#endif	
