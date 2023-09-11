#include "supporter.h"


/* ---------------  需要获取的内核符号声明 START ------------------- */

struct bpf_map_ops *k_htab_ops = NULL;
bpf_map_update_elem_proto_t k_bpf_map_update_elem_proto = NULL;
bpf_map_lookup_elem_proto_t k_bpf_map_lookup_elem_proto= NULL;
/* --------------- 需要获取的内核符号声明 END ------------------- */


/* --------------- 需要获取的符号地址 START ------------------- */
// 获取全部所需的内核符号
// 在main.c的init函数调用该函数即可
void get_all_symbol_addr(void){
	k_htab_ops = (struct bpf_map_ops*)kallsyms_lookup_name("htab_ops");
	k_bpf_map_update_elem_proto = (bpf_map_update_elem_proto_t)kallsyms_lookup_name("bpf_map_update_elem_proto");
	k_bpf_map_lookup_elem_proto = (bpf_map_lookup_elem_proto_t)kallsyms_lookup_name("bpf_map_lookup_elem_proto");
}
/* --------------- 需要获取的符号地址 END ------------------- */