package main

import (
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"syscall"
)

func main() {
	cmd := exec.Command("sh")

	cmd.SysProcAttr = &syscall.SysProcAttr{
		Cloneflags: syscall.CLONE_NEWUTS |
			syscall.CLONE_NEWIPC |
			syscall.CLONE_NEWPID |
			syscall.CLONE_NEWNS |
			syscall.CLONE_NEWUSER |
			syscall.CLONE_NEWNET,
		UidMappings: []syscall.SysProcIDMap{
			{
				ContainerID: 0,
				HostID:      os.Getuid(),
				Size:        1, // Size为2, 则继续将HostID+1映射到ContainerID+1, 以此类推
			},
		},
		GidMappings: []syscall.SysProcIDMap{
			{
				ContainerID: 0,
				HostID:      os.Getgid(),
				Size:        1,
			},
		},
	}

	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Run(); err != nil {
		log.Fatal(err)
	}

	// 简单版
	// cmd := exec.Command(args[0], args[1:]...)
	// err = cmd.Run() // Run = Start + Wait
	// if err != nil {
	// 	fmt.Println("err in cmd.Run():", err.Error())
	// }

	// 获取并显示输出
	// out, err := cmd.Output()
	// if err != nil {
	// 	fmt.Printf("err in cmd.Output(): %s\n", err.Error())
	// }
	// fmt.Println("out", string(out))
	// err = cmd.Wait()
	// if err != nil {
	// 	fmt.Printf("err in cmd.Wait(): %w", err)
	// }
}

// 封装层级: exec.Cmd.Start > os.StartProcess > syscall.ForkExec

func fork3() (error, int) {
	args := os.Args
	procAttr := &syscall.ProcAttr{
		Env:   os.Environ(),
		Files: []uintptr{os.Stdin.Fd(), os.Stdout.Fd(), os.Stderr.Fd()},
	}

	// windows下不可用
	pid, err := syscall.ForkExec(args[0], []string{args[0]}, procAttr)
	if err != nil {
		return err, pid
	}
	return nil, pid
}

func fork2() (*exec.Cmd, error) {
	path, err := os.Executable() // 与 os.Args[0] 类似
	if err != nil {
		return nil, err
	}
	cmd := &exec.Cmd{
		Path:        path,               //文件路径 包括文件名
		Args:        []string{path},     //执行的命令
		Dir:         filepath.Dir(path), //文件路径 不包括文件名
		Env:         os.Environ(),       //环境变量
		Stdin:       os.Stdin,           // 输入
		Stdout:      os.Stdout,          // 输出
		Stderr:      os.Stderr,          // 错误
		SysProcAttr: &syscall.SysProcAttr{},
		/*		SysProcAttr: &syscall.SysProcAttr{
				Pdeathsig: syscall.SIGTERM,
			},*/
	}
	err = cmd.Start() // 不阻塞地执行
	if err != nil {
		return nil, err
	}
	/*	if err := cmd.Wait(); err != nil { // 阻塞
		log.Panicf("failed to wait command: %s", err)
	}*/
	return cmd, nil
}
