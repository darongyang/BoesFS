#include "supporter.h"


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

struct bpf_map_ops *k_array_ops = NULL;
bpf_map_update_elem_proto_t k_bpf_map_update_elem_proto = NULL;
bpf_map_lookup_elem_proto_t k_bpf_map_lookup_elem_proto= NULL;
/* --------------- 需要获取的内核符号声明 END ------------------- */


/* --------------- 需要获取的符号地址 START ------------------- */
// 获取全部所需的内核符号
// 在main.c的init函数调用该函数即可
void get_all_symbol_addr(void){
	k_array_ops = (struct bpf_map_ops*)kallsyms_lookup_name("htab_ops");
	k_bpf_map_update_elem_proto = (bpf_map_update_elem_proto_t)kallsyms_lookup_name("bpf_map_update_elem_proto");
	k_bpf_map_lookup_elem_proto = (bpf_map_lookup_elem_proto_t)kallsyms_lookup_name("bpf_map_lookup_elem_proto");
}
/* --------------- 需要获取的符号地址 END ------------------- */