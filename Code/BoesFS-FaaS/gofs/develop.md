2023-5-18 *Development* 成功调用`github.com/cilium/ebpf`与ebpf文件进行交互

2023-5-21 *ProblemSolving* 分析了无法加载`github.com/cilium/ebpf/examples/cgroup_skb/cgroup_skb.c`的原因(ebpf文件或头文件与Linux版本不匹配), 成功加载了`linux-4.11.12/samples/bpf/sockex1_kern.c`

2023-5-21 *ProblemSolving* 解决了`src/bpf.go:33   LoadAndAssign   {"error": "field bpfProg: can't set value"}`问题. 原因: 结构体字段名首字母小写, 该field的value具有flagRO属性, `github.com/cilium/ebpf/collection.go:731 field.value.CanSet()`为false. 解决方法: 将变量名`bpfProg`改为`BpfProg`

2023-5-22 *Development* 实现了clone子进程到新的namespace中

2023-5-23 *ProblemSolving* 解决了`fork/exec ./example/cloneMain.go: no such file or directory`问题. 原因: 需要在前面加上go run命令

2023-5-23 *ProblemSolving* 解决了`fork/exec go: no such file or directory`问题. 原因: PATH中没有go运行环境. 暂时解决方法: 使用绝对路径

2023-5-23 *ProblemSolving* 解决了子进程无法通过管道读取数据的问题. 原因: bufio会将数据先缓存而不是直接写入. 解决方法: 直接使用io.WriteString

2023-5-23 14:36:27 *ProblemSolving* 解决了找不到go文件路径的问题. 解决方法: 使用`exec.LookPath("go")`获取go二进制文件, 使用`os.Getwd()`获取当前工作目录, 以适应相对路径.

2023-5-23 14:40:45 *Rename* 更改了`example/`路径下文件的命名格式, 以`example`开头, 便于辨识.

2023-6-26 14:32:04 *Documentation* 添加了`tags.md`, 记录各种标记的含义

2023-6-26 20:00:35 *Development* 主框架基本建立, 进入联调阶段

2023-6-27 16:01:54 *ProblemSolving* 解决了`LoadAndAssign	{"error": "field BpfProg: cannot load program boesfs_main_handler: program type is unspecified"`问题. 解决方法: 修改`github.com/cilium/ebpf`框架代码, 在`types.go`中添加`Boesfs`种类

2023-6-27 20:16:03 *ProblemSolving* 解决了`invalid array index`问题. 解决方法: 手动修改框架代码`types_string.go`的相关代码

2023-6-28 10:40:47 *ProblemSolving* 成功安装`stringer`程序, 进一步确认了本应自动生成的`types_string.go`代码正确性. 解决方法: 使用`go install`而不是`go get`下载stringer

2023-6-28 14:04:05 *ProblemSolving* 解决了项目C语言部分无法成功运行的问题, 错误
```
failed to create a map: 1 Operation not permitted
Failed to load bpf prog!
```
解决方法: 修改/etc/security/limits.conf, 对root用户添加memclock上限
```
boes    hard    memlock 4096
root    hard    memlock 4096
boes    soft    memlock 4096
root    soft    memlock 4096
```

2023-6-30 19:34:05 *Development* 提出将C Agent部分代码编译为动态链接库, 使用go调用, 以解决ebpf无法加载问题

2023-7-1 01:23:57 *ProblemSolving* 解决了`#cgo LDFLAGS: -L.share`设置动态链接库寻找路径后仍然无法找到库的问题. 暂时解决方法: 使用`export LD_LIBRARY_PATH=./share`命令设置动态链接库寻找路径.

2023-7-1 16:44:39 *ProblemSolving* 解决了动态链接库难以编译的问题. 解决方法: 使用C Agent部分编译的.o文件, 直接将其链接为动态链接库.

2023-7-2 01:34:36 *ProblemSolving* 成功加载boesfs类型的ebpf字节码`user_prog.o`

2023-7-2 10:31:25 *Development* 成功将加载字节码部分程序整合进go前端中.

2023-7-3 11:14:28 *ProblemSolving* 解决了`ebpf.NewMapFromFD`函数报`get map info: /proc/self/fdinfo/9: not supported`错误的问题. 原因: 该函数调用了`BPF_OBJ_GET_INFO_BY_FD`系统调用来获取map信息, 而本系统版本似乎并不支持该系统调用. 解决方法: 在库中添加`NewMapFromFDAndInfo`函数, 直接传入`ebpf.MapInfo`结构体构造`ebpf.Map`结构体, 没有选择直接构造`ebpf.Map`结构体的原因是该结构体的属性均为小写, 无法在包外引用.

