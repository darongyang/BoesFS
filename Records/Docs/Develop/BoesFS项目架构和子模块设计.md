# 项目架构和设计

## **项目设计理念**

BoesFS，全称为Based on eBPF Sandboxing File System。BoesFS是一个将eBPF技术和内核文件系统相融合的高性能、最实用的沙盒文件系统。

在设计思想上，BoesFS项目改进和创新了sandfs的设计思想。BoesFS项目的设计理念是：

- 最完善的访问控制接口
- 最灵活的自定义访问控制规则
- 最友好的用户交互
- 最小幅度的内核源码修改
- 最高的性能
- 最可靠的系统
- 最实用的沙盒文件系统
- 最丰富的应用场景

## **项目整体架构图**

**BoesFS架构(Flexible)**

![项目整体架构图](../images/boesfs架构设计/BoesFS架构(Flexible).png)

## **项目架构介绍**

下面如果不做特殊说明，我们默认所提的BoesFS项目均为BoesFS(Flexible)。BoesFS(Flexible)和BoesFS(Normal)没有本质的区别，只是定义eBPF Helper的位置发生了改变，这个我们会在最后做更详细介绍。

BoesFS项目由四个部分组成（上图蓝色区域所示）：

- **BoesFS Agent**

BoesFS位于用户态的部分。该部分能够在用户态实现BoesFS项目功能的导出，使得能够根据用户的需求，形成BoesFS沙盒环境，负责加载字节码，加载用户安全检查ACL规则，此外需要实现进程管理、用户交互等诸多功能。BoesFS Agent设想通过Golang实现。

如架构图所示，BoesFS运行时，该部分要和BoesFS内核部分、BoesFS检查字节码部分、不可信应用程序等多个部分进行交互。

- **BoesFS in Kernel**

BoesFS位于内核的部分。该部分能够在内核为BoesFS项目提供诸如参数采集和传递、审计日志、非法请求拦截等功能。该部分会以增强型的拓展文件系统的形式存在，拓展性体现在结合eBPF技术，并实现上述功能。

如架构图所示，运行时该部分会以一个文件系统的形式存在于内核中，沙盒环境下的所有VFS请求都会通过该文件系统。其采集到的参数会传递给字节码检查程序。

- **BoesFS Check Module**

BoesFS用于检查的部分。该部分负责BoesFS项目中安全检查并决定对传入的请求是deny还是allow。该部分首先要负责导出BoesFS内核部分支持的API，可供用户基于这些API进行规则指定。但基于用户友好的设计理念，该部分会封装ACL模型，用户无需书写字节码也能实现访问控制。

如架构图所示，运行时该部分会BoesFS客户端动态的加载到内核当中。

- **Few Motify in Kernel**

这个部分是为了能够实现BoesFS项目所需的功能，对原生Linux内核所做的必要的修改。这些功能包括：eBPF技术融合、让非特权用户也能使用BoesFS。

其中前三个是BoesFS项目的核心部分。当然BoesFS项目在运行时还会动态的包括位于内核中的eBPF虚拟机、eBPF Maps等部分。

## 子模块设计

### BoesFS in Kernel

BoesFS in Kernel是一个运行于内核态、可动态插入内核、注册为文件系统的内核模块。它由三个子部分组成：①第一部分是BoesFS，一个基于Wrapfs开发的、实现文件请求参数采集的内核文件系统；②第二部分是eBPF Helper，一个实现BoesFS项目对eBPF的支持而添加的工具函数集。③第三部分是LKM Supporter，一个基于kprobe和kallsyms，实现BoesFS模块化和eBPF Hepler模块化需求的支持函数库。

BoesFS项目力求将更多的主动权交给用户自己去做自定义实现，BoesFS in Kernel将所有用户可能用于安全检查的VFS API进行了hook和参数采集，以追求最完善的访问控制接口的设计理念。所有的支持的VFS API我们都提供了具体采集的参数列表、导出的原因和潜在的利用场景。

BoesFS in Kernel以内核模块的形式存在，以支持动态插入内核，体现BoesFS项目对内核源码的最小幅度修改的理念。当需要使用BoesFS项目提供沙盒环境，在实际运行时，这个部分会被插入（insmod）到虚拟文件系统（VFS）和基本文件系统（BFS）之间，对VFS传来的所有文件请求进行参数的采集，并将参数传递给Check Module。当不需要BoesFS沙盒环境时，它又可以从内核被动态的卸载，以还原接近原生(native)的内核环境。这使得BoesFS项目的使用变得特别灵活，这为BoesFS项目用以实际应用提供了更大的可能。这从某种程度上提现我们最高的性能、最可靠的系统、最实用的沙盒文件系统、最丰富应用场景的理念。同时模块化的方式也为BoesFS项目的开发和维护带来了很大的便利，避免了每次修改都会带来的繁琐内核编译，降低了上手和维护成本。

