/*
   Based on userns_child_exec.c from Michael Kerrisk.

   Copyright 2018, Michael Kerrisk
   Copyright 2018, Ashish Bijlani
   Licensed under GNU General Public License v2 or later

   Execute a process under SandFS sandbox.

   It first create a child process that executes a shell command in new
   namespace(s); allow UID and GID mappings to be specified when
   creating a user namespace.
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <ebpf.h>
#include <utils.h>

#define STACK_SIZE (1024 * 1024)

#define PATH_MAX_SUPP 256

static int verbose;

struct child_args {
    char **argv;        /* Command to be executed by child, with arguments */
    int    pipe_fd[2];  /* Pipe used to synchronize parent and child */
};

static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s [options] app [arg...]\n\n", pname);
    fprintf(stderr, "Executes a shell command under a sandbox.\n\n");
    fprintf(stderr, "Options can be:\n\n");
#define fpe(str) fprintf(stderr, "    %s", str);
    fpe("-d          File system dir to sandbox\n");
    fpe("-s          BPF sandboxing program file path\n");
    fpe("-v          Display verbose messages\n");
    fpe("\n");
    exit(EXIT_FAILURE);
}

static int              /* Start function for cloned child */
childFunc(void *arg)
{
    struct child_args *args = (struct child_args *) arg;
    char ch;

    /* Wait until the parent has updated the UID and GID mappings. See
       the comment in main(). We wait for end of file on a pipe that will
       be closed by the parent process once it has updated the mappings. */
	   //等待父进程改uid和gid，实际上没有改
    close(args->pipe_fd[1]);    /* Close our descriptor for the write end
                                   of the pipe so that we see EOF when
                                   parent closes its descriptor */
    if (read(args->pipe_fd[0], &ch, 1) != 0) {
        fprintf(stderr, "Failure in child: read from pipe returned != 0\n");
		exit(EXIT_FAILURE);
    }

	close(args->pipe_fd[0]);

	if (verbose)
		stats("=== Child Process : Before reset_caps ===");

    // drop privileges
	//将子进程的capability置为全0，即限制子进程的能力
	reset_caps();
	/***
	 * 新添加的功能，更改子进程工作目录（可能重载了目录内存因此修复了刚启动禁用列表无效的bug
	*/
	chdir("/home/fs/");
	if (verbose)
		stats("=== Child Process : After reset_caps ===");

	printf("start a child proc!\n");
    /* Execute a shell command */
	//这里的参数就是环境，比如/bin/bash
    execvp(args->argv[0], args->argv);
    perror("execvp");
	exit(EXIT_FAILURE);
}

