/*
 * Copyright (c) 2019, 2020 Oracle and/or its affiliates. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 package commands

 import (
	 "log"
	 "os"
	 "os/exec"
	 "os/signal"
	 "syscall"
	 "fmt"
	 "regexp"
	 "path/filepath"
 
	 "github.com/urfave/cli"
 )
 
 // StartCommand returns start server cli.command
 func StartCommand() cli.Command {
	 return cli.Command{
		 Name:        "start",
		 Usage:       "Start a local Fn server",
		 Category:    "SERVER COMMANDS",
		 Description: "This command starts a local Fn server by downloading its docker image.",
		 Action:      start,
		 Flags: []cli.Flag{
			 cli.StringFlag{
				 Name:  "version",
				 Usage: "Specify a specific fnproject/fnserver version to run, ex: '1.2.3'.",
				 Value: "latest",
			 },
			 cli.IntFlag{
				 Name:  "port, p",
				 Value: 8080,
				 Usage: "Specify port number to bind to on the host.",
			 },
		 },
	 }
 }
 
 func start(c *cli.Context) error {

	// 创建/tmp/fn工作文件夹
	homeDir, _ := os.UserHomeDir()
	workDir := homeDir+"/.boesfs-faas"

	if _, err := os.Stat(workDir); os.IsNotExist(err) {
		// 文件夹不存在，创建它
		err := os.Mkdir(workDir, 0755)
		if err != nil {
			fmt.Println("Fail to create work dir : ~/.boesfs-faas", err)
			return err
		}
	}

	// 清理残破的sock工作

	// 清理残破的sock工作
	sockErr := filepath.Walk(workDir, func(path string, info os.FileInfo, err error) error {
		regex := regexp.MustCompile(`.*\.sock$`)
		if regex.MatchString(path) {
			err := os.Remove(path)
			if err != nil {
				fmt.Printf("清理套接字文件时发生错误：%v\n", err)
				return err
			}
		}
		return nil
	})

	if sockErr != nil {
		fmt.Println("套接字清理工作失败\n")
	}

	// 设置环境变量
	os.Setenv("LD_LIBRARY_PATH", homeDir+"/.boesfs/acl/library")
 
	 // TODO: 完善BoesFS沙盒运行fnserver
	 // args := []string{""}
	 // cmd := exec.Command("boesfs", args...)
	 cmd := exec.Command("boesserver")
	 cmd.Stdout = os.Stdout
	 cmd.Stderr = os.Stderr
	 cmd.Env = os.Environ()
	 err := cmd.Start()
	 if err != nil {
		 log.Fatalln("Starting command failed:", err)
	 }
 
	 done := make(chan error, 1)
	 go func() {
		 done <- cmd.Wait()
	 }()
	 // catch ctrl-c and kill
	 sigC := make(chan os.Signal, 2)
	 signal.Notify(sigC, os.Interrupt, syscall.SIGTERM)
 
	 log.Println("¡¡¡ 'fn start' should NOT be used for PRODUCTION !!! see https://github.com/fnproject/fn-helm/")
 
	 for {
		 select {
		 case <-sigC:
			 log.Println("Interrupt caught, exiting")
			 err = cmd.Process.Signal(syscall.SIGTERM)
			 if err != nil {
				 log.Println("Error: could not kill process:", err)
				 return err
			 }
		 case err := <-done:
			 if err != nil {
				 log.Println("Error: processed finished with error", err)
			 }
		 }
		 return err
	 }
 }
 