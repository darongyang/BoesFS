#ifdef CONFIG_SANDFS

#include <linux/filter.h>
#include <linux/bpf.h>
#include <linux/perf_event.h>
#include <linux/bpf_perf_event.h>
#include "sandfs.h"

//  作用：运行bpf_prog来检测参数是否合法.
int sandfs_request_bpf_op(void *priv, struct sandfs_args *args)
{
	int ret = -ENOSYS;
	struct bpf_prog *accel_prog;

	if (!priv)
		return ret;
	//  根据sandsf.h中定义的结构体，该bpf_prog结构体地址与底层文件系统超级块被sandfs记录在同一结构体
	accel_prog = READ_ONCE(priv);
	//  READ_ONCE()是对volatile的封装，即每次检测重新从内存中读取一次该结构体
	//  这里就是SANDFS类型的 bpf prog对应的hook点！
	if (accel_prog) {
		/* run program */
		rcu_read_lock(); 
		//  2. ydr: 串行，ebpf prog的作用就是检测参数是否合法，检测之后再返回.
		// 代码BPF_PROG_RUN在include/linux/filter.h中. 就是把字节码放到虚拟机执行. 获取返回值
		//  3. lqc：根据Linux中的RCU锁机制，对于共享数据，获取读锁后所有读操作可并行执行
		//  解除所有读锁前所有写操作无法完成（执行但是未保存数据），从而保证prog的检测是正确的 
		ret = BPF_PROG_RUN(accel_prog, args);	
		rcu_read_unlock();
		printk("hook!\n");
	}

	return ret;
}

//  作用：感觉是清空superblock下保存的bpf prog地址
void sandfs_clear_bpf_ops(struct super_block *sb)
{
	struct bpf_prog *old_prog;

	BUG_ON(!sb || !SANDFS_SB(sb) || !SANDFS_SB(sb)->priv);

	printk(KERN_INFO "Clearing XDP operations\n");

	old_prog = xchg(&SANDFS_SB(sb)->priv, NULL);
	// 使用xchg交换两个地址的值，这里就是把保存的bpf prog地址置0
	if (old_prog)
		bpf_prog_put(old_prog);			
	//  空函数 . 见/include/linux/bpf.h

	printk(KERN_INFO "Cleared SANDFS ops\n");
}

//  作用: 将fd对应的ebpf prog挂到super_block中
int sandfs_set_bpf_ops(struct super_block *sb, int fd)
{
	struct bpf_prog *prog = NULL;
	struct bpf_prog *old_prog;
	struct sandfs_sb_info *sbi;

	BUG_ON(!sb);
	//	这里传入的fd参数就是bpf prog的文件描述符
	if (fd <= 0) {
		printk(KERN_ERR "Failed to setup sandfs bpf ops. "
			"Invalid prog_fd %d!\n", fd);
		return -EINVAL;
	}

	sbi = SANDFS_SB(sb);
	if (!sbi) {
		printk(KERN_ERR "Failed to setup sandfs bpf ops. "
			"NULL sb info!\n");
		return -EINVAL;
	}

	printk(KERN_INFO "Setting SANDFS bpf ops\n");
	//  获取bpf prog信息
	//  sandfs确实在 enum bpf_prog_type（/include/uapi/linux/bpf.h）里新增了BPF_PROG_TYPE_SANDFS. 文件已上传
	prog = bpf_prog_get_type(fd, BPF_PROG_TYPE_SANDFS);
	if (IS_ERR(prog))
		return -1;

	old_prog = xchg(&sbi->priv, prog);
	// 把绑定的bpf prog地址设置为上面拿到的新prog地址
	if (old_prog)
		bpf_prog_put(old_prog);

	printk(KERN_INFO "SANDFS bpf program updated\n");
	return 0;
}
// 返回一个指向当前进程的task_struct结构
static u64 bpf_get_current_task1(u64 r1, u64 r2, u64 r3, u64 r4, u64 r5)
{
	return (long) current;
}

static const struct bpf_func_proto bpf_get_current_task_proto = {
	.func		= bpf_get_current_task1,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,
};