2023-7-3 11:24:37 *ProblemSolving* 解决了`src/bpf.go:30:2: use of internal package github.com/cilium/ebpf/internal/sys not allowed`问题. 原因: 在构建`sys.Fd`结构体时, 导入了一个internal的包, 无法导入该库的内部包. 解决方法: 重新声明`NewMapFromFDAndInfo`函数, 将参数`*sys.FD`改为`int`, 避免在当前项目中引用其他项目的internal包

2023-7-3 12:35:15 *ProblemSolving* 解决了`update: invalid argument`等错误. 原因: 传入fd参数时将`data_fd`错传为`ctrl_fd`. 将`NewMapFromFDAndInfo`函数删除.

2023-7-3 12:50:27 *Development* 成功实现go前端与ebpf map的交互.

2023-7-3 13:03:34 *TODO* 完成了`src/myLog.go:31 getWriteSync TODO: 未判断路径合理性`

2023-7-3 13:37:28 *TODO* 完成了`src/bpf.go:58 LoadProgC TODO 可以考虑将返回值结构体中改为Map, 因为NewMapFromFD函数注释中提示, 使用此函数之后不应该继续使用fd`

2023-7-3 17:16:01 *Development* 初赛部分主框架搭建完成, 还未进行测试和bug修复

2023-7-6 21:52:57 *Development* 编写了简易的测试代码, 但是基本上无法访问任何文件, 显示没有权限

