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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/capability.h>

#include <ebpf.h>
#include <utils.h>
#include <interact.h>

#define STACK_SIZE (1024 * 1024)

#define PATH_MAX_SUPP 256

static struct interact_status {
    int is_ACL;
    char prog[PATH_MAX_SUPP];
    char mount_dir[PATH_MAX_SUPP];
    int verbose;
    int kill;
    char result_file[PATH_MAX_SUPP];
    int has_log;
    char log_file[PATH_MAX_SUPP];
} status = {1, "./acl/prog/acl_prog.o", "/", 1, 0, "./sample.txt", 0, "./sample.txt"};

static pid_list *head;

static char work_dir[PATH_MAX_SUPP] = {0};
static char acl_model[PATH_MAX_SUPP] = {0};
static char acl_policy[PATH_MAX_SUPP] = {0};

struct clone_args {
    char **argv;
    ebpf_ctxtext_t *ctx;
    int  *wait_pipe_fd;
    int  *pipe_fd;
    int  reload_fd;
    char filename[256];
};

static void reload_acl(pid_t pid, char *mount_dir, char *mount_str, char* filename, int index)
{
	ebpf_ctxtext_t *ctx = NULL;
    char buf[256] = "";
    // 使用sigstop停止执行二进制文件的进程
    kill(pid, SIGSTOP);

    // 用户态没有清空bpf map的方法，因此只能重新加载一次prog
    umount(mount_dir);
    ctx = ebpf_init(status.prog, index);

    if (!ctx)
    {
        info("Failed to load bpf prog!\n");
        kill(pid, SIGHUP);
        exit(EXIT_FAILURE);
    }
    int err = read_acl_rbac(ctx, acl_model, acl_policy, buf, filename);
    if(err)
    {
        info(buf);
        ebpf_fini(ctx);
        kill(pid, SIGHUP);
        exit(EXIT_FAILURE);
    }
    int i, len = strlen(mount_str);
    for(i = 0; i < len; i++)
        if(mount_str[i] == 'k')
            break;
    snprintf(buf, 256, "fd=%d,pid=%d,%s", ctx->ctrl_fd, pid, mount_str + i);
    if (mount(status.mount_dir, status.mount_dir, "boesfs", MS_NOSUID|MS_NODEV, buf))
    {
        info("mount() boesfs failed\n");
        ebpf_fini(ctx);
        kill(pid, SIGHUP);
        exit(EXIT_FAILURE);
    }
    ebpf_fini(ctx);

    // 使用SIGCONT恢复进程
    kill(pid, SIGCONT);
}

static void reload_list(void)
{
    if(!status.is_ACL)
    {
        info("not in ACL mode\n");
        return;
    }
    pid_list *prev_tmp, *tmp;
    tmp = head;
    while (tmp->next)
    {
        prev_tmp = tmp;
        tmp = tmp->next;
        if(kill(tmp->exec_pid, 0) < 0)
        {
            close(tmp->fd);
            prev_tmp->next = tmp->next;
            free(tmp);
        }
        else
        {
            write(tmp->fd, "r", 1);
        }
    }
    
    /***
     * 1.循环遍历链表
     * 2.判断pid对应的进程是否存在，若不存在，从链表删除该结点
     * 3.若存在，通知进程重新载入规则
     * 4.继续循环
     * */
}

static void release_list(void)
{
    pid_list *prev_tmp, *tmp;
    tmp = head;
    while (tmp->next)
    {
        prev_tmp = tmp;
        tmp = tmp->next;
        if(kill(tmp->exec_pid, 0) >= 0)
            kill(tmp->exec_pid, SIGHUP);
        close(tmp->fd);
        prev_tmp->next = tmp->next;
        free(tmp);
    }
    free(head);
}

