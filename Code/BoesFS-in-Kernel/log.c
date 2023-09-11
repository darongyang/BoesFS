#include "boesfs.h"
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

static struct task_struct *log_write_kthread = NULL;

/**
 * @brief 日志记录的线程函数
 * 
 * @param log 日志结构信息
 * @return int 0 成功 1失败
 */
int log_func(void * data){
    struct log_info *log = (struct log_info*)data;
    char message[MESSAGE_MAX];
	char logfile[PATH_MAX];
    int len;
    struct file *file = NULL;
    mm_segment_t fs_bak;
    loff_t pos;

    memcpy(message, log->message, strlen(log->message) + 1);
    memcpy(logfile, log->logfile, strlen(log->logfile) + 1);

    
    len = strlen(message);
    file = filp_open(logfile, O_RDWR|O_APPEND|O_CREAT,0644);

    if(IS_ERR(file)){
        printk("ERROR: Open log file fail!");
        return ERR;
    }

    fs_bak = get_fs(); 
    set_fs(KERNEL_DS);

    pos = file->f_pos;
    vfs_write(file, message, len, &pos);

    filp_close(file,NULL);
    set_fs(fs_bak);
    
    if(data != NULL)
        kfree(data);

    do_exit(0);
    return 0;
}

/**
 * @brief 输入一个boesfs_args结构体，格式化得到字符串信息
 * 
 * @param args 待格式化的参数
 * @param message 返回结果
 * @param err 访问是否允许
 * @return int 0: 成功 1: 失败
 */
