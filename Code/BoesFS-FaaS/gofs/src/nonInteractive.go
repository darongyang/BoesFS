package gofs

import (
	"io"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"

	"go.uber.org/zap"
)

func NonInteract(interrupt_c chan int, arg_mountDir string, arg_userProg string, arg_killNum int, arg_reload bool, arg_model string, arg_policy string, arg_log string, args []string) {
	InitLogger("")
	logger := zap.L()
	// 接收父进程的通知, 中断运行的程序
	mutex := sync.Mutex{}
	var childPipe io.WriteCloser
	go func() {
		<-interrupt_c
		mutex.Lock()
		// 如果收到消息, 或者channel关闭, 则给子程序发送kill信号, 并中断运行
		if childPipe == nil {
			logger.Info("child is not running")
		} else {
			childPipe.Close()
			logger.Info("send kill signal to child")
		}
		// 不管怎样, 此程序都应该结束, 没有特别需要执行的defer
	}()

	if arg_mountDir == "" || arg_killNum == -1 || args == nil || len(args) == 0 {
		// Fatal会自动退出
		logger.Fatal("not enough args!")
	}
	logger.Info("start nonInteract mode")
	logger.Debug(
		"all args",
		zap.String("mountDir", arg_mountDir),
		zap.String("userProg", arg_userProg),
		zap.Int("killNum", arg_killNum),
		zap.Bool("reload", arg_reload),
		zap.Strings("args", args),
	)

	// 判断是user还是ACL模式, 获取bpf程序路径
	// var bpfProg string
	// if arg_userProg == "" {
	// 	logger.Info("ACL mode")
	// 	bpfProg = "~/.boesfs/acl/prog/acl_prog.o"
	// } else {
	// 	logger.Info("User mode")
	// 	bpfProg = arg_userProg
	// }

	// clone子程序
	cloneArgs := []string{"-d", arg_mountDir, "-k", strconv.Itoa(arg_killNum), "-m", arg_model, "-p", arg_policy, "-l", arg_log}
	if arg_reload {
		cloneArgs = append(cloneArgs, "-r")
	}
	// cmd, childPipe, _ := Clone("./child/cloneMain.go", ctx.CtrlFd, append(cloneArgs, args...))

	// 如果已经收到中断信号, 则不再执行
	if !mutex.TryLock() {
		return
	}
	var cmd *exec.Cmd
	cmd, childPipe, _ = Clone(ParseHome("~/.boesfs/boesfs1"), 1, append(cloneArgs, args...))
	mutex.Unlock()

	err := cmd.Start()
	if err != nil {
		logger.Error("exec.Cmd.Start", zap.Error(err))
	}
	defer cmd.Wait()

	// TODO: 只是单纯实现了reload功能, 交互使用逻辑有待完善
	// TODO: 这里没有实现C中的程序链表功能, 认为只有一个程序
	// 如果需要reload, 这里发送信号
	if arg_reload {
		time.Sleep(time.Duration(5) * time.Second)
		io.WriteString(childPipe, "reload\n")
	}
}

func Clone(newGoName string, fd int, args []string) (cmd *exec.Cmd, stdinPipe io.WriteCloser, err error) {
	logger := zap.L()

	// 执行clone调用的各种配置
	sysProcAttr := &syscall.SysProcAttr{
		Cloneflags: //syscall.CLONE_NEWUTS |
		// syscall.CLONE_NEWIPC |
		// 	syscall.CLONE_NEWPID |
		// syscall.CLONE_PARENT |
		// syscall.CLONE_NEWNET,
		syscall.CLONE_NEWNS |
			syscall.CLONE_NEWUSER,
		UidMappings: []syscall.SysProcIDMap{
			{
				ContainerID: 0,
				HostID:      os.Getuid(),
				Size:        1,
			},
		},
		GidMappings: []syscall.SysProcIDMap{
			{
				ContainerID: 0,
				HostID:      os.Getgid(),
				Size:        1,
			},
		},
		GidMappingsEnableSetgroups: false,
	}

	// OPTM go语言似乎没有单独实现类似fork功能的操作
	// 基本上都是fork+exec, 所以这里直接重启一个新的go程序
	// 作为新namespace中的客户端

	// 当前文件的工作目录
	workDir, err := os.Getwd()
	if err != nil {
		logger.Error("os.Getwd", zap.Error(err))
		return nil, nil, err
	}

	// 将文件描述符传递给子进程
	// 子进程中对应的文件描述符为3+i
	file := os.NewFile(uintptr(fd), "")
	if file == nil {
		logger.Fatal("nil file")
	}

	if strings.HasSuffix(newGoName, ".go") {
		// 使用go run运行目标程序

		// go可执行文件的路径
		goPath, err := exec.LookPath("go")
		if err != nil {
			logger.Error("exec.LookPath", zap.Error(err))
			return nil, nil, err
		}

		cmd = &exec.Cmd{
			Path: goPath,
			Args: append([]string{goPath, "run", newGoName}, args...),
			Dir:  workDir,
			Env:  os.Environ(),
			// Stdin:       os.Stdin, // 留着程序后续(可能)用来交互
			Stdout:      os.Stdout,
			Stderr:      os.Stderr,
			ExtraFiles:  []*os.File{file},
			SysProcAttr: sysProcAttr,
		}
	} else {
		// 直接运行编译后的目标程序
		cmd = &exec.Cmd{
			Path: newGoName,
			Args: append([]string{newGoName}, args...),
			Dir:  workDir,
			Env:  os.Environ(),
			// Stdin:       os.Stdin, // 留着程序后续(可能)用来交互
			Stdout:      os.Stdout,
			Stderr:      os.Stderr,
			ExtraFiles:  []*os.File{file},
			SysProcAttr: sysProcAttr,
		}
	}

	// 与子进程通讯的管道
	stdinPipe, err = cmd.StdinPipe()
	if err != nil {
		logger.Error("cmd.StdinPipe", zap.Error(err))
		return nil, nil, err
	}

	return cmd, stdinPipe, nil
}