//  对比下kernel原生的 long bpf_probe_read_str(void *dst, u32 size, const void *unsafe_ptr). 在4.10-rc8版本下，似乎并没有该函数. 所以sandfs自己添加了一个
//  把一个NULL终止的字符串从内存位置复制到BPF堆栈，以便BPF以后可以对其进行操作。
//  如果字符串长度小于size，则目标不会填充进一步的空字节。
//  如果字符串长度大于大小，则只复制大小-1字节，并将最后一个字节设置为NULL。
BPF_CALL_3(bpf_probe_read_str1, void *, dst, u32, size,
       const void *, unsafe_ptr)
{
    int ret;
    /*
     * The strncpy_from_unsafe() call will likely not fill the entire
     * buffer, but that's okay in this circumstance as we're probing
     * arbitrary memory anyway similar to bpf_probe_read() and might
     * as well probe the stack. Thus, memory is explicitly cleared
     * only in error case, so that improper users ignoring return
     * code altogether don't copy garbage; otherwise length of string
     * is returned that can be used for bpf_perf_event_output() et al.
     */
    ret = strncpy_from_unsafe(dst, unsafe_ptr, size);
    if (unlikely(ret < 0))
        memset(dst, 0, size);
	//  return 可用于bpf_perf_event_output()的长度
    return ret;
}
//  /include/linux/bpf.h
//  eBPF function prototype used by verifier to allow BPF_CALLs from eBPF programs
//  to in-kernel helper functions and for adjusting imm32 field in BPF_CALL instructions after verifying
//  被verifier使用的ebpf function prototype 允许 ebpf prog调用BPF_CALL到kernel的helper function. 并在验证后调整BPF_CALL中的imm32字段
static const struct bpf_func_proto bpf_probe_read_str_proto = {
    .func       = bpf_probe_read_str1,
    .gpl_only   = true,
    .ret_type   = RET_INTEGER,
	.arg1_type  = ARG_PTR_TO_RAW_STACK,
    .arg2_type  = ARG_CONST_STACK_SIZE,
    .arg3_type  = ARG_ANYTHING,
};

// 由于eBPF虚拟机的只有寄存器和栈，所以要访问其他内核空间或者用户控件地址，就需要借助bpf_probe_read系列辅助函数，eg
	// bpf_probe_read：从内存指针中读取数据
	// bpf_probe_read_user：从用户空间内存指针中读取数据
	// bpf_probe_read_kernel：从内核空间内存指针中读取数据
//  为ebpf prog提供helper function : bpf_probe_read1
BPF_CALL_3(bpf_probe_read1, void *, dst, u32, size, const void *, unsafe_ptr)
{
	int ret;
	ret = probe_kernel_read(dst, unsafe_ptr, size);
	// probe_kernel_read()通过__probe_kernel_read()安全的将用户空间地址unsafe_ptr开始的大小为size的数据拷贝到内核空间地址dst
	if (unlikely(ret < 0))
		memset(dst, 0, size);

	return ret;
}
//  与load_prog相关. 见sycall.c fixup_bpf_calls()
static const struct bpf_func_proto bpf_probe_read_proto = {
	.func		= bpf_probe_read1,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type  = ARG_PTR_TO_RAW_STACK,
    .arg2_type  = ARG_CONST_STACK_SIZE,
	.arg3_type	= ARG_ANYTHING,
};

static __always_inline u64
__bpf_perf_event_output(struct pt_regs *regs, struct bpf_map *map,
			u64 flags, struct perf_raw_record *raw)
{
	struct bpf_array *array = container_of(map, struct bpf_array, map);
	unsigned int cpu = smp_processor_id();
	u64 index = flags & BPF_F_INDEX_MASK;
	struct perf_sample_data sample_data;
	struct bpf_event_entry *ee;
	struct perf_event *event;

	if (index == BPF_F_CURRENT_CPU)
		index = cpu;
	if (unlikely(index >= array->map.max_entries))
		return -E2BIG;

	ee = READ_ONCE(array->ptrs[index]);
	if (!ee)
		return -ENOENT;

	event = ee->event;
	if (unlikely(event->attr.type != PERF_TYPE_SOFTWARE ||
		     event->attr.config != PERF_COUNT_SW_BPF_OUTPUT))
		return -EINVAL;

	if (unlikely(event->oncpu != cpu))
		return -EOPNOTSUPP;

	//  将原本在BPF_CALL_5中的调用移动到了__bpf_perf_event_output内部
	perf_sample_data_init(&sample_data, 0, 0);
	sample_data.raw = raw;

	perf_event_output(event, &sample_data, regs);
	return 0;
}

//  一大问题: 目前怀疑没用到. 感觉是调试用的？
//  我并不理解这个函数的用途何在，所以下面只是对比了一下和kernel原生函数的区别。xdm可以试着看看。
//  并且我也没有找到什么时候会调用到这个sandFs提供的bpf_perf_event_output1.

//  对比一下kernel原生的bpf_perf_event_output : 
	//  BPF_CALL_5(bpf_perf_event_output, struct pt_regs *, regs, struct bpf_map *, map, u64, flags, void *, data, u64, size)