int args_to_string(struct boesfs_args * args, char * message, int err){
    int pos = 0; // 字符串拼接指针

    int type = args->op;
    int len = 0;
    char log_path[LOG_PATH_MAX];
    char log_other[MESSAGE_MAX - LOG_PATH_MAX];

    if(message == NULL){
        printk("ERROR: message is NULL\n");
        return ERR;
    }

    // deny or allow
    if(err >= 0){
        memcpy(&message[pos], "[allow] ", strlen("[allow] ")+1);
        pos += strlen("[allow] ");
    }
    else{
        memcpy(&message[pos], "[deny] ", strlen("[deny] ")+1);
        pos += strlen("[deny] ");
    }
        
    char * elfpath = args->elfpath;

    // elf name
    memcpy(&message[pos], elfpath, strlen(elfpath) + 1);
    pos += strlen(elfpath);
    message[pos] = ',';
    pos += 1;

    // type and args
    switch (type)
    {
    case BOESFS_READ:
        memcpy(&message[pos], "read,", strlen("read,")+1);
        pos += strlen("read,");
        len = strlen(args->args.read_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.read_args.path);
        memcpy(log_path, args->args.read_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d,%d",args->args.read_args.len, args->args.read_args.offset);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_WRITE:
        memcpy(&message[pos], "write,", strlen("write,")+1);
        pos += strlen("write,");
        len = strlen(args->args.write_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.write_args.path);
        memcpy(log_path, args->args.write_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d,%d",args->args.write_args.len, args->args.write_args.offset);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_LOOKUP:
        memcpy(&message[pos], "lookup,", strlen("lookup,")+1);
        pos += strlen("lookup,");
        len = strlen(args->args.lookup_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.lookup_args.path);
        memcpy(log_path, args->args.lookup_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d",args->args.lookup_args.flags);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_OPEN:
        memcpy(&message[pos], "open,", strlen("open,")+1);
        pos += strlen("open,");
        len = strlen(args->args.open_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.open_args.path);
        memcpy(log_path, args->args.open_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s",log_path);
        break;
    case BOESFS_MKDIR:
        memcpy(&message[pos], "mkdir,", strlen("mkdir,")+1);
        pos += strlen("mkdir,");
        len = strlen(args->args.mkdir_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.mkdir_args.path);
        memcpy(log_path, args->args.mkdir_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d",args->args.mkdir_args.mode);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_UNLINK:
        memcpy(&message[pos], "unlink,", strlen("unlink,")+1);
        pos += strlen("unlink,");
        len = strlen(args->args.unlink_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.unlink_args.path);
        memcpy(log_path, args->args.unlink_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s",log_path);
        break;
    case BOESFS_RMDIR:
        memcpy(&message[pos], "rmdir,", strlen("rmdir,")+1);
        pos += strlen("rmdir,");
        len = strlen(args->args.rmdir_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.rmdir_args.path);
        memcpy(log_path, args->args.rmdir_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s",log_path);
        break;
    case BOESFS_MKNOD:
        memcpy(&message[pos], "mknod,", strlen("mknod,")+1);
        pos += strlen("mknod,");
        len = strlen(args->args.mknod_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.mknod_args.path);
        memcpy(log_path, args->args.mknod_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d,%d",args->args.mknod_args.mode, args->args.mknod_args.dev);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_CREATE:
        memcpy(&message[pos], "create,", strlen("create,")+1);
        pos += strlen("create,");
        len = strlen(args->args.create_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.create_args.path);
        memcpy(log_path, args->args.create_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d",args->args.create_args.mode);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_LINK:
        memcpy(&message[pos], "link,", strlen("link,")+1);
        pos += strlen("link,");
        len = strlen(args->args.link_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.link_args.path);
        memcpy(log_path, args->args.link_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        len = strlen(args->args.link_args.linkpath) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.link_args.linkpath);
        memcpy(log_other, args->args.link_args.linkpath, len+1);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_SYMLINK:
        memcpy(&message[pos], "symlink,", strlen("symlink,")+1);
        pos += strlen("symlink,");
        len = strlen(args->args.symlink_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.symlink_args.path);
        memcpy(log_path, args->args.symlink_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        len = strlen(args->args.symlink_args.linkpath) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.symlink_args.linkpath);
        memcpy(log_other, args->args.symlink_args.linkpath, len+1);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_RENAME:
        memcpy(&message[pos], "rename,", strlen("rename,")+1);
        pos += strlen("rename,");
        len = strlen(args->args.rename_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.rename_args.path);
        memcpy(log_path, args->args.rename_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        len = strlen(args->args.rename_args.newpath) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.rename_args.newpath);
        memcpy(log_other, args->args.rename_args.newpath, len+1);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_SETATTR:
        memcpy(&message[pos], "setattr,", strlen("setattr,")+1);
        pos += strlen("setattr,");
        len = strlen(args->args.setattr_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.setattr_args.path);
        memcpy(log_path, args->args.setattr_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d,%d,%d",args->args.setattr_args.mode, args->args.setattr_args.uid, args->args.setattr_args.gid);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_GETATTR:
        memcpy(&message[pos], "getattr,", strlen("getattr,")+1);
        pos += strlen("getattr,");
        len = strlen(args->args.getattr_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.getattr_args.path);
        memcpy(log_path, args->args.getattr_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s",log_path);
        break;
    case BOESFS_LLSEEK:
        memcpy(&message[pos], "llseek,", strlen("llseek,")+1);
        pos += strlen("llseek,");
        len = strlen(args->args.llseek_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.llseek_args.path);
        memcpy(log_path, args->args.llseek_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d,%d",args->args.llseek_args.offset, args->args.llseek_args.whence);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_MMAP:
        memcpy(&message[pos], "mmap,", strlen("mmap,")+1);
        pos += strlen("mmap,");
        len = strlen(args->args.mmap_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.mmap_args.path);
        memcpy(log_path, args->args.mmap_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"0x%lx,0x%lx",args->args.mmap_args.startaddr, args->args.mmap_args.endaddr);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_ITERATE:
        memcpy(&message[pos], "iterate,", strlen("iterate,")+1);
        pos += strlen("iterate,");
        len = strlen(args->args.iterate_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.iterate_args.path);
        memcpy(log_path, args->args.iterate_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s",log_path);
        break;
    case BOESFS_LOOKUP2:
        memcpy(&message[pos], "lookup2,", strlen("lookup2,")+1);
        pos += strlen("lookup2,");
        len = strlen(args->args.lookup2_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.lookup2_args.path);
        memcpy(log_path, args->args.lookup2_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d",args->args.lookup2_args.flags);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    case BOESFS_STATFS:
        memcpy(&message[pos], "statfs,", strlen("statfs,")+1);
        pos += strlen("statfs,");
        len = strlen(args->args.statfs_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.statfs_args.path);
        memcpy(log_path, args->args.statfs_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s",log_path);
        break;
    case BOESFS_FSYNC:
        memcpy(&message[pos], "fsync,", strlen("fsync,")+1);
        pos += strlen("fsync,");
        len = strlen(args->args.fsync_args.path) > LOG_PATH_MAX-1 ? LOG_PATH_MAX-1 : strlen(args->args.fsync_args.path);
        memcpy(log_path, args->args.fsync_args.path, len+1);
        log_path[len] = '\0';
        sprintf(&message[pos],"%s,",log_path);
        pos += len+1;
        sprintf(log_other,"%d",args->args.fsync_args.len);
        len = strlen(log_other);
        memcpy(&message[pos], log_other, len);
        break;
    default:
        return ERR;
        break;
    }
    pos += len;
    message[pos] = '\n';
    pos += 1;
    message[pos] = '\0';
    return 0;
}


/**
 * @brief 往指定文件输出一条日志信息
 * 
 * @param args 要往文件写入的参数结构
 * @param logfile 要写入的文件名
 * @param err 访问是否允许
 * @return int -1: 成功 0: 失败
 */
int boesfs_log(struct boesfs_args * args, char * logfile, int err){
    struct log_info * log;
    log = (log_info *)kmalloc(sizeof(log_info),GFP_KERNEL);
    if(log == NULL)
        printk("ERROR: kmalloc fail!\n");


    char message[MESSAGE_MAX];

    memset(message, 0, MESSAGE_MAX);

    int ret = args_to_string(args, (char *)message, err);

    if(ret < 0)
        return ERR;

    memcpy(log->message, message, strlen(message) + 1);
    memcpy(log->logfile, logfile, strlen(logfile) + 1);

    log_write_kthread = kthread_run(log_func, log, "kthread-boesfs-log"); 
    if (!log_write_kthread) {
		printk("kthread_run fail");
		return ERR;
	}
    return 0;
}

/* 有时候貌似会有乱码？什么原因？ */

