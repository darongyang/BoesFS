# Gofs: Boesfs的go语言版本agent

<!-- [Badges] -->

## 概述

BoesFS-Agent是BoesFS项目的用户态程序部分，由C/go编写而成，C语言部分最终编译为一个二进制可执行文件。而go部分着力于解决FaaS的高性能使用和网络通讯的需求，将C语言的agent改写为go语言，在提供了 C Agent 基本功能的同时，优化了很多处理逻辑，并提供了简单方便的对外接口，便于接入主流的go语言FaaS框架。

## 安装及使用方法

- 使用`env.md`的教程安装go语言, 您也可以访问go官网来安装更新版本的go语言
- 使用`git clone`下载仓库, 即可使用go语言进行试运行

fn版本使用方法:

`make build`获取boesfs和boesfs1, 前者是主控程序, 可以随意放置, 后者为子程序, 需要放在`~/.boesfs`目录下

`export LD_LIBRARY_PATH=~/.boesfs/acl/library`

`~/.boesfs/acl/library`放`libboesfs.so`, `~/.boesfs/acl/prog`放`acl_prog.o`

主控程序准备接入fn, 所以暂时会有两个程序

## 设计思路 这里思路十分混乱

主要的设计思路在 C Agent 方面已经阐述较为完善，这里主要阐述go语言agent相较于C方面的改进。

ebpf的核心程序是通过c编写，clang进行编译的

go语言在高并发方面相较于C有较大的优势，但是与Linux系统交互的部分，特别是与ebpf相关的操作，C语言大量依赖内核的各种库，各类与系统调用相关的结构体都在C的头文件中，其本身编译已经较为困难，使用go语言更是有一些劣势。我们这里选择使用`github.com/cilium/ebpf`库来与ebpf部分交互，可以实现很好的交互效果。但是该库在Linux较低版本的支持并不是很好，故本项目在这个库的基础上，额外引入了C部分的代码作为动态链接库，使程序在较低版本的Linux版本和非发行版上依旧可以顺利运行。

在数据处理等方面，本项目也充分发挥go语言相对于C语言的优势，引入一些系统自带的，或者一些第三方库来更加简洁、高效地完成各项任务。例如在字符串处理方面使用正则表达式，可以更加通用地匹配到字符串。

日志：
本项目也引入了zap库，用来记录程序的日志，并且可以输出到日志文件，可以更好地记录程序的运行情况，对可能发生的错误有着更强大的复盘及恢复能力。

添加了主动中断程序执行的功能

## 各模块介绍

### 概述
本项目的模块主要分为四部分：

1. 项目主函数：`main.go`。提供了项目运行的示例代码，解析命令行参数并调用项目主体函数。可以编译成可执行文件，实现与C语言部分基本一致的效果，同时也将项目调用的代码和主体代码分离，使得本项目既可以独立运行，完成目标任务，也使得主体代码可以作为模块被其他项目调用，增强了项目的灵活性。
2. 项目主体：`gofs`包。包含了在物理机内需要执行的各种函数，该包提供了沙盒创建、ACL规则读取、bpf加载与交互，以及日志记录等一系列工具集。其中NonInteract函数作为对外的接口，其他包可以调用这个函数来创建沙盒环境并运行目标程序。
3. 沙盒管理进程：`cloneMain.go`。作为新namespace的第一个进程，管理着其所在的namespace，并运行一个新的监控进程，让监控进程重置父进程的各种特权，并运行用户指定的目标函数。同时与项目主体代码保持沟通，接收外部传来的各种命令。
4. 代码转换器：`generator`文件夹。将Boesfs项目C部分的各种头文件中所需要的结构体、宏定义、枚举类等转化为go语言，并提供一些字符串到枚举类的转换函数，大大提高了项目的可维护性。

### main.go

main.go使本模块可以被独立编译成一个可执行文件，实现与C Agent基本一样的运行效果。main函数沿用C的逻辑，根据参数数量选择接下来要执行的模式，没有参数则进入交互模式，有参数则进入非交互模式。而我们这里深入分析了go Agent的应用场景，认为并不需要实现交互模式，故精简了代码逻辑。main函数负责解析各项命令行参数，并将这些参数传递给NonInteract函数，实现项目功能。

### nonInteractive.go

#### NonInteract函数

NonInteract函数为项目主体逻辑的入口函数。将解析过的各个参数传递进来即可运行项目。程序首先分析传入参数的合法性，将各项参数输出到日志，未设置必选参数的直接报错。随后直接调用`LoadProgC`函数加载bpf字节码，ACL模式调用`ReadAclAndSetMapValue`函数读取model和policy文件，User模式则根据用户自定义操作进行设置。最后调用Clone函数，在新的namespace中启动子进程，给子进程传入一系列所需的参数，构造沙盒环境，运行目标程序。