static int make_prog_u(void)
{
    int status;
    char buf[256] = {0};
    //创建子进程让子进程去make，防止主进程被接管
    pid_t child_pid = fork();
    if(child_pid < 0)
    {
        // 创建子进程失败
        info("Failed to compile the bpf prog\n");
        return -1;
    }
    else if(child_pid == 0)
    {
        // 更改目录到prog下
        snprintf(buf, PATH_MAX_SUPP, "%s/prog/", work_dir);
        chdir(buf);
        char *command = "make";
        char *argument_list[] = {"make", NULL};
        // 子进程执行命令编译prog程序
        setenv("LLC", "llc", 1);
        setenv("CLANG", "clang", 1);
        int status = execvp(command, argument_list);
        // 执行失败需要返回failure
        if(status == -1)
            exit(EXIT_FAILURE);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // 等待子进程的结果
        waitpid(child_pid, &status, 0);
        // 必须要子进程正常结束才会返回0
        if(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
        {
            info("Success\n");
            return 0;
        }
        else
        {
            info("Failed to compile the bpf prog\n");
            return -1;
        }
        
    }
}

static int childFunc(void *arg)
{
    struct clone_args *args = (struct clone_args *) arg;
    char ch;
    int mounted = 0;
    char buf[256] = "";
    char mount_str[256] = "";
    pid_t elf_pid;
    int elf_pipe[2];
    
    // 关闭传递信息管道的读端口和同步管道的写端口
    close(args->pipe_fd[0]);
    close(args->wait_pipe_fd[1]);    
    
	write(args->pipe_fd[1], "start a child proc!\n", PATH_MAX_SUPP);
    // 等待父进程关闭同步管道的写端口
    if (read(args->wait_pipe_fd[0], &ch, 1) != 0)
    {
        write(args->pipe_fd[1], "Failure in child: read from pipe returned != 0\n", PATH_MAX_SUPP);
        close(args->pipe_fd[1]);
		goto end;
    }
    // 关闭同步管道
	close(args->wait_pipe_fd[0]);
    
    // 进入新的mount namespace
    if (unshare(CLONE_NEWNS) == -1)
    {
        write(args->pipe_fd[1], "Failure in child: failed to unshare\n", PATH_MAX_SUPP);
        close(args->pipe_fd[1]);
        goto end;
    }
    // 分配与运行不可信二进制文件的进程同步的管道
    if (pipe(elf_pipe) == -1)
    {
        write(args->pipe_fd[1], "Failure in child: failed to pipe elf_pipe\n", PATH_MAX_SUPP);
        close(args->pipe_fd[1]);
        goto end;
    }
    // 将子进程的根目录挂载点转换为从站挂载点
    if (mount("none", "/", NULL, MS_REC|MS_SLAVE, ""))
    {
        snprintf(buf, 1024, "mount '/' as slave failed: %s!\n", strerror(errno));
        write(args->pipe_fd[1], buf, PATH_MAX_SUPP);
        close(args->pipe_fd[1]);
        goto end;
    }
    // 创建子进程
    elf_pid = fork();
    if(elf_pid == 0)
    {
        // 关闭写端口
        close(elf_pipe[1]);    
        // 等待原进程关闭写端口
        if (read(elf_pipe[0], &ch, 1) != 0) {
            write(args->pipe_fd[1], "Failure in elf thread: read from pipe returned != 0\n", PATH_MAX_SUPP);
            close(args->pipe_fd[1]);
            exit(EXIT_FAILURE);
        }
        close(elf_pipe[0]);
        ebpf_fini(args->ctx);
        if (status.verbose)
            write(args->pipe_fd[1], "ready to reset capability\n", PATH_MAX_SUPP);
        //将子进程的capability置为全0，即限制子进程的能力
        reset_caps();
        if (status.verbose)
		    write(args->pipe_fd[1], "succeed resetting capability\n", PATH_MAX_SUPP);
            
	    write(args->pipe_fd[1], "start to run elf!\n", PATH_MAX_SUPP);
        // 转移程序的输出到结果文件
        int fd = open(status.result_file, O_WRONLY);
        if(!fd)
            write(args->pipe_fd[1], "open result file error\n", PATH_MAX_SUPP);
        else
        {
            dup2(fd, STDERR_FILENO);
            dup2(fd, STDOUT_FILENO);
        }
        // 更换工作目录，从而刷新缓存
        int dir = open(".", O_RDONLY);
        if(!dir)
            write(args->pipe_fd[1], "open dir error\nfailed to refresh cache\n", PATH_MAX_SUPP);
        else
        {
            chdir(status.mount_dir);
            fchdir(dir);
            close(dir);
        }
        close(args->pipe_fd[1]);
        execvp(args->argv[0], args->argv);
        exit(EXIT_FAILURE);
    }
    else
    {
        // 关闭读端口
        close(elf_pipe[0]);
        
        snprintf(buf, PATH_MAX_SUPP, "PID of child created by clone() is %ld\n", (long) elf_pid);
        write(args->pipe_fd[1], buf, PATH_MAX_SUPP);

        // 生成传递给文件系统的字符串
        if (args->ctx)
        {
            char tmp[PATH_MAX_SUPP] = {0};
            snprintf(tmp, sizeof(tmp), "%s_%d.txt", status.log_file, getpid());
            if(strlen(tmp) > 64)
            {
                write(args->pipe_fd[1], "log filename too long\n", PATH_MAX_SUPP);
                close(args->pipe_fd[1]);
                write(elf_pipe[1], "1", 1);
                close(elf_pipe[1]);
                goto end;
            }
            snprintf(mount_str, sizeof(buf), "fd=%d,pid=%d,kill=%d,log=%d,logpath=%s,elfpath=%s",
                args->ctx->ctrl_fd, elf_pid, status.kill, status.has_log, tmp, args->argv[0]);
        }

        //将boesfs挂载到此处，也就是说从这里开始对文件操作进行检查
        if (mount(status.mount_dir, status.mount_dir, "boesfs", MS_NOSUID|MS_NODEV, mount_str))
        {
            write(args->pipe_fd[1], "mount() boesfs failed\n", PATH_MAX_SUPP);
            close(args->pipe_fd[1]);
            write(elf_pipe[1], "1", 1);
            close(elf_pipe[1]);
            goto end;
        }

        mounted = 1;
        snprintf(buf, 25, "mypid%d\n", elf_pid);
        write(args->pipe_fd[1], buf, 25);
        ebpf_fini(args->ctx);

        if (status.verbose)
        {
            snprintf(buf, 256, "succeed mounting Boesfs on %s!\n", status.mount_dir);
            write(args->pipe_fd[1], buf, PATH_MAX_SUPP);
        }

        close(elf_pipe[1]);
	    close(args->pipe_fd[1]);

        // 等待运行elf的进程结束
        snprintf(buf, 256, "%s", status.mount_dir);
        int index = 1;
        while (read(args->reload_fd, &ch, 1) != 0)
        {
            reload_acl(elf_pid, buf, mount_str, args->filename, index);
            index++;
        }
    }
    // 卸载boesfs文件系统并退出子进程
end:
    if(mounted)
        umount(status.mount_dir);
    exit(EXIT_FAILURE);
}

static int run_elf_boesfs(char *command, int pipe_fd, int reload_fd)
{
	ebpf_ctxtext_t *ctx = NULL;
    char command_buf[256] = "";
    char buf[256] = "";
    int argc = 0;
    char *argv[20] = {0};
    char child_stack[STACK_SIZE];
    int new_pipe[2];
    int wait_pipe[2];
    int flags = SIGCHLD | CLONE_NEWUSER;

    strcpy(command_buf, command);
    char *p = strtok(command_buf," ");
    while(p)
    {
        argv[argc]=p;
        argc ++;
        p = strtok(NULL," ");
    }

    if(create_file(status.result_file))
    {
        snprintf(buf, PATH_MAX_SUPP, "failed to create result file: %s!\n", status.result_file);
        info(buf);
        close(pipe_fd);
        close(reload_fd);
        goto error;
    }

    // 该pipe用于子进程传递信息给父进程
    if (pipe(new_pipe) == -1)
    {
        info("create pipe failed\n");
        close(pipe_fd);
        close(reload_fd);
        exit(EXIT_FAILURE);
    }

    // 该pipe用于子父进程同步
    if (pipe(wait_pipe) == -1)
    {
        info("create pipe failed\n");
        close(pipe_fd);
        close(reload_fd);
        exit(EXIT_FAILURE);
    }

    // 读取prog
    ctx = ebpf_init(status.prog, 0);

    if (!ctx)
    {
        info( "Failed to load bpf prog!\n");
        close(pipe_fd);
        close(reload_fd);
        exit(EXIT_FAILURE);
    }
    if (status.verbose)
    {
        snprintf(buf, PATH_MAX_SUPP, "Loaded sandbox progam %s with fd=%d\n", status.prog, ctx->ctrl_fd);
        info(buf);
    }

    struct clone_args args = {argv, ctx, wait_pipe, new_pipe, reload_fd, ""};
    if(get_filename(argv[0], args.filename))
    {
        perror("failed to get filename");
        close(pipe_fd);
        close(reload_fd);
        goto error;
    }

    // 根据模式写入规则到map中
    if(status.is_ACL)
    {
        int err = read_acl_rbac(ctx, acl_model, acl_policy, buf, args.filename);
        if(err)
        {
            info(buf);
            close(pipe_fd);
            close(reload_fd);
            goto error;
        }
    }
    else
    {
        /***
         * write code here when not in ACL mode
         * example as followed:
         */
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

    
    //在创建子进程的同时创建新的namespace，子进程会在一个新user namespace中
    pid_t child_pid = clone(childFunc, child_stack + STACK_SIZE, flags, &args);
    if (child_pid == -1)
    {
        info("clone() a process failed\n");
        close(pipe_fd);
        close(reload_fd);
        goto error;
    }

    if (status.verbose)
        info("succeed creating a new user namespace\n");
    // 关闭同步管道的读端口
    close(wait_pipe[0]);

    // 更行子进程的uid和gid映射
    if (setgroups_control(child_pid, SETGROUPS_DENY))
	{
        write(wait_pipe[1], "1", 1);
        close(wait_pipe[1]);
        info("failed to setgroups_control\n");
        close(pipe_fd);
        close(reload_fd);
    	goto error;
    }

    if (update_uid_map(child_pid, geteuid(), geteuid()) ||
		update_gid_map(child_pid, getegid(), getegid()))
	{
        write(wait_pipe[1], "1", 1);
        close(wait_pipe[1]);
        info("failed to update map\n");
        close(pipe_fd);
        close(reload_fd);
    	goto error;
    }
    // 关闭同步管道的写口
    close(wait_pipe[1]);

    close(new_pipe[1]);
    while (read(new_pipe[0], buf, PATH_MAX_SUPP) != 0)
    {
        if(!memcmp("mypid", buf, 5))
            write(pipe_fd, buf + 5, 20);
    }
    close(new_pipe[0]);
    write(pipe_fd, &child_pid, 20);
    close(pipe_fd);
    close(reload_fd);
    ebpf_fini(ctx);
    // 等到子进程的结束
    if (waitpid(child_pid, NULL, 0) == -1)
        goto error;
    exit(EXIT_SUCCESS);
error:
    if (ctx)
        ebpf_fini(ctx);
    exit(EXIT_FAILURE);
}

void interactive(void)
{
    int choice;
    int yes;
    struct stat ststat;
    char path[PATH_MAX_SUPP] = {0};
    char *home;
    home = getenv("HOME");
    snprintf(work_dir, 256, "%s/.boesfs", home);
    if(stat(work_dir, &ststat) || !S_ISDIR(ststat.st_mode))
        mkdir(work_dir, 0774);
    snprintf(path, PATH_MAX_SUPP, "%s/result", work_dir);
    if(stat(path, &ststat) || !S_ISDIR(ststat.st_mode))
        mkdir(path, 0774);
    snprintf(path, PATH_MAX_SUPP, "%s/log", work_dir);
    if(stat(path, &ststat) || !S_ISDIR(ststat.st_mode))
        mkdir(path, 0774);
    snprintf(status.prog, PATH_MAX_SUPP, "%s/acl/prog/acl_prog.o", work_dir);
    snprintf(acl_model, PATH_MAX_SUPP, "%s/acl/model/model.txt", work_dir);
    snprintf(acl_policy, PATH_MAX_SUPP, "%s/acl/model/policy.txt", work_dir);
    head = (pid_list *)malloc(sizeof(pid_list));
    head->next = NULL;

    while (1)
    {
        //打印交互界面的菜单
        menu();
        // 获取用户的选择
        choice = get_choice();
        switch (choice)
        {
            case 1:
                yes = get_yn("Use User-defined-prog mode (default ACL/RBAC mode)?[Y/n]");
                if(yes)
                {
                    status.is_ACL = 0;
                    snprintf(status.prog, PATH_MAX_SUPP, "%s/prog/user_prog.o", work_dir);
                    info("You are now in User-defined-prog mode\n");
                    info("Compiling prog/user_prog.c for you\n");
                    // 对用户更改过后的user_prog.c重新编译
                    int err = make_prog_u();
                    if(err)
                        info("Failed\n");
                    else
                        info("Success\n");
                }
                else
                {
                    status.is_ACL = 1;
                    snprintf(status.prog, PATH_MAX_SUPP, "%s/acl/prog/acl_prog.o", work_dir);
                    info("You are now in ACL mode\n");
                }
                break;
            case 2:
                // 用户输入新的挂载点
                get_path("Please input the new mount point:", status.mount_dir);
                break;
            case 3:
                // 设置是否打印信息
                yes = get_yn("Display verbose messages?[Y/n]");
                if(!yes)
                    status.verbose = 0;
                else
                    status.verbose = 1;
                break;
            case 4:
                yes = get_yn("Kill the process when it has triggered multiple 'denied' events?[Y/n]");
                // 修改当触发denied次数过多时是否杀死子进程
                if(!yes)
                    status.kill = 0;
                else
                    // 若杀死子进程，设定杀死子进程的数量
                    status.kill = get_num("Please input the number of 'denied' events before kill the process[0~9999]:");
                break;
            case 5:
                // 用户设置是否使用日志功能，并提示可能会降低性能
                info("Use Log (default no)?");
                yes = get_yn("(Warning:Using logs may significantly reduce performance)[Y/n]:");
                if(!yes)
                    status.has_log = 0;
                else
                    status.has_log = 1;
                break;
            case 6:
            {
                char buf[PATH_MAX_SUPP] = {0};
                char command[128] = {0};
                int pipe_fd[2], reload_fd[2];

                // 获取需要执行的执行命令
                get_command(command);
                // 获取文件名
                if(get_filename(command, buf))
                {
                    info("filename too long\n");
                    break;
                }
                // 生成结果文件路径和日志文件路径
                snprintf(status.result_file, sizeof(status.result_file), "%s/result/%s.txt", work_dir, buf);
                snprintf(status.log_file, sizeof(status.log_file), "%s/log/%s", work_dir, buf);
                // 使用管道在子父进程间交换信息
                pipe(pipe_fd);
                pipe(reload_fd);
                pid_t pid = fork();
                if(pid == 0)
                {
                    close(pipe_fd[0]);
                    close(reload_fd[1]);
                    run_elf_boesfs(command, pipe_fd[1], reload_fd[0]);
                }
                else
                {
                    pid_list *list;
                    close(pipe_fd[1]);
                    close(reload_fd[0]);
                    list = (pid_list *)malloc(sizeof(pid_list));
                    if(read(pipe_fd[0], buf, 20) != 0)
                    {
                        list->exec_pid = atoi(buf);
                    }
                    if(read(pipe_fd[0], &list->pid, 20) == 0)
                    {
                        free(list);
                    }
                    list->fd = reload_fd[1];
                    list->next = head->next;
                    head->next = list;
                    close(pipe_fd[0]);
                }

                break;
            }
            case 7:
                // 打印最新的结果文件
                if(!strcmp(status.result_file, "./sample.txt"))
                    info("No result is generated right now\n");
                else
                    dump_file(status.result_file);
                break;
            case 8:
                reload_list();
                break;
            case 9:
                release_list();
                exit(0);
            default:
                break;
        }
    }
    

}