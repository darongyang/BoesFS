# proj201-ebpf-optimize

# 使用 eBPF 进行 serverless 场景下或 FUSE 文件系统的增强和性能优化

### （已有课题，是否可以更新一下去年的课题？）

原本的赛题：https://github.com/oscomp/proj147-eBPF-FUSE

### 项目名称

使用 eBPF 进行 serverless 或 FUSE 文件系统的增强和性能优化

### 项目描述

随着云计算和容器化技术的不断发展，serverless 和 FaaS (Function as a Service) 的概念越来越受到关注。它们提供了一种全新的开发方式，使得开发者可以不用考虑服务器资源的管理和调度，专注于应用程序的逻辑实现。

在 serverless 和 FaaS 模型下，函数运行环境是动态创建和销毁的，因此安全问题尤为重要。本项目旨在利用 eBPF 技术，提供一种轻量级的文件系统沙盒框架，使得开发者能够安全地在 serverless 和 FaaS 环境中运行非受信第三方代码。

Filesystem in User-SpacE(FUSE)是Linux内核的用户态文件系统接口，方便用户实现用户态文件系统并与Linux内核对接。FUSE接口目前有很大的性能瓶颈，由于FUSE在用户态和内核态之间的数据传输要做内存拷贝；之前有一些使用 eBPF 优化 FUSE 性能的实践，但只对元数据访问进行了优化，并没有涉及内存拷贝的部分。

参赛团队可以选择将 eBPF 与 FUSE 或 WrapFS 相结合，或者将 eBPF 结合其他类似的技术，实现文件系统沙盒的增强和性能优化；或者基于 eBPF 实现高性能的用户态文件系统功能，同时要求保证高性能，即在用户态和内核态之间传输数据时避免内存拷贝。

参考资料：

- https://sandfs.github.io
- “A Lightweight and Fine-grained File System Sandboxing ” Paper
- “when eBPF meets FUSE” in OSS NA’18, LPC’18

### 所属赛道

2023全国大学生操作系统比赛的“OS功能设计”赛道

### 参赛要求

- 以小组为单位参赛，最多三人一个小组，且小组成员是来自同一所高校的本科生（2023春季学期或之后本科毕业的大一~大四的学生）或研究生
- 如学生参加了多个项目，参赛学生选择一个自己参加的项目参与评奖
- 请遵循“2023全国大学生操作系统比赛”的章程和技术方案要求

### 项目导师

施继成（DatenLord）

- Email: [jicheng.shi@datenlord.com](mailto:jicheng.shi@datenlord.com)

郑昱笙（浙江大学/DatenLord）

- Email： [yusheng.zheng@datenlord.com](mailto:yusheng.zheng@datenlord.com)

### 难度

- 基础特性的难度：中等
- 高级特性的难度：较高

### 基本要求

- 使用 eBPF 技术实现 serverless 或 FUSE 文件系统沙盒
- 提供 fine-grained 的访问控制
- 允许动态 (programmatic) 的自定义安全检查
- 具备低性能开销

### 预期目标

- **提供安全的 serverless 和 FaaS 环境。** 本项目旨在利用 eBPF 技术提供一种轻量级的文件系统沙盒框架，使得开发者能够安全地在 serverless 和 FaaS 环境中运行非受信第三方代码。为了达到这一目标，本项目需要提供 fine-grained 的访问控制和动态 (programmatic) 的自定义安全检查。
- **优化 serverless 和 FaaS 环境的性能。** 由于 serverless 和 FaaS 环境下函数运行环境是动态创建和销毁的，因此文件系统沙盒的性能对于整个应用程序的性能影响非常大。参赛团队可以尝试使用 eBPF 技术对 serverless 和 FaaS 环境的文件系统进行优化，以提高应用程序的性能。