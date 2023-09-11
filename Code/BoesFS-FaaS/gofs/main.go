package main

import (
	"flag"
	gofs "gofs/src"
	"os"
)

func interact() {
	println("interact")
}

func main() {
	// gofs.InitLogger("./log/log.txt")
	// gofs.InitLogger("")
	if len(os.Args) > 1 {
		// 解析参数及判断其合法性
		arg_mountDir := flag.String("d", "", "mount directory")
		arg_userProg := flag.String("u", "", "user prog directory, empty to acl mode") // 不设置-u则为ACL模式
		arg_killNum := flag.Int("k", -1, "threshold to kill the program")
		arg_reload := flag.Bool("r", false, "reload")
		arg_model := flag.String("m", "~/.boesfs/acl/model/model.txt", "model file path")
		arg_policy := flag.String("p", "~/.boesfs/acl/model/policy.txt", "policy file path")
		arg_log := flag.String("l", "", "log file path")
		flag.Parse()
		args := flag.Args()
		c := make(chan int)
		gofs.NonInteract(c, *arg_mountDir, *arg_userProg, *arg_killNum, *arg_reload, *arg_model, *arg_policy, *arg_log, args)
		// go gofs.NonInteract(c, *arg_mountDir, *arg_userProg, *arg_killNum, *arg_reload, *arg_model, *arg_policy, *arg_log, args)
		// time.Sleep(time.Duration(3) * time.Second)
		// close(c)
	} else {
		interact()
	}
}
