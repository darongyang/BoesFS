//  Demo For Boesfs

#define KBUILD_MODNAME "boesfs"

#include <uapi/linux/bpf.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <uapi/asm-generic/errno-base.h>
#include "bpf_helpers.h"        //  可以更改为你的bpf_helpers.h路径
#include "ebpf_helper.h"        //  可以更改为你的ebpf_helper.h路径

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

struct bpf_map_def SEC("maps") path_map = {
		.type			= BPF_MAP_TYPE_HASH,	
		.key_size 		= PATH_MAX_SUPP,
		.value_size 	= sizeof(int),
        .max_entries 	= 32,
};


static int boesfs_read_handler(struct boesfs_args *ctx)
{
    struct read_boesfs_args arg = {
        .path = {0},
        .len = 0,
        .offset = 0,
    };
    int ret = bpf_boesfs_read_args(BOESFS_READ,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs read : path %s\n",arg.path);
    int *val = bpf_map_lookup_elem(&path_map,arg.path);
    if(val!=NULL && *val == 0){
        DBG("boesfs read denied %s\n",arg.path);
        return -1;
    }
    
    return 0;
}

static int boesfs_write_handler(struct boesfs_args *ctx)
{
    struct write_boesfs_args arg = {
        .path = {0},
        .len = 0,
        .offset = 0,
    };

    int ret = bpf_boesfs_read_args(BOESFS_WRITE,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs write : path %s len %d offset %d\n",arg.path,arg.len,arg.offset);
    
    int *val = bpf_map_lookup_elem(&path_map,arg.path);
    if(val && *val == 1){
        DBG("boesfs write denied %s\n",arg.path);
        return -1;
    }

    return 0;
}

static int boesfs_lookup_handler(struct boesfs_args *ctx)
{
    struct lookup_boesfs_args arg = {
        .path = {0},
        .flags = 0,
    };
	
    int ret = bpf_boesfs_read_args(BOESFS_LOOKUP,&arg,ctx);

    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }

    DBG("boesfs lookup : path %s flags %d\n",arg.path,arg.flags);
    
    int *val = bpf_map_lookup_elem(&path_map,arg.path);
    if(val && *val == 2){
        DBG("boesfs lookup denied %s\n",arg.path);
        return -1;
    }
    
    return 0;
}

static int boesfs_open_handler(struct boesfs_args *ctx)
{
    struct open_boesfs_args arg = {
        .path = {0},
    };

    int ret = bpf_boesfs_read_args(BOESFS_OPEN,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }

    DBG("boesfs open : path %s\n",arg.path);
    
    int *val = bpf_map_lookup_elem(&path_map,arg.path);
    if(val!=NULL && *val == 3){
        DBG("boesfs open denied %s\n",arg.path);
        return -1;
    }
    
    return 0;
}


static int boesfs_mkdir_handler(struct boesfs_args *ctx)
{
    struct mkdir_boesfs_args arg = {
        .path = {0},
        .mode = 0,
    };
    int ret = bpf_boesfs_read_args(BOESFS_MKDIR,&arg,ctx);
    if(ret < 0)
        return -1;
    DBG("boesfs mkdir : path %s mode %d\n",arg.path,arg.mode);
    
    int *val = bpf_map_lookup_elem(&path_map,arg.path);
    if(val && *val == 4){
        DBG("boesfs mkdir denied %s\n",arg.path);
        return -1;
    }
    
    return 0;
}


static int boesfs_unlink_handler(struct boesfs_args *ctx)
{
    struct unlink_boesfs_args arg = {
        .path = {0},
    };

    int ret = bpf_boesfs_read_args(BOESFS_UNLINK,&arg,ctx);
    if(ret < 0)
        return -1;
    DBG("boesfs unlink : path %s\n",arg.path);

    int *val = bpf_map_lookup_elem(&path_map,arg.path);
    if(val && *val == 5){
        DBG("boesfs unlink denied %s\n",arg.path);
        return -1;
    }
    
    return 0;
}

static int boesfs_rmdir_handler(struct boesfs_args *ctx)
{
    struct rmdir_boesfs_args arg = {
        .path = {0},
    };
    int ret = bpf_boesfs_read_args(BOESFS_RMDIR,&arg,ctx);
    if(ret < 0)
        return -1;
    DBG("boesfs rmdir : path %s\n",arg.path);

    int *val = bpf_map_lookup_elem(&path_map,arg.path);
    if(val && *val == 6){
        DBG("boesfs rmdir denied %s\n",arg.path);
        return -1;
    }

    return 0;
}


int SEC("boesfs") boesfs_main_handler(struct boesfs_args *ctx)
{
    PRINTK("Boesfs Main Handler!\n");
    int op = ctx->op;
    switch (op)
    {
        case BOESFS_READ: 
           return boesfs_read_handler(ctx);
        case BOESFS_WRITE:
            return boesfs_write_handler(ctx);
        case BOESFS_LOOKUP:
            return boesfs_lookup_handler(ctx);
        case BOESFS_OPEN:
            return boesfs_open_handler(ctx);
        case BOESFS_MKDIR:
            return boesfs_mkdir_handler(ctx);
        case BOESFS_UNLINK:
            return boesfs_unlink_handler(ctx);
        case BOESFS_RMDIR:
            return boesfs_rmdir_handler(ctx);
        default:
            DBG("eBPF prog didn't finish this check\n");
            return -1;
    }
    return -1;
}

char _license[] SEC("license") = "GPL";
uint32_t _version SEC("version") = LINUX_VERSION_CODE;

