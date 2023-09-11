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
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/capability.h>

#include <ebpf.h>
#include <utils.h>
#include <non_interact.h>
#include <acl_public.h>

static int verbose = 0;
static int kill_num;
static int has_log = 0;
static char work_dir[256] = {0};
static char model_file[256] = {0};
static char policy_file[256] = {0};

struct child_args {
    char **argv;        /* Command to be executed by child, with arguments */
	ebpf_ctxtext_t *ctx;
    int    pipe_fd[2];  /* Pipe used to synchronize parent and child */
	char  *mount_dir;
};

static int              /* Start function for cloned child */
childFunc(void *arg)
{
    struct child_args *args = (struct child_args *) arg;
    char ch;
    int mounted = 0;
    char buf[256] = "";
    pid_t elf_pid;
    int elf_pipe[2];

    
    close(args->pipe_fd[1]);    /* Close our descriptor for the write end
                                   of the pipe so that we see EOF when
                                   parent closes its descriptor */
    if (read(args->pipe_fd[0], &ch, 1) != 0) {
        fprintf(stderr, "Failure in child: read from pipe returned != 0\n");
		exit(EXIT_FAILURE);
    }

	close(args->pipe_fd[0]);


    if (unshare(CLONE_NEWNS) == -1) {
        perror("unshare");
        goto end;
    }
    
    if (pipe(elf_pipe) == -1) {
        perror("pipe");
        goto end;
    }

    if (verbose)
        print_status("=== AFTER CLONE_NEWUSER | CLONE_NEWNS ===");

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
        goto end;
    }

    elf_pid = fork();
    if(elf_pid == 0)
    {
        close(elf_pipe[1]);    /* Close our descriptor for the write end
                                   of the pipe so that we see EOF when
                                   parent closes its descriptor */
        if (read(elf_pipe[0], &ch, 1) != 0) {
            fprintf(stderr, "Failure in child: read from pipe returned != 0\n");
            exit(EXIT_FAILURE);
        }

        close(elf_pipe[0]);

        if (verbose)
            print_status("=== Child Process : Before reset_caps ===");

        // drop privileges
        //将子进程的capability置为全0，即限制子进程的能力
        reset_caps();

        if (verbose)
            print_status("=== Child Process : After reset_caps ===");

        printf("start a child proc!\n");
        /* Execute a shell command */
        //这里的参数就是环境，比如/bin/bash
        int dir = open(".", O_RDONLY);
        if(!dir)
            fprintf(stderr, "open dir error\nfailed to refresh cache\n");
        else
        {
            chdir(args->mount_dir);
            fchdir(dir);
            close(dir);
        }

        execvp(args->argv[0], args->argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        close(elf_pipe[0]);
        printf("PID of child created by clone() is %ld\n", (long) elf_pid);

        if (args->ctx)
        {
            char tmp[PATH_MAX_SUPP] = {0};
            get_filename(args->argv[0], buf);
            snprintf(tmp, sizeof(tmp), "%s/log/%s_%d.txt", work_dir, buf, getpid());
            if(strlen(tmp) > 64)
            {
                write(elf_pipe[1], "1", 1);
                close(elf_pipe[1]);
                perror("log filename too long");
                goto end;
            }
            snprintf(buf, sizeof(buf), "fd=%d,pid=%d,kill=%d,log=%d,logpath=%s,elfpath=%s",
                args->ctx->ctrl_fd, elf_pid, kill_num, has_log, tmp, args->argv[0]);
        }

        if (mount(args->mount_dir, args->mount_dir, "boesfs", MS_NOSUID|MS_NODEV, buf))
        {
            write(elf_pipe[1], "1", 1);
            close(elf_pipe[1]);
            perror("mount() boesfs");
            goto end;
        }

    
        //将命令中地址所有内容再次挂载到挂载点
        //但从该文件系统执行程序时，不会设置进程的uid，gid和文件的capability权限，也不允许访问此文件系统上的设备（特殊文件）

        mounted = 1;

        if (verbose)
            print_status("=== Mounted Boesfs ===");

        close(elf_pipe[1]);

        /* Wait for child */
        if (waitpid(elf_pid, NULL, 0) == -1) {
            perror("waitpid");
            goto end;
        }
    }
end:
    if(mounted){
        // printf("umount!!!!\n");
        umount(args->mount_dir);
    }
    exit(EXIT_FAILURE);
}

static char child_stack[STACK_SIZE];    /* Space for child's stack */