//  作用：向类型为BPF_MAP_TYPE_PERF_EVENT_ARRAY 的map写入data
//  unused_regs : 在kernel原生的bpf_perf_output()中，是作为prog的上下文传入，要传递给helper function
//  map : 类型为BPF_MAP_TYPE_PERF_EVENT_ARRAY
//  flags :  用于指示map中的放置value的index，掩码是BPF_F_INDEX_MASK ；或者，flags可以设置BPF_F_CURRENT_CPU , 来指示应使用当前CPU的index
//  data : 要写入map的数据 
//  size :  The value to write, of size, is passed through eBPF stack and pointed by data。。。size通过ebpf stack传递，并且由value指向?
//  在用户态，一个prog要想read value，需要在perf event上调用perf_event_open()，并且将该fd存储到map中. 这必须在ebpf prog可以向其中send data之前完成. samples中有示例代码
//  bpf_perf_event_output() 在和user共享data的方面 取得了比bpf_trace_printk()更好的性能, 并且更适合来自ebpf prog的流式数据
//  success : return 0
BPF_CALL_5(bpf_perf_event_output1, struct pt_regs *, unused_regs/*FIXME*/, struct bpf_map *, map,
	   u64, flags, void *, data, u64, size)
{
	struct pt_regs regs = {0};
	//regs参数为空，即忽略上下文
	struct perf_raw_record raw = {
		.frag = {
			.size = size,
			.data = data,
		},
	};
	// linux kernel原先的
	// struct perf_sample_data *sd;
	// int err;
	// if (WARN_ON_ONCE(nest_level > ARRAY_SIZE(sds->sds))) {
	// 	err = -EBUSY;
	// 	goto out;
	// }
	// sd = &sds->sds[nest_level - 1];
	
	if (unlikely(flags & ~(BPF_F_INDEX_MASK)))		
		return -EINVAL;
	// linux kernel原先的
	// perf_sample_data_init(sd, 0, 0);
	// sd->raw = &raw;
	// sd->sample_flags |= PERF_SAMPLE_RAW;
	return __bpf_perf_event_output(&regs, map, flags, &raw);
}

static const struct bpf_func_proto bpf_perf_event_output_proto = {
	.func		= bpf_perf_event_output1,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_CONST_MAP_PTR,
	.arg3_type	= ARG_ANYTHING,
	.arg4_type  = ARG_PTR_TO_STACK,
    .arg5_type  = ARG_CONST_STACK_SIZE,
};

//  为ebpf prog提供helper function : bpf_sandfs_read_args
//  sandfs_read() -> get_path() -> bpf_sandfs_read_args()
//  作用：根据type(你要读取哪个参数)和size(你要读取的参数的大小)，正确的从p(sandfs_args)中读出数据，填入dst. 成功则返回0
	//  原因: bpf prog不能直接访问 kernel中的内存，必须通过helper function来进行访问.
	//  succes return 0
	//  fail 原因：使用bpf_sandfs_read_args传参不合法；probe_kernel_read失败
