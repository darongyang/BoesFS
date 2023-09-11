/* EBPF library */
#ifndef __LIBEBPF_H
#define __LIBEBPF_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/bpf.h>
#include <linux/version.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <pwd.h>

#include <libbpf.h>
#include <bpf_load.h>
#include <acl_public.h>
#include <utils.h>

#define PASSTHRU 1
#define RETURN 0
#define UPCALL -ENOSYS

#ifndef MAX_MAPS
#undef MAX_MAPS
#define MAX_MAPS 32
#endif

#define BOESFS_READ 	2
#define BOESFS_WRITE 	3
#define BOESFS_LOOKUP	4
#define BOESFS_OPEN	5
#define BOESFS_MKDIR	6
#define BOESFS_UNLINK	7
#define BOESFS_RMDIR	8
#define BOESFS_MKNOD	9
#define BOESFS_CREATE	10
#define BOESFS_LINK	11
#define BOESFS_SYMLINK	12
#define BOESFS_RENAME	13
#define BOESFS_SETATTR	14
#define BOESFS_GETATTR	15
#define BOESFS_LLSEEK	16
#define BOESFS_ITERATE	17
#define BOESFS_MMAP	18
#define BOESFS_LOOKUP2	19
#define BOESFS_STATFS	20
#define BOESFS_FSYNC	21
#define BOESFS_DEFAULT  22


typedef struct ebpf_context {
        int ctrl_fd;
        int data_fd[MAX_MAPS];
}ebpf_ctxtext_t;

typedef struct{
        char role[30];
        struct sub_list *next;
}sub_list;

typedef struct{
        char role[30];
        int op;
        uint32_t args_hash;
        struct act_args_list *next;
}act_args_list;

/* init/finalize */
ebpf_ctxtext_t* ebpf_init(char *filename, int index);
void ebpf_fini(ebpf_ctxtext_t *main);
int read_acl_rbac(ebpf_ctxtext_t *ctx, char *model_file, char *rule_file, char *err, char *elf);

/* Data handling abstractions */
int ebpf_data_next(ebpf_ctxtext_t *context, void *key, void *next, int idx);
int ebpf_data_lookup(ebpf_ctxtext_t *context, void *key, void *val, int idx);
int ebpf_data_update(ebpf_ctxtext_t *context, void *key, void *val, int idx,
		int overwrite);
int ebpf_data_delete(ebpf_ctxtext_t *context, void *key, int idx);

#endif
