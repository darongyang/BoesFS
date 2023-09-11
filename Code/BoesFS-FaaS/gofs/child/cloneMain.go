package main

import (
	"flag"
	"fmt"
	gofs "gofs/src"
	"io"
	"os"
	"os/exec"
	"strings"
	"sync"
	"syscall"

	"go.uber.org/zap"
)

func main() {
	// 工作目录为整个项目的目录
	// 初始化新namespace中的logger
	// gofs.InitLogger("./log/child.txt")
	gofs.InitLogger("")
	logger := zap.L()
	wd, _ := os.Getwd()
	logger.Debug("workDir", zap.String("path", wd))

	// 解析参数
	arg_child := flag.Bool("c", false, "child")
	arg_mountDir := flag.String("d", "", "mount directory")
	// fd := flag.Int("f", -1, "ebpf_context.ctrl_fd") // always 3
	arg_killNum := flag.Int("k", -1, "threshold to kill the program")
	arg_reload := flag.Bool("r", false, "reload")
	arg_model := flag.String("m", "./src/modules/model.txt", "*reload* model file path")
	arg_policy := flag.String("p", "./src/modules/policy.txt", "*reload* policy file path")
	arg_log := flag.String("l", "", "log file path")
	flag.Parse()
	args := flag.Args()

	if *arg_child {
		cloneChild()
	} else {
		cloneMain(*arg_mountDir, *arg_killNum, *arg_reload, *arg_model, *arg_policy, *arg_log, args)
	}
}

