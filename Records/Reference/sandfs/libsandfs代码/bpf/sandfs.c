/*
 * This module has the kernel code for SandFS
 */
//  这个应该就是 bpf prog
#define KBUILD_MODNAME "sandfs"
#include <uapi/linux/bpf.h>
#include "bpf_helpers.h"

#include <linux/version.h>
#include <linux/fs.h>

#include <linux/sandfs.h>

//  #include <uapi/asm-generic/errno-base.h>

/********************************************************************
	HELPERS
*********************************************************************/
#define PRINTK(fmt, ...)                                               \
                ({                                                      \
                        char ____fmt[] = fmt;                           \
                        bpf_trace_printk(____fmt, sizeof(____fmt),      \
                                     ##__VA_ARGS__);                    \
                })

#define DEBUG

#ifdef DEBUG
#define DBG PRINTK
#else
#define DBG(fmt, ...)
#endif

#define ERR PRINTK

#define PATH_MAX_SUPP 256
//  定义容量32bpf_map，key大小为PATH_MAX_SUPP，value为int的hash map
struct bpf_map_def SEC("maps") datamap = {
		.type			= BPF_MAP_TYPE_HASH,	// simple hash list
		.key_size 		= PATH_MAX_SUPP,
		.value_size 	= sizeof(int),
        .max_entries 	= 32,
};


static int get_path(void *ctx, char *path)
{
	int ret = -1;
	struct sandfs_args *args = (struct sandfs_args *)ctx;
	//  获取操作类型. SANDFS_LOOKUP , SANDFS_READ ..
	uint32_t opcode = args->op;

	uint32_t len;
	//  从sandfs_args中读取path的长度 (path是第 0 个操作数)，存入len
	ret = bpf_sandfs_read_args(ctx, PARAM_0_SIZE, &len, sizeof(u32));
	if (ret) {	
		ERR("op %d: failed to read path len: %d!\n", opcode, ret);
	} else {
		if (PATH_MAX_SUPP < len) {
			ERR("op %d: not enough memory!\n", opcode);
			ret = -E2BIG;
		} else {
			//  从sandfs_args中读取path
			ret = bpf_sandfs_read_args(ctx, PARAM_0_VALUE, path, PATH_MAX_SUPP);
			if (ret) {
				ERR("LOOKUP: failed to read path: %d!\n", ret);
			} else {
				ret = 0;
			}
		}
	}

	return ret;
}

// //  add unlink
// static int sandfs_unlink(void *ctx)
// {
//   DBG("ebpf prog sandfs_unlink!\n");
// 	char path[PATH_MAX_SUPP] = {0};
// 	//  读取path
// 	int ret = get_path(ctx, path);	
// 	if (!ret) {
// 		DBG("SANDFS_UNLINK(%s)\n", path);
// 		//  禁止删除path
// 		int *val = bpf_map_lookup_elem(&datamap, path);
// 		if(val && *val == -EPERM){
// 			DBG("SANDFS_UNLINK(%s): denied\n", path);
// 			ret = *val;
// 		}
// 	}
// 	return ret;
// }

static int sandfs_lookup(void *ctx)
{
	char path[PATH_MAX_SUPP] = {0};
	//  读取path
	int ret = get_path(ctx, path);	
	if (!ret) {
		DBG("SANDFS_LOOKUP(%s)\n", path);
		//  禁止访问path
		int *val = bpf_map_lookup_elem(&datamap, path);
		if (val) {
		// if(val && *val == -EACCES){
			DBG("SANDFS_LOOKUP(%s): denied\n", path);
			ret = *val;		//  unaccess
		}
	}
	return ret;
}

static int sandfs_open(void *ctx)
{
	return 0;
}

static int sandfs_read(void *ctx)
{
	char path[PATH_MAX_SUPP] = {0};
	int ret = get_path(ctx, path);
	if (!ret) {
		DBG("SANDFS_READ(%s)\n", path);
	}
	return ret;
}

static int sandfs_write(void *ctx)
{
	char path[PATH_MAX_SUPP] = {0};
	int ret = get_path(ctx, path);
	if (!ret) {
		DBG("SANDFS_READ(%s)\n", path);
	}
	return ret;
}

static int sandfs_close(void *ctx)
{
	return 0;
}

/*
 * SandFS main handler function
 */
//  插入的callback. 根据kernel传入的sandfs_op_t执行相应callback
int SEC("sandfs") sandfs_main_handler(void *ctx)
{
	int ret = 0;
	struct sandfs_args *args = (struct sandfs_args *)ctx;
	uint32_t opcode = args->op;
	DBG("opcode %d\n", opcode);
	//  根据操作类型LOOKUP / ... /  ，执行相应的检测函数
	if (opcode == SANDFS_LOOKUP)
		ret = sandfs_lookup(ctx);
	else if (opcode == SANDFS_OPEN)
		ret = sandfs_open(ctx);
	else if (opcode == SANDFS_READ)
		ret = sandfs_read(ctx);
	else if (opcode == SANDFS_WRITE)
		ret = sandfs_write(ctx);
	else if (opcode == SANDFS_CLOSE)
		ret = sandfs_close(ctx);
	return ret;
}

char _license[] SEC("license") = "GPL";
uint32_t _version SEC("version") = LINUX_VERSION_CODE;
// Notice that SEC("license") is essential for this program. Because some helper functions are only accessible to programs that are compatible with the GNU Privacy License (GPL). In order to use such helpers, the eBPF program must be loaded with the correct license string.