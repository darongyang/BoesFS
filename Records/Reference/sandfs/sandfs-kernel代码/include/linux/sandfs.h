#ifndef _LINUX_SANDFS_H
#define _LINUX_SANDFS_H

#define MAX_NUM_ARGS 4
//  操作类型
typedef enum {
	SANDFS_LOOKUP,
	SANDFS_OPEN,
	SANDFS_CLOSE,
	SANDFS_READ,
	SANDFS_WRITE,
	// SANDFS_UNLINK		//  add unlink
} sandfs_op_t;

//  这些代表我希望读取到sandfs_args中的哪个参数的信息?
typedef enum {
	OPCODE = 0,				//  希望读取到sandfs_args中的操作类型 
	NUM_ARGS,				//  希望读取到sandfs_args中的num args
	PARAM_0_SIZE,			//  希望读取到第0个参数的大小
	PARAM_0_VALUE,			//  希望读取到第1个参数的值
	PARAM_1_SIZE,			//  ...
	PARAM_1_VALUE,
	PARAM_2_SIZE,
	PARAM_2_VALUE,
	PARAM_3_SIZE,
	PARAM_3_VALUE,
} sandfs_arg_t;

//  一个操作参数(大小和值)
struct sandfs_arg {
	uint32_t size;
	void *value;
};
//  一堆操作参数
struct sandfs_args {
	sandfs_arg_t op;						//  操作类型. LOOKUP / OPEN / CLOSE / READ / WRITE
	//  我只能说这tm定义错了吧... 根本没有把他当作sandfs_arg_t用的时候...
	//  修改 sandfs_arg_t -> sandfs_op_t
	uint32_t num_args;						//  参数个数
	struct sandfs_arg args[MAX_NUM_ARGS];	//  操作数量上限 ./ 最大操作范围
};

#endif /* _LINUX_SANDFS_H */
