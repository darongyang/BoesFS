#include "supporter.h"
#include "boesfs.h"

static struct kprobe kp; // 声明一个kprobe 结构体
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);


static kallsyms_lookup_name_t k_kallsyms_lookup_name = NULL;

/* ---------  封装supporter函数 START ----------- */ 
// 找到内核符号 name 对应的地址
// 流程：kprobe 得到 kallsyms_lookup_name 的地址 -> kallsyms_lookup_name(name)得到 name 对应
unsigned long my_kallsyms_lookup(const char *name){
    if(k_kallsyms_lookup_name == NULL){ // 如果直接获取失败，则用kprobe方式
        kp.symbol_name = "kallsyms_lookup_name"; // 传入内核符号名
        if(!register_kprobe(&kp)){ // 如果返回0，注册成功
            k_kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr; // 获取kallsmy_lookup_name函数地址
            unregister_kprobe(&kp); // 取消探测点
        }
    }
    // 调用orig_kallsyms_lookup_name
    return k_kallsyms_lookup_name ? k_kallsyms_lookup_name(name) : 0;
}
/* ---------  封装supporter函数 END ----------- */ 


/* ------------------------- 以上部分是固定部分，不需要修改 ----------------------*/


/* ------------------------ 以下部分是根据LKM需要的内核符号，进行修改的部分 ----------*/
// 修改涉及的文件:supporter.h、supporter.c、main.c、bpf.c、Makefile

// 修改的流程：1.添加符号指针声明(supporter.h)
//            2.添加extern导出(supporter.h)
//            3.添加符号声明(supporter.c)
//            4.补充get_all_symbol_addr添加获取符号地址逻辑(supporter.c)
//            5.修改init添加get_all_symbol_addr，并#include "supporter.h"(main.c)
//            6.替换bpf.c中的为自己获取的内核符号，并#include "supporter.h"(bpf.c)
//            7.修改Makefile保证supporter.o在bpf.o前(Makefile)

/* ---------------  需要获取的内核符号声明 START ------------------- */
bpf_ktime_get_ns_proto_t k_bpf_ktime_get_ns_proto = NULL;
bpf_map_delete_elem_proto_t k_bpf_map_delete_elem_proto = NULL;
bpf_get_trace_printk_proto_t k_bpf_get_trace_printk_proto = NULL;
perf_event_output_t k_perf_event_output = NULL;
bpf_get_current_pid_tgid_proto_t k_bpf_get_current_pid_tgid_proto = NULL;
bpf_get_smp_processor_id_proto_t k_bpf_get_smp_processor_id_proto = NULL;
bpf_map_lookup_elem_proto_t k_bpf_map_lookup_elem_proto= NULL;
bpf_get_current_comm_proto_t k_bpf_get_current_comm_proto = NULL;
strncpy_from_unsafe_t k_strncpy_from_unsafe = NULL;
bpf_map_update_elem_proto_t k_bpf_map_update_elem_proto = NULL;
bpf_get_current_uid_gid_proto_t k_bpf_get_current_uid_gid_proto = NULL;
bpf_get_prandom_u32_proto_t k_bpf_get_prandom_u32_proto = NULL;
bpf_register_prog_type_t k_bpf_register_prog_type = NULL;
bpf_probe_read_proto_t k_bpf_probe_read_proto = NULL;
bpf_get_current_task_proto_t k_bpf_get_current_task_proto = NULL;
bpf_perf_event_output_proto_t k_bpf_perf_event_output_proto = NULL;
bpf_probe_read_str_proto_t k_bpf_probe_read_str_proto = NULL;
list_head_t k_bpf_prog_head = NULL;

__kill_pgrp_info_t k___kill_pgrp_info = NULL;
rwlock_t * k_tasklist_lock = NULL;
/* --------------- 需要获取的内核符号声明 END ------------------- */


/* --------------- 需要获取的符号地址 START ------------------- */
// 获取全部所需的内核符号
// 在main.c的init函数调用该函数即可
void get_all_symbol_addr(void){
    k_bpf_ktime_get_ns_proto = (bpf_ktime_get_ns_proto_t)my_kallsyms_lookup("bpf_ktime_get_ns_proto");
	k_bpf_map_delete_elem_proto = (bpf_map_delete_elem_proto_t)my_kallsyms_lookup("bpf_map_delete_elem_proto");
	k_bpf_get_trace_printk_proto = (bpf_get_trace_printk_proto_t)my_kallsyms_lookup("bpf_get_trace_printk_proto");
	k_perf_event_output = (perf_event_output_t)my_kallsyms_lookup("perf_event_output");
	k_bpf_get_current_pid_tgid_proto = (bpf_get_current_pid_tgid_proto_t)my_kallsyms_lookup("bpf_get_current_pid_tgid_proto");
	k_bpf_get_smp_processor_id_proto = (bpf_get_smp_processor_id_proto_t)my_kallsyms_lookup("bpf_get_smp_processor_id_proto");
	k_bpf_map_lookup_elem_proto = (bpf_map_lookup_elem_proto_t)my_kallsyms_lookup("bpf_map_lookup_elem_proto");
	k_bpf_get_current_comm_proto = (bpf_get_current_comm_proto_t)my_kallsyms_lookup("bpf_get_current_comm_proto");
	k_strncpy_from_unsafe = (strncpy_from_unsafe_t)my_kallsyms_lookup("strncpy_from_unsafe");
	k_bpf_map_update_elem_proto = (bpf_map_update_elem_proto_t)my_kallsyms_lookup("bpf_map_update_elem_proto");
	k_bpf_get_current_uid_gid_proto = (bpf_get_current_uid_gid_proto_t)my_kallsyms_lookup("bpf_get_current_uid_gid_proto");
	k_bpf_get_prandom_u32_proto = (bpf_get_prandom_u32_proto_t)my_kallsyms_lookup("bpf_get_prandom_u32_proto");
	k_bpf_register_prog_type = (bpf_register_prog_type_t)my_kallsyms_lookup("bpf_register_prog_type");
	k_bpf_probe_read_proto = (bpf_probe_read_proto_t)my_kallsyms_lookup("bpf_probe_read_proto");
	k_bpf_get_current_task_proto = (bpf_get_current_task_proto_t)my_kallsyms_lookup("bpf_get_current_task_proto");
	k_bpf_perf_event_output_proto = (bpf_perf_event_output_proto_t)my_kallsyms_lookup("bpf_perf_event_output_proto");
	k_bpf_probe_read_str_proto = (bpf_probe_read_str_proto_t)my_kallsyms_lookup("bpf_probe_read_str_proto");
	k___kill_pgrp_info = (__kill_pgrp_info_t)my_kallsyms_lookup("__kill_pgrp_info");
    k_tasklist_lock = (rwlock_t *)my_kallsyms_lookup("tasklist_lock");
	k_bpf_prog_head = (list_head_t)my_kallsyms_lookup("bpf_prog_types");
}
/* --------------- 需要获取的符号地址 END ------------------- */