// 新namespace的主控程序
func cloneMain(arg_mountDir string, arg_killNum int, arg_reload bool, arg_model string, arg_policy string, arg_log string, args []string) {
	logger := zap.L()
	if arg_mountDir == "" || arg_killNum == -1 || flag.NArg() == 0 {
		logger.Fatal("not enough args!", zap.Int("NArg", flag.NArg()))
	}
	logger.Debug(
		"child args",
		zap.String("mountDir", arg_mountDir),
		zap.Int("killNum", arg_killNum),
		zap.Bool("reload", arg_reload),
		zap.Strings("args", args),
	)

	var cmd *exec.Cmd
	var reloadData string
	mutex := sync.Mutex{}
	// 接收父进程的信号
	go func() {
		for {
			// 通过stdin输入
			var buffer [16]byte
			cnt, err := os.Stdin.Read(buffer[:])
			mutex.Lock()
			if err != nil || cnt == 0 {
				if err != io.EOF {
					println("read failed")
				} else if cmd != nil {
					// 管道关闭, 直接杀死子进程, 后续的wait和unmount由主逻辑完成
					err := syscall.Kill(-cmd.Process.Pid, syscall.SIGKILL)
					if err != nil {
						logger.Fatal("syscall.Kill SIGKILL", zap.Error(err))
					}
					logger.Info("kill child process")
				}
				logger.Info("stdin closed")
				break
			}

			// 解析收到的消息
			msg := string(buffer[:])
			logger.Info("receive msg", zap.String("msg", msg))
			if cmd == nil {
				logger.Error("user process is not running")
			} else if arg_reload && reloadData != "" {
				// 父进程通知可以开始reload
				reload(cmd.Process.Pid, arg_mountDir, args[0], reloadData, arg_model, "/home/boes/gofs/testCases/testData/policy_reload.txt")
			}
		}
	}()

	// 加载bpf程序并设置map
	ctx, _ := gofs.LoadProgC(gofs.ParseHome("~/.boesfs/acl/prog/acl_prog.o"))
	gofs.ReadAclAndSetMapValue(ctx, args[0], arg_model, arg_policy)
	// defer ctx.Close()

	// 将该用户的根目录挂载点转换为从站挂载点, 即
	// 其他用户出现的挂载或卸载事件会传播到该用户
	// 而该用户出现的挂载或卸载事件不会传播到其他用户
	// 单纯的 mount namespace 并不能做到隔离作用
	// 可以参考 https://zhuanlan.zhihu.com/p/166393945
	err := syscall.Mount("none", "/", "", syscall.MS_REC|syscall.MS_SLAVE, "")
	if err != nil {
		logger.Fatal("mount slave failed", zap.Error(err))
	}

	// 运行目标程序
	if !mutex.TryLock() {
		// 如果已经锁住, 就可以直接退了
		logger.Info("child process canceled")
		return
	}
	// newGoName := "./child/childExec/execWaiter.go"
	// newGoName := "/home/boes/.boesfs/boesfs1"
	newGoName, _ := os.Executable()
	// 当前文件的工作目录
	workDir, err := os.Getwd()
	if err != nil {
		logger.Fatal("os.Getwd", zap.Error(err))
	}
	if strings.HasSuffix(newGoName, ".go") {
		// go可执行文件的路径
		goPath, err := exec.LookPath("go")
		if err != nil {
			logger.Fatal("exec.LookPath", zap.Error(err))
		}
		// 使用go run运行目标程序
		cmd = &exec.Cmd{
			Path:        goPath,
			Args:        append([]string{goPath, "run", newGoName}, args...),
			Dir:         workDir,
			Env:         os.Environ(),
			Stdout:      os.Stdout,
			Stderr:      os.Stderr,
			SysProcAttr: &syscall.SysProcAttr{Setpgid: true},
		}
	} else {
		cmd = &exec.Cmd{
			Path:        newGoName,
			Args:        append([]string{newGoName, "-c"}, args...),
			Dir:         workDir,
			Env:         os.Environ(),
			Stdout:      os.Stdout,
			Stderr:      os.Stderr,
			SysProcAttr: &syscall.SysProcAttr{Setpgid: true},
		}
	}

	w, err := cmd.StdinPipe()
	if err != nil {
		logger.Fatal("cmd.StdinPipe", zap.Error(err))
	}

	err = cmd.Start()
	if err != nil {
		logger.Fatal("cmd.Start", zap.Error(err))
	}
	mutex.Unlock() // 从此刻开始, cmd的pid才有了意义

	// 挂载boesfs文件系统
	// logpath := path.Join(workDir, "log/kernel.txt")
	// 向boesfs内核传递需要的数据
	// 这里的fd恒为3, 详见exec.Cmd.ExtraFiles处注释(entry i becomes file descriptor 3+i)
	isLog := 1
	if arg_log == "" {
		isLog = 0
	}
	data := fmt.Sprintf("fd=%d,pid=%d,kill=%d,log=%d,logpath=%s,elfpath=%s", ctx.CtrlFd, cmd.Process.Pid, arg_killNum, isLog, arg_log, args[0])
	reloadData = fmt.Sprintf("kill=%d,log=%d,logpath=%s,elfpath=%s", arg_killNum, isLog, arg_log, args[0])
	logger.Info(data)
	err = syscall.Mount(arg_mountDir, arg_mountDir, "boesfs", syscall.MS_NOSUID|syscall.MS_NODEV, data)
	if err != nil {
		logger.Fatal("mount boesfs failed", zap.Error(err))
	}
	logger.Info("mount boesfs success")
	ctx.Close()

	// 向子进程stdin管道写入数据, 解除子进程阻塞
	io.WriteString(w, "start\n")
	w.Close()
	logger.Info("write start to child process")

	err = cmd.Wait()
	if err != nil {
		logger.Warn("cmd.Wait()", zap.Error(err))
	}

	// TODO: 如果前面出现问题, 跳转到这个地方, 防止内核模块引用过多
	err = syscall.Unmount(arg_mountDir, 0) // 这样能更直观地看到卸载的效果
	if err != nil {
		logger.Error("syscall.Unmount", zap.Error(err))
		logger.Info("lazy unmount", zap.Error(syscall.Unmount(arg_mountDir, syscall.MNT_DETACH)))
	}
	logger.Info("cloneMain.go exited")
	os.Exit(0)
}

