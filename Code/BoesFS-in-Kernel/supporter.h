#include <linux/filter.h>
#include <linux/perf_event.h>
#include <linux/bpf_perf_event.h>
#include <linux/bpf.h>

// kill.c
#include <uapi/asm-generic/siginfo.h>
#include <linux/types.h>



/* -------------------- 内核符号指针声明  START -------------------- */
typedef const struct bpf_func_proto * bpf_ktime_get_ns_proto_t;
typedef const struct bpf_func_proto * bpf_map_delete_elem_proto_t;
typedef typeof(bpf_get_trace_printk_proto) * bpf_get_trace_printk_proto_t;
typedef void (*perf_event_output_t)(struct perf_event *event, struct perf_sample_data *data,struct pt_regs *regs);
typedef const struct bpf_func_proto * bpf_get_current_pid_tgid_proto_t;
typedef const struct bpf_func_proto * bpf_get_smp_processor_id_proto_t;
typedef const struct bpf_func_proto * bpf_map_lookup_elem_proto_t;
typedef const struct bpf_func_proto * bpf_get_current_comm_proto_t;
typedef long (*strncpy_from_unsafe_t)(char *dst, const void *unsafe_addr, long count);
typedef const struct bpf_func_proto * bpf_map_update_elem_proto_t;
typedef const struct bpf_func_proto * bpf_get_current_uid_gid_proto_t;
typedef const struct bpf_func_proto * bpf_get_prandom_u32_proto_t;
typedef typeof(bpf_register_prog_type) * bpf_register_prog_type_t;
typedef const struct bpf_func_proto * bpf_probe_read_proto_t;
typedef const struct bpf_func_proto * bpf_get_current_task_proto_t;
typedef const struct bpf_func_proto * bpf_perf_event_output_proto_t;
typedef const struct bpf_func_proto * bpf_probe_read_str_proto_t;
typedef struct list_head * list_head_t;
// kill.c
typedef int (*__kill_pgrp_info_t)(int sig, struct siginfo *info, struct pid *pgrp);

/* -------------------- 内核符号指针声明 END -------------------- */



/* -------------------- extern导出  START -------------------- */
extern bpf_ktime_get_ns_proto_t k_bpf_ktime_get_ns_proto;
extern bpf_map_delete_elem_proto_t k_bpf_map_delete_elem_proto;
extern bpf_get_trace_printk_proto_t k_bpf_get_trace_printk_proto;
extern perf_event_output_t k_perf_event_output;
extern bpf_get_current_pid_tgid_proto_t k_bpf_get_current_pid_tgid_proto;
extern bpf_get_smp_processor_id_proto_t k_bpf_get_smp_processor_id_proto;
extern bpf_map_lookup_elem_proto_t k_bpf_map_lookup_elem_proto;
extern bpf_get_current_comm_proto_t k_bpf_get_current_comm_proto;
extern strncpy_from_unsafe_t k_strncpy_from_unsafe;
extern bpf_map_update_elem_proto_t k_bpf_map_update_elem_proto;
extern bpf_get_current_uid_gid_proto_t k_bpf_get_current_uid_gid_proto;
extern bpf_get_prandom_u32_proto_t k_bpf_get_prandom_u32_proto;
extern bpf_register_prog_type_t k_bpf_register_prog_type;
extern bpf_probe_read_proto_t k_bpf_probe_read_proto;
extern bpf_get_current_task_proto_t k_bpf_get_current_task_proto;
extern bpf_perf_event_output_proto_t k_bpf_perf_event_output_proto;
extern bpf_probe_read_str_proto_t k_bpf_probe_read_str_proto;
extern list_head_t k_bpf_prog_head;

extern __kill_pgrp_info_t k___kill_pgrp_info;
extern rwlock_t * k_tasklist_lock;
/* -------------------- extern导出  END -------------------- */