以下为NonInteract函数的各项参数说明

```go
// @param	interrupt_c		chan int	接收父进程的通知, 传入任何数据或直接关闭管道均可直接中断运行的程序
// @param	arg_mountDir	string		挂载目录
// @param	arg_userProg	string		用户程序路径
// @param	arg_killNum		int			阈值
// @param	arg_reload		bool		是否reload
// @param	arg_model		string		模型文件路径
// @param	arg_policy		string		策略文件路径
// @param	arg_log			string		日志文件路径
// @param	args			[]string	目标程序参数
```

此外，我们还深度调研了Fn的使用场景，发现用户的目标程序可能会是一个网络服务器，一直在监听连接处阻塞，其实内核部分也有直接终止函数执行的逻辑，但是需要程序触发规则，而在这种场景下，可能是用户主动要求中断程序执行，故这里使用go语言的管道，实现了一个中断程序执行的功能，传入一个`interrupt_c`管道，用来通知程序直接中断运行，程序会关闭其与新namespace内主控程序的进程间管道，通知子进程尽快结束执行。

这里使用了goroutine来处理管道传递的消息，很好地发挥了goroutine轻量级线程的优势，使主程序不需要阻塞来接收通知，并且消耗很少的系统资源。

```go
	go func() {
		for {
			_, ok := <-interrupt_c
			// 如果收到消息, 或者channel关闭, 则给子程序发送kill信号, 并中断运行
			if ok {
				logger.Info("receive interrupt signal")
				if childPipe == nil {
					logger.Info("child is not running")
				} else {
					childPipe.Close()
					logger.Info("send kill signal to child")
				}
			} else {
				logger.Info("interrupt channel closed")
				childPipe.Close()
				logger.Info("send kill signal to child")
			}
			// 不管怎样, 此程序都应该结束, 没有特别需要执行的defer
			os.Exit(0)
		}
	}()
```

#### Clone函数

Clone函数的主要功能为创建新namespace，启动沙盒管理者程序，传入参数和返回值在下文阐述。该函数的核心功能为构造exec.Cmd结构体，用于启动新namespace及子程序。为了实现构造新namespace的功能，这里需要构造`syscall.SysProcAttr`结构体，设置clone的各项flags，并且设置新namespace中的uid、gid映射，这里我们采用和C一样的操作，将当前用户映射为新namespace中的root用户，虽然名义上是root，但是实际运行时与当前用户的权限是一致的。

```go
// @param	newGoName	string		新namespace管理程序的路径, 可直接传入go源码路径或可执行程序路径
// @param	fd			int			ebpf program的文件描述符, 子程序用于传递给内核
// @param	args		[]string	运行用户程序的参数
// @return	cmd			*exec.Cmd	启动新进程的结构体
// @return 	stdinPipe	io.WriteCloser	新进程的stdin管道, 用来传递消息
```

此外，此函数还对传入的子程序名称进行了处理，利用文件后缀名称判断传入的是go源码路径还是可执行文件的路径。如果传入的是go源码路径，则程序会查找go可执行文件的路径，并用`go run`命令直接运行go源码，如果传入的是可执行文件路径，则直接运行可执行文件。

```go
	if strings.HasSuffix(newGoName, ".go") {
		// 使用go run运行目标程序

		// go可执行文件的路径
		goPath, err := exec.LookPath("go")
		if err != nil {
			...
		}

		cmd = &exec.Cmd{
			Path: goPath,
			Args: append([]string{goPath, "run", newGoName}, args...),
			...
		}
	} else {
		// 直接运行编译后的目标程序
		cmd = &exec.Cmd{
			Path: newGoName,
			Args: append([]string{newGoName}, args...),
			...
		}
	}
```

### bpf.go

主要负责与ebpf文件的交互部分，以及对调用动态链接库的代码的操作进行封装。

#### ebpf_context结构体

```go
type ebpf_context struct {
	CtrlFd int
	maps   [max_entries]*ebpf.Map
}

func (e ebpf_context) Close() (err error)
```

效仿C部分的ebpf_context结构体，原本只是存储文件描述符，而这里对结构体进行了优化，使用 cilium ebpf 第三方库的`ebpf.Map`结构体对文件描述符进行包装，使得与 ebpf map 交互的过程更加简单便捷。并且针对这个结构体定义了Close方法，在函数中循环关闭相关的文件描述符，配合go的defer关键字，即可更加简单有效地释放系统资源，