// reload policy
func reload(pid int, mountDir string, argv0 string, reloadData string, model_path string, policy_path string) {
	logger := zap.L()
	logger.Info("reload start")

	// 使用SIGSTOP停止执行二进制文件的进程(组)
	// 实际上是execWaiter.go及其同进程组的进程
	logger.Info("stop process", zap.Int("pid", pid))
	err := syscall.Kill(-pid, syscall.SIGSTOP)
	if err != nil {
		logger.Fatal("syscall.Kill SIGSTOP", zap.Error(err))
	}

	// 卸载boesfs文件系统
	// BUGS: 子进程不能将mount目录作为工作目录, 不能在此时打开mount目录下的文件, 否则会卸载失败
	err = syscall.Unmount(mountDir, 0)
	if err != nil {
		cmd := exec.Command("lsof", mountDir)
		out, err1 := cmd.Output()
		if err1 != nil {
			fmt.Printf("err in cmd.Output(): %s\n", err1.Error())
		}
		fmt.Println("lsof\n", string(out))
		logger.Fatal("syscall.Unmount", zap.Error(err))
	}

	// 重新加载bpf字节码, 并设置map
	// BUGS: 这里如果多次reload, 可能需要将ebpf_init函数的index进行增加
	// 那个index, 每当一个进程load一次bpf字节码, 就会加一
	// lqc那边因为是fork, 新进程继承了上一个进程的index, 所以会出现问题, 而我这里因为是exec, 故还可以继续沿用index=0
	ctx, err := gofs.LoadProgC("./src/modules/acl_prog.o")
	if err != nil {
		logger.Fatal("LoadProgC", zap.Error(err))
	}
	gofs.ReadAclAndSetMapValue(ctx, argv0, model_path, policy_path)

	// 重新挂载boesfs文件系统
	mountData := fmt.Sprintf("fd=%d,pid=%d,%s", ctx.CtrlFd, pid, reloadData)
	logger.Debug(mountData)
	err = syscall.Mount(mountDir, mountDir, "boesfs", syscall.MS_NOSUID|syscall.MS_NODEV, mountData)
	if err != nil {
		logger.Fatal("mount boesfs failed", zap.Error(err))
	}
	ctx.Close()

	// 使用SIGCONT恢复执行二进制文件的进程(组)
	err = syscall.Kill(-pid, syscall.SIGCONT)
	if err != nil {
		logger.Fatal("syscall.Kill SIGCONT", zap.Error(err))
	}

	logger.Info("reload success")
}

// 用来等待父进程通知, 并启动目标子进程
// 因为这个时候父进程需要挂载文件系统, 但又需要获取pid
func cloneChild() {
	logger := zap.L()
	// 等待父进程通知可以开始
	var buffer [16]byte
	cnt, err := os.Stdin.Read(buffer[:])
	if err != nil || cnt == 0 {
		println("read failed")
		os.Exit(1)
	}
	logger.Info("receive msg", zap.String("msg", string(buffer[:])))

	// 解析参数
	args := flag.Args()
	if flag.NArg() == 0 {
		println("not enough args!")
		os.Exit(2)
	}

	// reset_caps
	gofs.ResetCaps()
	logger.Info("reset caps success")

	// 当前文件的工作目录
	workDir, err := os.Getwd()
	if err != nil {
		println("getwd")
		os.Exit(1)
	}
	// 这里寻找一下用户程序的路径, 防止某些像python之类可执行文件的无法找到的情况
	cmdPath, err := exec.LookPath(args[0])
	if err != nil {
		println("exec.LookPath")
		os.Exit(1)
	}
	cmd := &exec.Cmd{
		Path:   cmdPath,
		Args:   args,
		Dir:    workDir,
		Env:    os.Environ(),
		Stdin:  os.Stdin,
		Stdout: os.Stdout,
		Stderr: os.Stderr,
		// 如果沙盒内需要多次启动子程序，可以考虑将这里的Setpgid放到cloneMain中
		SysProcAttr: &syscall.SysProcAttr{Setpgid: true, Pgid: os.Getpid()},
	}
	logger.Info("start user process", zap.String("cmdPath", cmdPath), zap.Strings("args", args))
	err = cmd.Run()
	if err != nil {
		fmt.Println("err in cmd.Run():", err.Error())
	}
}
