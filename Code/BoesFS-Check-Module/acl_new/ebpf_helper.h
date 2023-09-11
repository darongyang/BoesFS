
#ifndef _EBPF_HELPER_H_
#define _EBPF_HELPER_H_

#define PATH_MAX_SUPP 256

// Hooked VFS Functions Type
#define BOESFS_READ 	0
#define BOESFS_WRITE 	1
#define BOESFS_LOOKUP	2
#define BOESFS_OPEN		3
#define BOESFS_MKDIR	4
#define BOESFS_UNLINK	5
#define BOESFS_RMDIR	6


//  BoesFS采集参数表示
//  对于每种操作(vfs_function),采集的参数都有一个结构体对应
typedef struct read_boesfs_args{
	char path[PATH_MAX_SUPP];
	int len;
	int offset;
}read_boesfs_args_t;

typedef struct write_boesfs_args{
	char path[PATH_MAX_SUPP];
	int len;
	int offset;
}write_boesfs_args_t;

typedef struct lookup_boesfs_args{
	char path[PATH_MAX_SUPP];
	int flags;
}lookup_boesfs_args_t;

typedef struct open_boesfs_args{
	char path[PATH_MAX_SUPP];
}open_boesfs_args_t;

typedef struct mkdir_boesfs_args{
	char path[PATH_MAX_SUPP];
	int mode;
}mkdir_boesfs_args_t;

typedef struct unlink_boesfs_args{
	char path[PATH_MAX_SUPP];
}unlink_boesfs_args_t;

typedef struct rmdir_boesfs_args{
	char path[PATH_MAX_SUPP];
}rmdir_boesfs_args_t;

//  union统一起来. 便于is_valid_access检测内存访问越界.
typedef union args{
	lookup_boesfs_args_t 	lookup_args;
	unlink_boesfs_args_t 	unlink_args;
	read_boesfs_args_t   	read_args;
	open_boesfs_args_t   	open_args;
	write_boesfs_args_t  	write_args;
	mkdir_boesfs_args_t 	mkdir_args;
	rmdir_boesfs_args_t 	rmdir_args;
} union_args_t;

//  传递给bpf prog的上下文结构体
typedef struct boesfs_args{
    int op;
	union_args_t	args;
	char elfpath[PATH_MAX_SUPP];
}boesfs_args_t;

#endif	