void non_interactive(int argc, char *argv[])
{
    info("Welcome to BoesFS-Agent!\n");
    int flags = SIGCHLD | CLONE_NEWUSER;
    int opt, is_acl = 1;
    pid_t child_pid;
    struct child_args args;
    char prog[256] = {0};
    ebpf_ctxtext_t *ctx = NULL;
    char buf[256] = {0};
    struct stat ststat;
    char *home;
    home = getenv("HOME");
    snprintf(work_dir, 256, "%s/.boesfs", home);
    if(stat(work_dir, &ststat) || !S_ISDIR(ststat.st_mode))
        mkdir(work_dir, 0774);
    snprintf(buf, PATH_MAX_SUPP, "%s/log", work_dir);
    if(stat(buf, &ststat) || !S_ISDIR(ststat.st_mode))
        mkdir(buf, 0774);

    /* Parse command-line options. The initial '+' character in
    the final getopt() argument prevents GNU-style permutation
    of command-line options. That's useful, since sometimes
    the 'command' to be executed by this program itself
    has command-line options. We don't want getopt() to treat
    those as options to this program. */
    while ((opt = getopt(argc, argv, "+d:u::vk:l")) != -1)
    {
        switch (opt)
        {
        case 'd':
            args.mount_dir = optarg;
            break;
        case 'u':
            strncpy(prog, optarg, PATH_MAX_SUPP);
            is_acl = 0;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'k':
            kill_num = atoi(optarg);
            break;
        case 'l':
            has_log = 1;
            break;
        default:
            usage(argv[0]);
        }
    }

    if (verbose)
        print_status("=== Parent ===");
    // 查看挂载点是否存在以及可操作
    if (!args.mount_dir)
    {
        fprintf(stderr, "Specify file system dir to sandbox\n");
        usage(argv[0]);
    }
    else
        check_access(args.mount_dir);
    // 检查ACL模式下用户给定的文件是否可访问
    if(is_acl)
    {
        snprintf(prog, PATH_MAX_SUPP, "%s/acl/prog/acl_prog.o", work_dir);
        snprintf(model_file, PATH_MAX_SUPP, "%s/acl/model/model.txt", work_dir);
        snprintf(policy_file, PATH_MAX_SUPP, "%s/acl/model/policy.txt", work_dir);
        check_access(model_file);
        check_access(policy_file);
    }
    //检查是否给定挂载后的环境（如/bin/bash）
    if (optind == argc) {
        fprintf(stderr, "Specify shell command to execute\n");
        usage(argv[0]);
    }
    
    args.argv = &argv[optind];

    if (pipe(args.pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* load sandboxing bpf program */
    //如上所述，插入bpf文件到沙盒层
    if (prog) {
        ctx = ebpf_init(prog, 0);
        if (!ctx) {
            fprintf(stderr, "Failed to load bpf prog!\n");
            exit(EXIT_FAILURE);
        }

        if (verbose)
            printf("Loaded sandbox progam %s with fd=%d\n",
                    prog, ctx->ctrl_fd);

    }
    if (!is_acl)
    {
        int val = -EACCES;
		int map_idx = 0; /* first map */
		char key0[PATH_MAX_SUPP] = {0};
		char key1[PATH_MAX_SUPP] = {0};
		char key2[PATH_MAX_SUPP] = {0};
		char key3[PATH_MAX_SUPP] = {0};
		char key4[PATH_MAX_SUPP] = {0};
		char key5[PATH_MAX_SUPP] = {0};
		char key6[PATH_MAX_SUPP] = {0};


		val = 0;
		strncpy(key0, "/home/boes/read.txt", PATH_MAX_SUPP);
		if (ebpf_data_update(ctx, (void *)key0, (void *)&val, map_idx, 1))
			perror("ebpf_data_update UNACCESS");
		
		val = 1;
		strncpy(key1, "/home/boes/write.txt", PATH_MAX_SUPP);
		if (ebpf_data_update(ctx, (void *)key1, (void *)&val, map_idx, 1))
			perror("ebpf_data_update UNACCESS");

		val = 2;
		strncpy(key2, "/home/boes/lookup.txt", PATH_MAX_SUPP);
		if (ebpf_data_update(ctx, (void *)key2, (void *)&val, map_idx, 1))
			perror("ebpf_data_update UNACCESS");

		val = 3;
		strncpy(key3, "/home/boes/open.txt", PATH_MAX_SUPP);
		if (ebpf_data_update(ctx, (void *)key3, (void *)&val, map_idx, 1))
			perror("ebpf_data_update UNACCESS");

		val = 4;
		strncpy(key4, "/home/boes/mkdir.txt", PATH_MAX_SUPP);
		if (ebpf_data_update(ctx, (void *)key4, (void *)&val, map_idx, 1))
			perror("ebpf_data_update UNACCESS");

		val = 5;
		strncpy(key5, "/home/boes/unlink.txt", PATH_MAX_SUPP);
		if (ebpf_data_update(ctx, (void *)key5, (void *)&val, map_idx, 1))
			perror("ebpf_data_update UNACCESS");

		val = 6;
		strncpy(key6, "/home/boes/rmdir.txt", PATH_MAX_SUPP);
		if (ebpf_data_update(ctx, (void *)key6, (void *)&val, map_idx, 1))
			perror("ebpf_data_update UNACCESS");

    }
    else
    {
        char tmp_elf[PATH_MAX_SUPP] = {0};
        if(get_filename(args.argv[0], tmp_elf))
        {
            perror("failed to get filename");
            goto error;
        }
        int err = read_acl_rbac(ctx, model_file, policy_file, buf, tmp_elf);
        if(err)
        {
            perror(buf);
            goto error;
        }
    }


    args.ctx = ctx;

    /* Create the child in new namespace(s) */
    //在新的namespace里创建子进程，当flags = SIGCHLD时，clone相当于fork，因为没有指定namespace类型
    child_pid = clone(childFunc, child_stack + STACK_SIZE, flags, &args);
    if (child_pid == -1) {
        perror("clone()");
        goto error;
    }

    close(args.pipe_fd[0]);

    /* Parent falls through to here */
    if (verbose)
        printf("succeed creating a new user namespace\n");

	/* since Linux 3.19 unprivileged writing of /proc/self/gid_map
	 * has s been disabled unless /proc/self/setgroups is written
	 * first to permanently disable the ability to call setgroups
	 * in that user namespace. */
	if (setgroups_control(child_pid, SETGROUPS_DENY))
	{
        write(args.pipe_fd[1], "1", 1);
        close(args.pipe_fd[1]);
    	goto error;
    }

    if (update_uid_map(child_pid, geteuid(), geteuid()) ||
		update_gid_map(child_pid, getegid(), getegid()))
	{
        write(args.pipe_fd[1], "1", 1);
        close(args.pipe_fd[1]);
    	goto error;
    }
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
	return;
}