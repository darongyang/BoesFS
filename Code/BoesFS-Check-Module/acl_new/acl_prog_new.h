#ifndef _ACL_PROG_H_
#define _ACL_PROG_H_

#include<linux/types.h>
/***** helper function proto ****/
#include "bpf_helpers.h"

/********* acl struct **********/
#include "acl_public_new.h"         						//  你的acl_public.h路径

/********* boesfs_op_t , struct args **********/
#include "ebpf_helper.h"     							//  你的bpf_helper.h路径

/********* debug macro**********/
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

typedef int boesfs_op_t;

#endif