2023-7-9 23:06:26 *ProblemSolving* 对之前的权限问题进行分阶段测试, 首先测试创建namespace时期的相关问题, 发现使用root用户运行程序时, 在子ns中, 无法在其他用户家目录中创建删除文件, 可以读写文件, 在使用boes用户运行程序时, 在子ns中, 无法访问`/root`目录下的文件. 阅读了[详解Linux Namespace之User](https://cloud.tencent.com/developer/article/1721820), 学习了Linux User-Namespace的相关知识, 初步了解了权限问题在namespace方面的原因. 了解到了创建namespace中的权限问题原理: 当访问其它 user namespace 里的资源时, 是以其它 user namespace 中的相应用户的权限来执行的, 比如这里 root 对应父 user namespace 的用户是 boes, 所以无法访问`/root`下的文件

2023-7-9 23:42:16 *ProblemSolving* 定位到父ns中的进程无法成功wait子ns中初始进程的原因: 子ns中的初始进程将特定的目录挂载了boesfs文件系统, 更具体原因未知

2023-7-10 10:54:57 *ProblemSolving* 对之前主程序无法wait子程序进行了分析, 根源是`cloneMain.go`中将boesfs文件系统进行了mount. 后添加unmount, 但是`cloneMain.go`中的Unmount会一直卡死, 即使没有Unmount, 该程序依然会在最后卡死, 导致主程序无法wait. 尝试多种flag进行unmount, 使用`MNT_EXPIRE`报`resource temporarily unavailable`, 使用`MNT_DETACH|MNT_FORCE`报`operation not permitted`(可能是这两个flag不兼容), 使用其他flag无用, 仍然卡死. 每次程序运行后, boesfs内核模块的引用都会加一, rmmod时报`Module boesfs is in use`. 未解决问题, 暂时搁置.

2023-7-10 18:55:34 *ProblemSolving* 分析unmount时使用`syscall.MNT_FORCE|syscall.MNT_DETACH`报`operation not permitted`的错误, 可能是由于boesfs不允许此操作, 应当先仔细分析与ebpf的交互.

2023-7-10 20:32:28 *ProblemSolving* 解决了测试用的python脚本无法正常输出的问题. 原因: python logging库中的StreamHandler默认选用`sys.stderr`, 采用`sys.stdout`则成功输出. 可能是由于这里的stderr被go截获.

2023-7-10 23:31:01 *ProblemSolving* 通过查看`dmesg`, 得知内核没有通过传入的fd正确获取到prog, 初步认为是启动子程序时没有复制文件描述符, 导致父ns中的fd实际上不在子ns第一个进程的文件描述表中.

2023-7-11 00:15:14 *ProblemSolving* 解决了父子进程之间传递文件描述符, 子进程关闭文件描述符后报`fatal error: runtime: netpoll: break fd ready for something unexpected`的问题. 原因: 使用`exec.Cmd.ExtraFiles`传递文件描述符时, 子进程中并不会保留该文件描述符原本的序号, 根据注释, `entry i becomes file descriptor 3+i.`, 故子进程中该文件描述符为3.

2023-7-11 00:28:21 *ProblemSolving* 解决了`unmount`时报`operation not permitted`的问题, 解决了父ns中的进程无法成功wait子ns中初始进程的问题. 总体上的问题是之前未复制文件描述符, 传递给内核的fd不合法, 内核报`ERROR: get prog from fd fail!`. 解决方法: 直接使用`syscall.Unmount(*mountDir, 0)`, 去掉任何flag

2023-7-11 00:35:49 *Development* 基本完成了初赛`user_prog`部分的代码, 且测试中未出现bug. 可以尽快写完初赛部分, 并尽快跟上决赛进度.

2023-7-11 21:38:05 *ProblemSolving* 解决了`p\s*,\s*(([\w/.]+)\s*,\s*){2}(file|dir)\s*,\s*(allow|deny)`只能返回第一个匹配的问题, 解决方法: 将`(something){2}`进行展开, 改写为`(something)(something)`

2023-7-12 16:03:04 *ProblemSolving* 解决了查找map时, 由于返回值为C中定义的结构体`policy_t`, 程序无法解析, 报`panic: reflect: reflect.Value.SetUint using value obtained using unexported field`的问题. 解决方法: 将`policy_t`中的两个`uint32`(C语言)视为一个`uint64`(go语言), 直接赋值后进行移位拆分. 但是会出现两个问题: 1.似乎会有大端序小端序问题, 2.如果后续升级, 扩充结构体, 可能会出现问题. 建议如果后续扩充该结构体, 则尝试采用bytes转int

2023-7-12 16:47:47 *Development* 完成初赛内容, 并通过基础testbench

2023-7-12 22:30:25 *Development* 将初赛部分的测试用例加入项目中

2023-7-15 14:34:50 *Development* 实现了决赛部分的vfs拓展和`dir_map`优化部分, 重构了解析policy的函数逻辑

2023-7-15 16:07:44 *TODO* 完成了`src/bpfMap.go:271 checkOp TODO: 这里的代码和上面C部分的代码通过go generate生成`

2023-7-15 21:09:23 *Development* 将C语言的头文件和字符串选择代码使用go template自动生成`c_headers.go`

2023-7-18 13:14:28 *ProblemSolving* 解决了kill子进程, 使用`-pid`显示`no such process`的问题, 解决方法: 在创建进程`Cmd`结构体时加入`SysProcAttr: &syscall.SysProcAttr{Setpgid: true},`, 设置子进程的进程组id为其pid

2023-7-18 19:46:31 *ProblemSolving* 解决了reload时unmount报`device or resource busy`的错误. 错误原因: 子进程正在使用mount目录下的文件, 包括1.选择该文件夹作为工作目录 2.打开目录下的文件. 解决方法: 暂时搁置这个错误, 因为并没有更好的方法来清空 bpf map, 只能说如果子进程正在使用, 那么就无法reload.

2023-7-18 19:55:46 *Development* 开发了 reload policy 的功能, 但只是能用, 具体使用逻辑还需要沟通后再进行编写.

2023-7-18 20:12:12 *Rename* 将`myLog.go`与`utils.go`合并, 将`clone.go`与`nonInteractive.go`合并

2023-8-3 22:15:58 *TODO* 完成了`src/bpfMap.go:35 ReadAclAndSetMapValue TODO: 这里的model.txt和policy.txt采用硬编码`

2023-8-3 22:23:26 *Development* 将model和policy文件的路径加入参数中

2023-8-5 13:24:10 *Development* 将`cloneMain.go`和`childExec.go`合并, 便于编成一个elf文件

2023-8-5 13:25:06 *Development* 对项目代码内多处路径和参数逻辑进行调整, 形成可对外开放的接口

2023-8-5 15:13:17 *ProblemSolving* 研究了动态链接库相关问题. 在`bpf.go`中使用`#cgo LDFLAGS: -L /home/boes/.boesfs/acl/library -lboesfs -lpthread -lelf -lcap`这个flag, 可以使程序在编译时找到动态链接库, 如果故意将后面`-lpthread -lelf -lcap`去掉, 则编译时就会报```/home/boes/.boesfs/acl/library/libboesfs.so: undefined reference to `gelf_getshdr` ```, 但是程序运行时还需要寻找动态链接库. 不知道为什么前面能找到, 后面就找不到. 解决方法在链接[cgo调用c动态库 - 掘金](https://juejin.cn/post/7121629004817235998), 使用`dlopen`和`dlsym`来加载动态链接库的符号, 虽然感觉还是很蠢, 不过也是一种解决方法, 目前还没有实装.

2023年8月5日18:31:56 *Development* 添加了中断程序的功能
