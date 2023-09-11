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

/**
 * TODO: Design your eBPF Map here
 * Tips: 
 * 1. There is an example here
 */

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
    DBG("boesfs read : arg1 %s\n",arg.path);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     * 3. there is an exsample:
     */

    // int *val = bpf_map_lookup_elem(&path_map,arg.path);
    // if(val!=NULL && *val == 0){
    //     DBG("boesfs read denied %s\n",arg.path);
    //     return -1;
    // }
    
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
    DBG("boesfs write : arg1 %s arg2 %d arg3 %d\n",arg.path,arg.len,arg.offset);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */
    
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

    DBG("boesfs lookup : arg1 %s arg2 %d\n",arg.path,arg.flags);
    
    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */
    
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

    DBG("boesfs open : arg1 %s\n",arg.path);
    
    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */
    
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
    DBG("boesfs mkdir : arg1 %s arg2 %d\n",arg.path,arg.mode);
    
    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */
    
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
    DBG("boesfs unlink : arg1 %s\n",arg.path);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */
    
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
    DBG("boesfs rmdir : arg1 %s\n",arg.path);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */

    return 0;
}

static int boesfs_mknod_handler(struct boesfs_args *ctx)
{
    struct mknod_boesfs_args arg = {
        .path = {0},
        .mode = 0,
        .dev = 0,
    };
    int ret = bpf_boesfs_read_args(BOESFS_MKNOD,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs mknod : arg1 %s arg2 %d arg3 %d\n",arg.path, arg.mode, arg.dev);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     * 3. there is an exsample:
     */

    // int *val = bpf_map_lookup_elem(&path_map,arg.path);
    // if(val!=NULL && *val == 0){
    //     DBG("boesfs read denied %s\n",arg.path);
    //     return -1;
    // }
    
    return 0;
}

static int boesfs_create_handler(struct boesfs_args *ctx)
{
    struct create_boesfs_args arg = {
        .path = {0},
        .mode = 0,
    };
    int ret = bpf_boesfs_read_args(BOESFS_CREATE,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs create : arg1 %s arg2 %d\n",arg.path, arg.mode);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     * 3. there is an exsample:
     */

    // int *val = bpf_map_lookup_elem(&path_map,arg.path);
    // if(val!=NULL && *val == 0){
    //     DBG("boesfs read denied %s\n",arg.path);
    //     return -1;
    // }
    
    return 0;
}

static int boesfs_link_handler(struct boesfs_args *ctx)
{
    struct link_boesfs_args arg = {
        .path = {0},
        .linkpath = {0},
    };
    int ret = bpf_boesfs_read_args(BOESFS_LINK,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs link : arg1 %s arg2 %s\n",arg.path, arg.linkpath);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     * 3. there is an exsample:
     */

    // int *val = bpf_map_lookup_elem(&path_map,arg.path);
    // if(val!=NULL && *val == 0){
    //     DBG("boesfs read denied %s\n",arg.path);
    //     return -1;
    // }
    
    return 0;
}


static int boesfs_symlink_handler(struct boesfs_args *ctx)
{
    struct symlink_boesfs_args arg = {
        .path = {0},
        .linkpath = {0},
    };
    int ret = bpf_boesfs_read_args(BOESFS_SYMLINK,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs symlink : arg1 %s arg2 %s\n",arg.path, arg.linkpath);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     * 3. there is an exsample:
     */

    // int *val = bpf_map_lookup_elem(&path_map,arg.path);
    // if(val!=NULL && *val == 0){
    //     DBG("boesfs read denied %s\n",arg.path);
    //     return -1;
    // }
    
    return 0;
}


static int boesfs_rename_handler(struct boesfs_args *ctx)
{
    struct rename_boesfs_args arg = {
        .path = {0},
        .newpath = {0},
    };
    int ret = bpf_boesfs_read_args(BOESFS_RENAME,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs rename : arg1 %s arg2 %d\n",arg.path, arg.newpath);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     * 3. there is an exsample:
     */

    // int *val = bpf_map_lookup_elem(&path_map,arg.path);
    // if(val!=NULL && *val == 0){
    //     DBG("boesfs read denied %s\n",arg.path);
    //     return -1;
    // }
    
    return 0;
}


static int boesfs_setattr_handler(struct boesfs_args *ctx)
{
    struct setattr_boesfs_args arg = {
        .path = {0},
        .mode = 0,
        .uid = 0,
        .gid = 0,
    };
    int ret = bpf_boesfs_read_args(BOESFS_SETATTR,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs setattr : arg1 %s arg2 %d arg3 %d arg4 %d\n",arg.path, arg.mode, arg.uid, arg.gid);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     * 3. there is an exsample:
     */

    // int *val = bpf_map_lookup_elem(&path_map,arg.path);
    // if(val!=NULL && *val == 0){
    //     DBG("boesfs read denied %s\n",arg.path);
    //     return -1;
    // }
    
    return 0;
}


static int boesfs_getattr_handler(struct boesfs_args *ctx)
{
    struct getattr_boesfs_args arg = {
        .path = {0},
    };
    int ret = bpf_boesfs_read_args(BOESFS_GETATTR,&arg,ctx);
    if(ret < 0)
        return -1;
    DBG("boesfs getattr : arg1 %s\n",arg.path);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */

    return 0;
}


static int boesfs_llseek_handler(struct boesfs_args *ctx)
{
    struct llseek_boesfs_args arg = {
        .path = {0},
        .offset = 0,
        .whence = 0,
    };
    int ret = bpf_boesfs_read_args(BOESFS_LLSEEK,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs llseek : arg1 %s arg2 %d arg3 %d\n",arg.path, arg.offset, arg.whence);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     * 3. there is an exsample:
     */

    // int *val = bpf_map_lookup_elem(&path_map,arg.path);
    // if(val!=NULL && *val == 0){
    //     DBG("boesfs read denied %s\n",arg.path);
    //     return -1;
    // }
    
    return 0;
}


static int boesfs_iterate_handler(struct boesfs_args *ctx)
{
    struct iterate_boesfs_args arg = {
        .path = {0},
    };
    int ret = bpf_boesfs_read_args(BOESFS_ITERATE,&arg,ctx);
    if(ret < 0)
        return -1;
    DBG("boesfs iterate : arg1 %s\n",arg.path);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */

    return 0;
}

static int boesfs_mmap_handler(struct boesfs_args *ctx)
{
    struct mmap_boesfs_args arg = {
        .path = {0},
        .startaddr = 0,
        .endaddr = 0,
    };
    int ret = bpf_boesfs_read_args(BOESFS_MMAP,&arg,ctx);
    if(ret < 0){
        DBG("failed to read args!\n");
        return -1;
    }
    DBG("boesfs mmap : arg1 %s arg2 0x%lx arg3 0x%lx\n",arg.path, arg.startaddr, arg.endaddr);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     * 3. there is an exsample:
     */

    // int *val = bpf_map_lookup_elem(&path_map,arg.path);
    // if(val!=NULL && *val == 0){
    //     DBG("boesfs read denied %s\n",arg.path);
    //     return -1;
    // }
    
    return 0;
}

static int boesfs_lookup2_handler(struct boesfs_args *ctx)
{
    struct lookup2_boesfs_args arg = {
        .path = {0},
        .flags = 0,
    };
    int ret = bpf_boesfs_read_args(BOESFS_LOOKUP2,&arg,ctx);
    if(ret < 0)
        return -1;
    DBG("boesfs lookup2 : arg1 %s arg2 %d\n",arg.path,arg.flags);
    
    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */
    
    return 0;
}


static int boesfs_statfs_handler(struct boesfs_args *ctx)
{
    struct statfs_boesfs_args arg = {
        .path = {0},
    };
    int ret = bpf_boesfs_read_args(BOESFS_STATFS,&arg,ctx);
    if(ret < 0)
        return -1;
    DBG("boesfs statfs : arg1 %s\n",arg.path);

    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */

    return 0;
}


static int boesfs_fsync_handler(struct boesfs_args *ctx)
{
    struct fsync_boesfs_args arg = {
        .path = {0},
        .len = 0,
    };
    int ret = bpf_boesfs_read_args(BOESFS_FSYNC,&arg,ctx);
    if(ret < 0)
        return -1;
    DBG("boesfs fsync : arg1 %s arg2 %d\n",arg.path,arg.len);
    
    /**
     * TODO: Finish your check rule here
     * Tips:
     * 1. you should add code here and in Boesfs-Agent
     * 2. you can use map to communicate between Boesfs-Agent and here
     */
    
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
        case BOESFS_MKNOD:
            return boesfs_mknod_handler(ctx);
        case BOESFS_CREATE:
            return boesfs_create_handler(ctx);
        case BOESFS_LINK:
            return boesfs_link_handler(ctx);
        case BOESFS_SYMLINK:
            return boesfs_symlink_handler(ctx);
        case BOESFS_RENAME:
            return boesfs_rename_handler(ctx);
        case BOESFS_SETATTR:
            return boesfs_setattr_handler(ctx);
        case BOESFS_GETATTR:
            return boesfs_getattr_handler(ctx);
        case BOESFS_LLSEEK:
            return boesfs_llseek_handler(ctx);
        case BOESFS_ITERATE:
            return boesfs_iterate_handler(ctx);
        case BOESFS_MMAP:
            return boesfs_mmap_handler(ctx);
        case BOESFS_LOOKUP2:
            return boesfs_lookup2_handler(ctx);
        case BOESFS_STATFS:
            return boesfs_statfs_handler(ctx);
        case BOESFS_FSYNC:
            return boesfs_fsync_handler(ctx);
        default:
            DBG("eBPF prog didn't finish this check\n");
            return -1;
    }
    return -1;
}

char _license[] SEC("license") = "GPL";
uint32_t _version SEC("version") = LINUX_VERSION_CODE;

