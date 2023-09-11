## 使用eBPF技术实现Linux内核FUSE模块并支持零拷贝

【项目难度】：进阶

【项目描述】：FUSE作为Linux提供的可以实现用户态文件系统的模块，与内核文件系统相比在提供灵活性的同时它也有两个劣势，多一次的内存拷贝和多一次的上下文切换导致性能问题。eBPF作为Linux的新技术提供了kernel space和user space共享数据的方式。本项目尝试将eBPF技术应用到FUSE模块中从而实现零拷贝。

【产出标准】：
1、实现FUSE 一次读/写请求的零拷贝

【技术要求】：
1、熟悉eBPF；
2、熟悉FUSE等相关Linux内核模块；
3、熟悉C语言

【导师姓名/导师邮箱】：周鹤洋 [heyang.zhou@datenlord.com](mailto:heyang.zhou@datenlord.com)
【成果提交仓库】：[https://github.com/datenlord/ebpf-fuse](https://gitee.com/link?target=https%3A%2F%2Fgithub.com%2Fdatenlord%2Febpf-fuse)

【相关参考资料】：
1、[https://github.com/libfuse/libfuse](https://gitee.com/link?target=https%3A%2F%2Fgithub.com%2Flibfuse%2Flibfuse)
2、[https://elixir.bootlin.com/linux/latest/source/fs/fuse](https://gitee.com/link?target=https%3A%2F%2Felixir.bootlin.com%2Flinux%2Flatest%2Fsource%2Ffs%2Ffuse)
3、[https://ebpf.io/](https://gitee.com/link?target=https%3A%2F%2Febpf.io%2F)
