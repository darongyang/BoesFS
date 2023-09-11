### 概述

由于ebpf程序的验证和运行的过程都发生在内核态中，有一些需要被内核识别的声明和定义必须添加进内核文件中，但是本项目为了追求更少的内核修改，尽可能地将大部分与内核交互的代码封装到项目的内核模块中，因此整个项目中仅对内核添加了4行代码

### 改动位置

为了让源码能够识别新的bpf prog类型和新的bpf helper function，我们对需要重新编译的内核源码进行了极少的修改：

在tools/include/uapi/linux/bpf.h文件里，由于bpf_prog_type为枚举类型，我们在定义新的bpf prog类型时必须往改类型中添加，因此我们在这里添加了我们自己的bpf prog类型——BPF_PROG_TYPE_BOESFS，在第115行添加了

```c
	BPF_PROG_TYPE_BOESFS,
```

并且该文件还定义了BPF helper functions的映射，内核必须从此处获取注册的BPF helper function id。如果我们的prog要使用我们自定义的BPF helper functions，那么必须在此处添加BPF helper functions的注册，因此我们添加了我们自己需要使用的两个自己新定义bpf helper function进行注册，在第508-512行添加了

```c
 	FN(boesfs_read_args),	\
	FN(boesfs_acl_dfs_path),	\
	FN(boesfs_destroy_dir_tree),	\
	FN(boesfs_build_dir_tree),	\
	FN(boesfs_args_hash),
```

除此之外，在kernel/bpf/syscall.c文件里，为了让非特权用户也能使用我们的BOESFS类型的prog，我们需要添加代码使内核检测使用者是否为特权用户时，跳过类型为BPF_PROG_TYPE_BOESFS的prog的检查，把第868行修改为：

```c
	if (type != BPF_PROG_TYPE_SOCKET_FILTER && type != BPF_PROG_TYPE_BOESFS && !capable(CAP_SYS_ADMIN))
```