static char child_stack[STACK_SIZE];    /* Space for child's stack */
//  这就是user prog阿
int
main(int argc, char *argv[])
{
	printf("Welcome to LibSandFS!\n");	
    int flags, opt;
    pid_t child_pid;
    struct child_args args;
	char *prog = NULL;
	char *dir = NULL;
    ebpf_context_t *ctx = NULL;
    char opts[256] = "";

    /* Parse command-line options. The initial '+' character in
       the final getopt() argument prevents GNU-style permutation
       of command-line options. That's useful, since sometimes
       the 'command' to be executed by this program itself
       has command-line options. We don't want getopt() to treat
       those as options to this program. */
    flags = CLONE_NEWNS | CLONE_NEWUSER;
	// 获取命令行参数，-d即挂载点，-s即bpf的elf文件路径，-v为打印该命令的运行信息
    while ((opt = getopt(argc, argv, "+d:s:v")) != -1) {
        switch (opt) {
		case 'd': dir = optarg;                 break;
        case 's': prog = optarg;                break;
        case 'v': verbose = 1;                  break;
        default:  usage(argv[0]);
        }
    }

	if (verbose)
		stats("=== Parent ===");
	// 查看挂载点是否存在以及可操作
	if (!dir) {
		fprintf(stderr, "Specify file systen dir to sandbox\n");
		usage(argv[0]);
	} else {
		if (access(dir, R_OK)) {
			fprintf(stderr, "File system dir %s is not accessible: %s!\n",
				dir, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	//检查是否给定挂载后的环境（如/bin/bash）
	if (optind == argc) {
		fprintf(stderr, "Specify shell command to execute\n");
		usage(argv[0]);
	}

    /* We use a pipe to synchronize the parent and child, in order to
       ensure that the parent sets the UID and GID maps before the child
       calls execve(). This ensures that the child maintains its
       capabilities during the execve() in the common case where we
       want to map the child's effective user ID to 0 in the new user
       namespace. Without this synchronization, the child would lose
       its capabilities if it performed an execve() with nonzero
       user IDs (see the capabilities(7) man page for details of the
       transformation of a process's capabilities during execve()). */
	/*我们使用管道来同步父子进程，以确保父进程在子进程调用execve（）之前设置UID和GID映射。
		这确保了子进程的execve（）在处于我们希望将子进程的有效UID映射到新namespace中时为0的常见情况下，保持其capability权限。
		如果没有这种同步，如果子进程使用非零用户ID执行execve（），它将失去capability权限。*/
    if (pipe(args.pipe_fd) == -1) {
        perror("pipe");
		exit(EXIT_FAILURE);
	}

	/* load sandboxing bpf program */
	//如上所述，插入bpf文件到沙盒层
	if (prog) {
		ctx = ebpf_init(prog);
    	if (!ctx) {
			fprintf(stderr, "Failed to load bpf prog!\n");
			exit(EXIT_FAILURE);
		}

    	if (verbose)
			printf("Loaded sandbox progam %s with fd=%d\n",
					prog, ctx->ctrl_fd);

		/* Example: update ebpf map to not allow access to ssh keys */
		//  这个val的值感觉应该让SANDFS自己定义一系列宏，每个宏代表不同的禁止操作。
		//  比如说SANDFS_UNACCESS , SANDFS_UNLINK ...
		int val = -EACCES;
		int map_idx = 0; /* first map */
		char key[PATH_MAX_SUPP] = {0};
		strncpy(key, "/home/fs/.ssh", PATH_MAX_SUPP);
		if (ebpf_data_update(ctx, (void *)key, (void *)&val, map_idx, 0))
			perror("ebpf_data_update UNACCESS");
		
		//  增添
		// val = -EPERM;
		// memset(key,0,sizeof key);
		// strncpy(key,"/home/fs/a.txt",PATH_MAX_SUPP);
		// if(ebpf_data_update(ctx,(void*)key,(void*)&val,map_idx,0))
		// 	perror("ebpf_data update UNLINK");

	}
	// 在这里创建了新的namespace，使该进程直接跳入到一个mount namespace和user namespace中
	// 即在这一步之后，进程的用户已经变成nobody，且此后挂载的文件系统已被隔离开来

    if (unshare(flags) == -1) {
		perror("unshare");
		goto error;
	}

    if (verbose)
		stats("=== AFTER CLONE_NEWUSER | CLONE_NEWNS ===");

	/*
	 * Recursively mount everything below / as slave in new fs namespace
	 * this should override shared propagation.
	 */
	// 将该用户的根目录挂载点转换为从站挂载点，即
	// 其他用户出现的挂载或卸载事件会传播到该用户
	// 而该用户出现的挂载或卸载事件不会传播到其他用户
	if (mount("none", "/", NULL, MS_REC|MS_SLAVE, "")) {
		fprintf(stderr, "mount '/' as slave failed: %s!\n", strerror(errno));
		perror("mount slave");
		goto error;
	}

	/* sandbox file system */
	if (ctx)
    	snprintf(opts, sizeof(opts), "fd=%d", ctx->ctrl_fd);

	//将命令中地址所有内容再次挂载到挂载点
	//但从该文件系统执行程序时，不会设置进程的uid，gid和文件的capability权限，也不允许访问此文件系统上的设备（特殊文件）
	if (mount(dir, dir, "sandfs", MS_NOSUID|MS_NODEV, opts)) {
		perror("mount() sandfs");
		goto error;
	}

    if (verbose)
		stats("=== Mounted Sandfs ===");
	//获取挂载后的环境
    args.argv = &argv[optind];
	flags = SIGCHLD;

    /* Create the child in new namespace(s) */
	//子进程沙盒，使用clone()创建子进程沙箱。
	//注释有误，当flags = SIGCHLD时，clone相当于fork，因为没有指定namespace类型
    child_pid = clone(childFunc, child_stack + STACK_SIZE, flags, &args);
    if (child_pid == -1) {
		perror("clone()");
        goto error;
	}

    /* Parent falls through to here */
    if (verbose)
        printf("PARENT %s: PID of child created by clone() is %ld\n",
                argv[0], (long) child_pid);

	/* since Linux 3.19 unprivileged writing of /proc/self/gid_map
	 * has s been disabled unless /proc/self/setgroups is written
	 * first to permanently disable the ability to call setgroups
	 * in that user namespace. */
	//if (setgroups_control(child_pid, SETGROUPS_DENY))
	//	goto error;

    //if (update_uid_map(getpid(), ROOT_UID, geteuid(), verbose) ||
	//	update_gid_map(getpid(), ROOT_GID, getegid(), verbose))
	//	goto error;
	// 它把更新UID和GID的部分注释了，推测为Linux3.19后的版本去掉注释
	// 是他更新失败了，一把子无语住了
    /* Close the write end of the pipe, to signal to the child that we
       have updated the UID and GID maps */
    close(args.pipe_fd[1]);

	/* Wait for child */
    if (waitpid(child_pid, NULL, 0) == -1) {
        perror("waitpid");
		goto error;
	}

	if (verbose)
        printf("%s: terminating\n", argv[0]);

error:
	if (ctx)
    	ebpf_fini(ctx);
	return 0;
}