### BoesFS Check Module

BoesFS Check Module是一个在用户态编译、最终被加载到内核运行的eBPF字节码程序。它由两个子部分组成：①一部分是Native API，它导出了BoesFS in Kernel所能支持和采集的VFS文件请求API供用户使用。用户可以利用Native API的接口，借助eBPF知识和eBPF Maps，任意设计检查规则和编写eBPF字节码程序，具有非常大的自定义性。②一部分是ACL/RBAC控制程序，它是基于Native API实现的访问控制模型处理程序。Native API带来很大自定义性的同时，我们也不能否认它的上手成本（例如需要扎实的eBPF功底和对很好的eBPF Maps的设计等）和可能带来很大的修改幅度（你可能需要同时修改字节码和我们的Golang App）。我们无法保证每个使用者都具备类似的能力。而且往往很多场景下，我们只希望简单的配置一下访问控制，这种方法大材小用，过于冗余复杂。因此考虑到以上诸多因素，我们希望提供访问控制模型的方式，来允许用户通过简单的定义访问控制模型，就能达到自定义访问控制的目的。ACL/RBAC控制程序是运行时位于内核中的eBPF字节码程序，它将依据用户提供的、存放在eBPF Maps里的ACL/RBAC Rules和ACL/RBAC Definition来对BoesFS in Kernel传来的参数，进行访问控制模型的检查和判断，返回deny或allow。BoesFS Check Module体现了BoesFS项目最灵活的自定义访问控制规则和最友好的用户交互的设计理念。

### BoesFS Agent

BoesFS Agent是一个运行于用户态的BoesFS的客户端。它是/拟采用Golang语言进行编写的（前期可能会基于C实现，后期再做重构），具有很好用户交互性和很高的性能优势。BoesFS Agent作为BoesFS项目在用户态的控制端，承担了很多任务，它会负责：和用户进行一定程度的交互、以沙盒方式运行不可信的应用程序（Untrusted Applications）、加载安全检查规则BoesFS Check Module（eBPF字节码）到内核、获取访问控制策略模型、和位于内核的eBPF字节码通过eBPF Maps进行交互、输出运行日志信息（设想中）等（也可以补充BoesFS实现更多的功能）。BoesFS Agent体现了我们最友好的用户交互的设计理念。

### Few Motify in Kernel

Few Motify in Kernel是为了让Linux 5.10的内核支持BoesFS项目而进行的一些修改。很遗憾，根据目前内核的eBPF的设计实现，这些修改是不可避免的（当然并不多，只有几行）。主要是修改了include/uapi/linux/bpf.h和kernel/bpf/syscall.c两个文件（共计5行）。前者的修改主要是为了注册BoesFS字节码类型和添加eBPF工具函数的定义，后者的修改主要是为了实现非特权用户对BoesFS项目的使用。

## **BoesFS的Flexible和Normal**

| BoesFS架构类型   | 适用的Linx内核版本 |
| ---------------- | ------------------ |
| BoesFS(Flexible) | Linux 4.11及以下   |
| BoesFS(Normal)   | Linux 任意内核版本 |

首先，下面是BoesFS(Normal)架构图。

**BoesFS架构(Normal)**

![项目整体架构图](../images/boesfs架构设计/BoesFS架构(Normal).png)

可以直观的看到，eBPF Helper模块被移动到了内核。这是无奈之举，BoesFS项目是基于Linux的项目，和其他类似项目一样，都必须根据Linux内核的变化而做出改变。Linux内核4.12开始的任何版本之后，移除了bpf_prog_type_list结构，不再支持eBPF prog类型的动态注册。起初的原因只是因为来自intel的开发者Johannes Berg认为动态注册的场景用不到。这很令人遗憾。以下链接是更多更多相关信息：

> There's no need to have struct bpf_prog_type_list since it just contains a list head, the type, and the ops pointer. Since the types are densely packed and not actually dynamically registered, it's much easier andsmaller to have an array of type->ops pointer. (From: https://www.mail-archive.com/netdev@vger.kernel.org/msg162502.html)
>
> All BPF helper functions are part of the core kernel and cannot be extended or added through kernel modules.(From: [BPF and XDP Reference Guide — Cilium 1.6.12 documentation](https://docs.cilium.io/en/v1.6/bpf/))

这意味着，在Linux内核4.12及以后的版本，BoesFS项目无法再通过内核模块的方式动态插入eBPF Helper，这意味着，eBPF Helper必须在编译内核的时候被写死。这和BoesFS项目的一个设计理念产生了一定冲突--最小内核修改量，同时这也为BoesFS项目的开发和维护带来一定困难。但实际上，其实也无伤大雅。BoesFS(Flexible)到BoesFS(Normal)只有eBPF Helper的位置发生了改变，能够很轻松的完成BoesFS(Flexible)到BoesFS(Normal)的移植工作。