BPF_CALL_4(bpf_sandfs_read_args, void *, p, u32, type, void *, dst, u32, size)
{
	int ret = -EINVAL;
	const void *inptr = NULL;
	struct sandfs_args *args = (struct sandfs_args *)p;
	//  要操作的参数个数
	unsigned num_args = args->num_args;
	//  根据sandfs_args的type和size，选取适当的inptr  -->  为什么这么选取 ?
	//  因为如此一来，仅这1个bpf类系统调用就做到了获取不同数据的效果，而不需要定义更多的bpf类系统调用
	
	//  读取操作类型 : SANDFS_LOOKUP , SANDFS_READ ...
	if (type == OPCODE && size == sizeof(uint32_t))
		inptr = (void *)&args->op;
	//  读取操作参数个数  
	else if (type == NUM_ARGS && size == sizeof(uint32_t))
		inptr = (void *)&args->num_args;
	//  读取第0个操作参数的大小
	else if (type == PARAM_0_SIZE && size == sizeof(uint32_t) &&
			num_args >= 1 && num_args <= MAX_NUM_ARGS) {
		inptr = &args->args[0].size;
	}
	//  读取第0个操作参数的值
	else if (type == PARAM_0_VALUE && num_args >= 1 &&
			num_args <= MAX_NUM_ARGS) {
		ret = -E2BIG;
		if (size >= args->args[0].size) {
			size = args->args[0].size;
			inptr = args->args[0].value;
		}
	}
	// param 1	
	else if (type == PARAM_1_SIZE && size == sizeof(uint32_t) &&
			num_args >= 2 && num_args <= MAX_NUM_ARGS) {
		inptr = &args->args[1].size;
	}

	else if (type == PARAM_1_VALUE && num_args >= 2 &&
			num_args <= MAX_NUM_ARGS) {
		ret = -E2BIG;
		if (size >= args->args[1].size) {
			size = args->args[1].size;
			inptr = args->args[1].value;
		}
	}

	// param 2
	else if (type == PARAM_2_SIZE && size == sizeof(uint32_t) &&
			num_args >= 3 && num_args <= MAX_NUM_ARGS)
		inptr = &args->args[2].size;

	else if (type == PARAM_2_VALUE && num_args >= 3 &&
			num_args <= MAX_NUM_ARGS) {
		ret = -E2BIG;
		if (size >= args->args[2].size) {
			size = args->args[2].size;
			inptr = args->args[2].value;
		}
	}

	// param 3
	else if (type == PARAM_3_SIZE && size == sizeof(uint32_t) &&
			num_args == MAX_NUM_ARGS)
		inptr = &args->args[3].size;

	else if (type == PARAM_3_VALUE && num_args == MAX_NUM_ARGS) {
		ret = -E2BIG;
		if (size >= args->args[3].size) {
			size = args->args[3].size;
			inptr = args->args[3].value;
		}
	}
	//  bpf prog 调用 helper function 传参错误. 
	//  或者可以说 bpf prog 想要读取的内容在sandfs_agrs中并不存在
	if (!inptr) {
		printk(KERN_ERR "Invalid input to sandfs_read_args"
			"type: %d num_args: %d size: %d\n",
			type, num_args, size);
		return ret;
	}

	//  probe_kernel_read 探测inptr是否可读，出错就返回非0
		//  将要读取的值 从kernel(inptr) 拷贝到 bpf prog的空间(dst)中   
	ret = probe_kernel_read(dst, inptr, size);
	//  读取失败，清空bpf prog的dst
	if (unlikely(ret < 0))
		memset(dst, 0, size);

	return ret;
}

static const struct bpf_func_proto bpf_sandfs_read_args_proto = {
	.func		= bpf_sandfs_read_args,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING, //ARG_CONST_SIZE_OR_ZERO,
	.arg3_type  = ARG_PTR_TO_RAW_STACK,
    .arg4_type  = ARG_CONST_STACK_SIZE,
};

//  为ebpf prog提供helper function : bpf_sandfs_write_args
//  作用：根据type(你要读取哪个参数)和size(你要读取的参数的大小)，用src中的数据，改写p(sandfs_args)中的参数
	//  其实也没被调用
BPF_CALL_4(bpf_sandfs_write_args, void *, p, u32, type, const void *,
		src, u32, size)
{
	int ret = -EINVAL;
	void *outptr = NULL;
	struct sandfs_args *args = (struct sandfs_args *)p;
	unsigned numargs = args->num_args;
	if (type == PARAM_0_VALUE && numargs >= 1 &&
		numargs <= MAX_NUM_ARGS && size == args->args[0].size)
		outptr = args->args[0].value;

	else if (type == PARAM_1_VALUE && numargs >= 2 &&
			numargs <= MAX_NUM_ARGS && size == args->args[1].size)
		outptr = args->args[1].value;

	else if (type == PARAM_2_VALUE && numargs >= 3 &&
			numargs <= MAX_NUM_ARGS && size == args->args[2].size)
		outptr = args->args[2].value;

	else if (type == PARAM_3_VALUE && numargs == MAX_NUM_ARGS &&
			size == args->args[1].size)
		outptr = args->args[3].value;

	if (!outptr) {
		printk(KERN_ERR "Invalid input to sandfs_write_args type: %d "
				"num_args: %d size: %d\n", type, numargs, size);
		return ret;
	}
	//  bpf prog向kernel写数据(探测outptr是否可写，否则返回非0)
		//  src copy 到 outptr
	ret = probe_kernel_write(outptr, src, size);
	if (unlikely(ret < 0))
		memset(outptr, 0, size);

	return ret;
}

static const struct bpf_func_proto bpf_sandfs_write_args_proto = {
	.func		= bpf_sandfs_write_args,			//  这个地址就是上述BPF_CALL_4(bpf_sandfs_write_args)的地址. 我们先默认他们就是这样. 怎么做到的我还不知道. 合理怀疑是一个entry.S进行了记录
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING, //ARG_CONST_SIZE_OR_ZERO,
	.arg3_type  = ARG_PTR_TO_STACK,
    .arg4_type  = ARG_CONST_STACK_SIZE,
};