#### 加载bpf字节码相关函数

```go
func LoadProgGo(filePath string) (*ebpf_context, error)
func LoadProgC(filePath string) (*ebpf_context, error)
```

提供了使用C语言和使用go语言两种不同的bpf字节码加载方式，均为传入bpf字节码路径，返回`ebpf_context`结构体指针。

`LoadProgGo`函数使用 cilium ebpf 库加载ebpf文件，使用`ebpf.LoadCollectionSpec`和`spec.LoadAndAssign`两个函数即可将bpf文件加载到内核中，缺点是对低版本和非发行版的Linux内核支持不完善。

`LoadProgC`函数调用动态链接库加载ebpf文件，C语言的代码与内核提供的库一起编译，可以更好地适配当前Linux系统，具有更佳的通用性，在内核加载完毕之后，将获取到的C语言ebpf_context结构体转化为go语言的结构体，方便后续调用map交互函数。

### bpfMap.go

主要负责与ebpf map交互的部分，其中也包含了解析ACL规则的部分。其核心函数为ReadAclAndSetMapValue函数，负责读取ACL规则文件，解析其中的规则，将规则写入ebpf map中。

#### ReadAclAndSetMapValue函数

负责读取ACL规则文件，包括model和policy文件，解析其中的规则，并将规则写入ebpf map中。由于此段代码较为复杂，故在从C迁移到go时，对代码进行了一定程度的重构，将一些逻辑拆分成为函数，提高了代码的可读性与可维护性。代码的主体逻辑仍然和C保持一致，首先读取model文件，根据model文件中的规则，设置ebpf map中与模型基本配置相关的部分。随后读取policy文件中的每一个规则，根据model的类型，选用不同的处理逻辑进行处理，并将规则写入对应的ebpf map中。

与C部分有很大不同的是，这里的字符串解析逻辑大量使用了正则表达式，不仅使得代码更加简洁高效，还提高了程序的通用性，可以适应更多的字符串格式，使用户可以更加自由地编写model与policy文件。此外，根据现有的使用场景，规则文件并不会很大，故这里在函数开始时直接将文件内容全部一次性读入内存，使用正则表达式进行解析，而不是像C部分一样，一行一行地读取文件，这样可以减少系统调用的次数，提高程序的运行效率。

#### C相关结构体的处理

在C部分中，agent与ebpf约定了很多结构体，用来在ebpf map中装载数据，其定义在几个相关的`.h`头文件中，由于cgo会将C语言中的类型转化为cgo模块中的类型，故如果使用cgo直接识别头文件中的结构体，会造成相似的变量类型在go与C之间反复转换，并且cilium ebpf库较难识别这样的结构体，从ebpf map中读取之后较难进行识别，导致不必要的麻烦。故这里我们将两个重要的结构体`policy_t`和`dir_entry_t`改写到go语言中，并且为其创造了对应的unmarshal函数，从ebpf map中读取数据时直接使用字节切片存放数据，之后在unmarshal函数中使用`unsafe.Pointer`将其转化到对应的结构体中，使得程序可以更好地读取这些结构体。

```go
// 从acl_public.h中复制, 如果更改, 则需要同步更改下面的unmarshal函数
type policy_t struct {
	valid uint32
	allow uint32
	deny  uint32
}

const PATH_LEN int = 256

type dir_entry_t struct {
	path [PATH_LEN]byte
	rule policy_t
}

func unmarshal_polict_t(data []byte) (policy_t, error) {
	if len(data) != 12 {
		return policy_t{}, fmt.Errorf("invalid policy_t length")
	}
	return policy_t{
		valid: *(*uint32)(unsafe.Pointer(&data[0])),
		allow: *(*uint32)(unsafe.Pointer(&data[4])),
		deny:  *(*uint32)(unsafe.Pointer(&data[8])),
	}, nil
}

func unmarshal_dir_entry_t(data []byte) (dir_entry_t, error) {
	if len(data) != PATH_LEN+12 {
		return dir_entry_t{}, fmt.Errorf("invalid dir_entry_t length")
	}
	var path [PATH_LEN]byte
	copy(path[:], data[:PATH_LEN])
	rule, err := unmarshal_polict_t(data[PATH_LEN:])
	if err != nil {
		return dir_entry_t{}, fmt.Errorf("unmarshal dir_entry_t failed: %v", err)
	}
	return dir_entry_t{
		path: path,
		rule: rule,
	}, nil
}
```

### 代码转换器与c_headers.go

代码转换器位于`generator`文件夹内，核心代码为`generate.go`，其主要功能为将C部分的头文件中的结构体、宏定义、枚举类等转化为go语言，并提供一些字符串到枚举类的转换函数，生成的代码放到了`c_headers.go`内，便于项目主体代码调用。如果boesfs项目主体添加了新的功能，用到了新的结构体或对原有的结构体、宏定义进行了修改，只需要重新运行`go generate`，便可以很快进行对接，大大提高了项目的可维护性和可拓展性。

`generate.go`主要使用正则表达式和go语言内置的`text/template`代码模板包来完成代码生成工作，未来也可能使用c2go活stringer这类工具来完成，目前的代码量较少，用这些也可以较好地完成任务。

#### generateCheckOp函数

主要负责生成`checkOp`函数，该函数用于判断用户在policy文件规则中给定的vfs操作是否是支持的操作类型，并将操作类型字符串转化为对应的操作码。这些操作码在C部分的头文件中以宏定义的形式定义，有着比较明显的规则，即`#define BOESFS_操作类型 对应操作码`，故这里使用正则表达式匹配这些宏定义，将操作类型和操作码部分提取出来，将前者的字母全部改为小写之后，放入Switch的case部分，对应的操作码在对应case中作为返回值返回，利用template包即可快速生成代码

```go
	type caseStruct struct {
		Name  string // case的选项, 即操作类型字符串
		Value string // case内函数的返回值, 即对应的操作码
	}
	var caseList []caseStruct
	for _, match := range matches {
		name := match[1]
		value := match[2]
		// constMap["BOESFS_"+name] = value
		caseList = append(caseList, caseStruct{Name: strings.ToLower(name), Value: value})
	}
	const templateCode = `
// 反正我自己也不用宏定义, 所以这里直接将宏定义展开写死
// 要是改了 直接重新 go generate
func checkOp(s string) int {
	switch s {
{{- range .}}
	case "{{.Name}}":
		return {{.Value}}
{{- end}}
	default:
		return -1
	}
}
`
	// 解析模板
	tmpl := template.Must(template.New("opCode").Parse(templateCode))

	// 将数值插入模板中，并添加到文件
	if err != nil {
		log.Fatal(err)
	}
```

#### generateEnums函数

主要负责生成各类枚举类。由于go语言中没有直接的枚举类，故这里选用const+iota的方式来实现C语言中枚举类的功能。使用正则表达式匹配C语言中的`typedef enum`即可将枚举类的名称和内容提取出来，内容部分也不需要做过多的处理，只需要将第一个逗号改为`= iota`，其余的逗号直接移除即可。这里由于有较多的枚举类，故需要一个for循环。

```go
	// 定义模板
	const templateCode = `
// {{.Name}}
const (
	{{- .Value -}}
)
`
	...

	type enumStruct struct {
		Name  string
		Value string
	}
	// 依次按照模板生成到文件中
	for _, match := range matches {
		enumName := match[1]
		content := match[2]
		content = strings.Replace(content, ",", " = iota", 1)
		content = strings.ReplaceAll(content, ",", "")
		err = tmpl.Execute(file, enumStruct{Name: "enum " + enumName, Value: content})
		if err != nil {
			log.Fatal(err)
		}
	}
```

### utils.go

一些简单的工具函数，逻辑并不复杂，这里简单介绍。

OpenAndReadAllContent函数：打开文件，读取全部内容并返回一个字符串。

InitLogger函数：初始化日志记录器，传入logPath参数，可以实现将日志同时输出到指定的文件和命令行中，如果该参数为空则只输出到命令行。并且，这个函数不需要将日志管理器返回，此函数设置了全局的日志管理器，随后的每个函数中，只需要调用`zap.L()`，即可获取到全局的日志管理器，进行日志记录，这样可以有效减少各函数的参数数量（不用每次都传logger），大大方便了各模块间的参数传递。在开发过程中，日志记录器的模式设置为DEBUG模式，可以更多地打印程序运行的详细信息，方便调试。而在发行版中，可以将日志记录器的模式设置为INFO模式，只记录程序运行的关键信息，减少日志文件的大小，同时源码中的各项DEBUG输出代码也无需删除，很大程度上方便了程序员的编写与代码的发行。

ParseHome函数：为家目录解析提供支持。代表当前用户家目录的`~`符号通常只会由命令行解释器解释，而编程语言对此却很少有支持。这里提供这个函数，用来解析路径中的`~`符号，将其替换为用户的家目录，方便用户使用。

### cloneMain.go