//  设定sandfs bpf prog的helper function
	//  根据id返回bpf_func_proto. 与load_prog相关. 见sycall.c fixup_bpf_calls()
static
const struct bpf_func_proto *sandfs_prog_func_proto(enum bpf_func_id func_id)
{
	switch (func_id) {
	case BPF_FUNC_map_lookup_elem:
		return &bpf_map_lookup_elem_proto;
	case BPF_FUNC_map_update_elem:
		return &bpf_map_update_elem_proto;
	case BPF_FUNC_map_delete_elem:
		return &bpf_map_delete_elem_proto;
	case BPF_FUNC_probe_read:
		return &bpf_probe_read_proto;
	case BPF_FUNC_ktime_get_ns:
		return &bpf_ktime_get_ns_proto;
	case BPF_FUNC_get_current_pid_tgid:
		return &bpf_get_current_pid_tgid_proto;
	case BPF_FUNC_get_current_task:
		return &bpf_get_current_task_proto;
	case BPF_FUNC_get_current_uid_gid:
		return &bpf_get_current_uid_gid_proto;
	case BPF_FUNC_get_current_comm:
		return &bpf_get_current_comm_proto;
	case BPF_FUNC_trace_printk:
		return bpf_get_trace_printk_proto();
	case BPF_FUNC_get_smp_processor_id:
		return &bpf_get_smp_processor_id_proto;
	case BPF_FUNC_get_prandom_u32:
		return &bpf_get_prandom_u32_proto;
	case BPF_FUNC_perf_event_output:
		return &bpf_perf_event_output_proto;
	case BPF_FUNC_probe_read_str:
		return &bpf_probe_read_str_proto;
	// SANDFS related
	case BPF_FUNC_sandfs_read_args:
		return &bpf_sandfs_read_args_proto;
	case BPF_FUNC_sandfs_write_args:
		return &bpf_sandfs_write_args_proto;
	default:
		return NULL;
	}
}

/* bpf+sandfs programs can access fields of 'struct pt_regs' */
//  the callback is used to customize verifier to restrict eBPF program access to only certain fields within ctx structure with specified size and alignment.
//  For example, the following insn:
	//  bpf_ld R0 = *(u32 *)(R6 + 8)
	//  intends to load a word from address R6 + 8 and store it into R0 If R6=PTR_TO_CTX, via is_valid_access() callback the verifier will know that offset 8 of size 4 bytes can be accessed for reading, otherwise the verifier will reject the program.
	//  https://www.kernel.org/doc/html/v5.15//networking/filter.html
	//  https://blog.csdn.net/Linux_Everything/article/details/127698670
	//  目前认为是限制了 bpf prog对于kernel传入导bpf prog的上下文ctx的合法范围: 只读，且范围不能超过struct sandfs_args
static bool sandfs_prog_is_valid_access(int off, int size, enum bpf_access_type type,
					enum bpf_reg_type *reg_type)
{
	if (off < 0 || off >= sizeof(struct sandfs_args))
		return false;
	//  操作类型如果是 write 则不允许
	if (type != BPF_READ)
		return false;
	if (off % size != 0)
		return false;
	/*
	 * Assertion for 32 bit to make sure last 8 byte access
	 * (BPF_DW) to the last 4 byte member is disallowed.
	 */
	//  访问范围不能超过sandfs_args结构体
	if (off + size > sizeof(struct sandfs_args))
		return false;

	return true;
}

//  sandfs_prog_func_proto
	//  与load_prog相关 作用：根据id获取到该prog的正确的helper function.
	//  见sycall.c fixup_bpf_calls()
//  is valid access	：用于执行前的安全检测.
	//  sandfs_prog_is_valid_access
	//  checks if the read/write access for the memory offset is valid
static const struct bpf_verifier_ops sandfs_prog_ops = {
	.get_func_proto  = sandfs_prog_func_proto,
	.is_valid_access = sandfs_prog_is_valid_access,
};

//  内核中没找到该bpf_prog_type_list类型的定义. 是sandfs自己加的吗?
static struct bpf_prog_type_list sandfs_tl __ro_after_init = {
	.ops	= &sandfs_prog_ops,
	.type	= BPF_PROG_TYPE_SANDFS,
};

int __init sandfs_register_bpf_prog_ops(void)
{
	bpf_register_prog_type(&sandfs_tl);
	printk(KERN_INFO "Registered SANDFS operations\n");
	return 0;
}

#endif
