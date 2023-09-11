#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/mount.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <ebpf.h>

#include <mntent.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/mount.h>
#include <grp.h>

#include <utils.h>
#include <ebpf.h>
#include <acl_public.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static const char *setgroups_strings[] =
{
	[SETGROUPS_DENY] = "deny",
	[SETGROUPS_ALLOW] = "allow"
};

uint32_t hash_uint32(uint32_t integer)
{
	return HASH(integer);
}

uint32_t hash_str(char * str)
{
	uint32_t hash = 1;
	int len = strlen(str);
	int i = 0;
	for(i = 0; i < len; i++){
		hash = hash * (uint32_t)str[i];
	}
	return hash_uint32(hash);
}

uint32_t s2d(char *str)
{
    uint32_t result = 0;
    while(*str)
    {
        result = result * 10 + *str - '0';
        str++;
    }
    return result;
}

/***
 * 函数作用：匹配操作类型
 * str：要检查的字符串指针
 * 返回值：-1：匹配失败；其他：操作类型的值
*/
int check_op(char *str)
{
	if(!strcmp(str, "lookup"))
		return BOESFS_LOOKUP;
	else if(!strcmp(str, "open"))
		return BOESFS_OPEN;
	else if(!strcmp(str, "read"))
		return BOESFS_READ;
	else if(!strcmp(str, "write"))
		return BOESFS_WRITE;
	else if(!strcmp(str, "unlink"))
		return BOESFS_UNLINK;
	else if(!strcmp(str, "mkdir"))
		return BOESFS_MKDIR;
	else if(!strcmp(str, "rmdir"))
		return BOESFS_RMDIR;
	else if(!strcmp(str, "mknod"))
		return BOESFS_MKNOD;
	else if(!strcmp(str, "create"))
		return BOESFS_CREATE;
	else if(!strcmp(str, "link"))
		return BOESFS_LINK;
	else if(!strcmp(str, "symlink"))
		return BOESFS_SYMLINK;
	else if(!strcmp(str, "rename"))
		return BOESFS_RENAME;
	else if(!strcmp(str, "setattr"))
		return BOESFS_SETATTR;
	else if(!strcmp(str, "getattr"))
		return BOESFS_GETATTR;
	else if(!strcmp(str, "llseek"))
		return BOESFS_LLSEEK;
	else if(!strcmp(str, "iterate"))
		return BOESFS_ITERATE;
	else if(!strcmp(str, "mmap"))
		return BOESFS_MMAP;
	else if(!strcmp(str, "lookup2"))
		return BOESFS_LOOKUP2;
	else if(!strcmp(str, "statfs"))
		return BOESFS_STATFS;
	else if(!strcmp(str, "fsync"))
		return BOESFS_FSYNC;
	else if(!strcmp(str, "default"))
		return BOESFS_DEFAULT;
	else 
		return -1;
		
}

/***
 * 函数作用：将字符串的空格去掉并去掉回车
 * str：要处理的字符串指针
*/
void str_handle(char *str)
{
	size_t i;
	size_t j = 0;
	char result[512];
	memset(result, 0, 512);
	for (i = 0; i < strlen(str); i++)
	{
		if (str[i] == '\n' || str[i] == EOF || str[i] == '\r')
			result[j] = '\0';
		else if (str[i] == ' ' || str[i] == '\t')
			continue;
		else
			result[j] = str[i];
		j ++;
	}
	result[j] = '\0';
	memcpy(str, result, 512);
	return;
}

/***
 * 函数作用：创建一个文件
 * path：要创建的文件路径
 * 返回值：1：失败；0：成功
*/
int create_file(char *path)
{
	FILE *f = fopen(path, "w");
	if(!f)
	{
		fprintf(stderr, "create %s error\n\n", path);
		return 1;
	}
	fclose(f);
	return 0;
}

/***
 * 函数作用：重置进程权限
*/

void reset_caps(void)
{
	struct __user_cap_header_struct cap_hdr;
	cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
	cap_hdr.pid = 0;
	struct __user_cap_data_struct cap_data[_LINUX_CAPABILITY_U32S_3];
	bzero(cap_data, sizeof(cap_data));
	if (capset(&cap_hdr, &cap_data[0]) < 0) {
	        perror("capset()");
	}
}

/***
 * 函数作用：打印一条字符串
 * str：要打印的字符串
*/

void info(const char *str)
{
	printf("%s", str);
}

/***
 * 函数作用：获取一条用户输入的命令
 * dst：返回的命令字符串
*/

void get_command(char *dst)
{
	char buf[PATH_MAX_SUPP] = {0};
	int i = 0;
	while(1)
	{
		info("please input the command to run the elf:\n");
		i = read(STDIN_FILENO, buf, PATH_MAX_SUPP);
		if(i <= 0)
			continue;
		buf[i] = '\0';
		if (strlen(buf) > 127)
		{
			fprintf(stderr, "command too long\n");
			continue;
		}
		break;
	}
	snprintf(dst, strlen(buf), "%s", buf);
	info("Success\n");
}

/***
 * 函数作用：从命令中获取文件名
 * command：传入的命令
 * filename：要返回文件名
 * 返回值：1：失败；0：成功
*/
int get_filename(char *command, char *filename)
{
	char *tmp = NULL;
	char *p;
	//找到命令中的第一个空格
	for (p = command; p < command + strlen(command) && *p != ' '; p++)
		;
	if(*p == ' ')
	{
		tmp = p;
		*p = '\0';
	}
	p --;

	for(; p >= command && *p != '/'; p--)
		;
	p++;

	if(strlen(p) >= PATH_MAX_SUPP)
		return 1;
	memmove(filename, p, strlen(p));
	memset(filename + strlen(p), '\0', PATH_MAX_SUPP - strlen(p));
	if(tmp)
		*tmp = ' ';
	return 0;
}

void get_path(const char *str, char *path)
{
	char dir[PATH_MAX_SUPP] = {0};
	int i = 0;
	while(i <= 0)
	{
		info(str);
		fflush(stdout);
		i = read(STDIN_FILENO, dir, PATH_MAX_SUPP);
		dir[i - 1] = '\0';
		if (access(dir, R_OK))
		{
			fprintf(stderr, "File system dir %s is not accessible: %s!\n", dir, strerror(errno));
			i = 0;
		}
	}
	memcpy(path, dir, PATH_MAX_SUPP);
	info("Success\n");
	return;
}

/***
 * 函数作用：打印文件的所有内容
 * file：文件的路径
*/
void dump_file(char *file)
{
	char buf[512];
	FILE *f = fopen(file,"r");
	if(!f)
	{
		fprintf(stderr, "open %s error\n\n", file);
		return ;
	}
	while(1)
	{
		fgets(buf, 511, (FILE*)f);
		if(feof(f))
			break;
		printf("%s", buf);	
	}
	fclose(f);
	return;
}

/***
 * 函数作用：获取用户输入的数字
 * str：用户输入的提示语
 * 返回值：用户输入的数字
*/
int get_num(const char* str)
{
	char buf[5];
	int i;
	while (1)
	{
		memset(buf, 0, 5);
		printf("%s", str);
		fflush(stdout);
		read(STDIN_FILENO, buf, sizeof(buf));
		if(buf[0] > '9' || buf[0] < '0')
			continue;
		if(buf[4] != '\n' && buf[4] != '\0')
			continue;
		for(i = 1; i <= 4; i++)
		{
			if(buf[i] == '\n')
			{
				buf[i] = '\0';
				return atoi(buf);
			}
			if(buf[i] > '9' || buf[i] < '0')
				break;
		}
	}
}

/***
 * 函数作用：获取用户输入的yes和no
 * str：用户输入的提示语
 * 返回值：1：用户输入y/Y/回车；0：用户输入n/N
*/
int get_yn(const char* str)
{
	char buf[2] = {0};
	int result = -1;
	while (result == -1)
	{
		printf("%s", str);
		fflush(stdout);
		read(STDIN_FILENO, buf, sizeof(buf));
		if(buf[1] != '\n' && buf[1] != '\0')
			result = -1;
		else if(buf[0] == 'y' || buf[0] == 'Y' || buf[0] == '\n')
			result = 1;
		else if(buf[0] == 'n' || buf[0] == 'N')
			result = 0;
		else
			result = -1;
	}
	return result;
}

/***
 * 函数作用：获取用户输入的一位数
 * 返回值：用户输入的一位数
*/
int get_choice(void)
{
	char buf[2] = {0};
	printf("Your choice:");
	fflush(stdout);
	read(STDIN_FILENO, buf, sizeof(buf));
	if(buf[1] != '\n' || buf[0] > '9' || buf[0] <= '0')
		return -1;
	else
		return atoi(buf);
}

void menu(void)
{
	info("\n\nWelcome to BoesFS-Agent!\n");
	info("1. set mode\n");
	info("2. set mount point\n");
	info("3. set if show messege\n");
	info("4. set if kill process when having triggered multiple 'denied' events\n");
	info("5. set if use log\n");
	info("6. run a untrusted ELF file backstage\n");
	info("7. dump result\n");
	info("8. reload ACL/RBAC model and rules\n");
	info("9. exit\n");
}

int setgroups_control(pid_t pid, int action)
{
    char path[PATH_MAX];
	const char *cmd;
	int fd;

	if (action < 0 || (size_t) action >= ARRAY_SIZE(setgroups_strings))
		return -1;

	cmd = setgroups_strings[action];

    snprintf(path, PATH_MAX, "/proc/%ld/setgroups", (long) pid);
	fd = open(path, O_WRONLY);
	if (fd < 0) {
		if (errno == ENOENT)
			return -1;
		fprintf(stderr, "cannot open %s: %s!\n",
			path, strerror(errno));
		return -1;
	}

	if (write(fd, cmd, strlen(cmd)) != strlen(cmd)) {
		fprintf(stderr, "write failed %s:%s!\n",
				path, strerror(errno));
		fprintf(stderr, "%s\n", cmd);
		return -1;
	}
	close(fd);
	return 0;
}

int update_map(char *mapping, char *map_file)
{
    int fd, j;
    size_t map_len;     /* Length of 'mapping' */

    /* Replace commas in mapping string with newlines */
    map_len = strlen(mapping);
    for (j = 0; j < map_len; j++)
        if (mapping[j] == ',')
            mapping[j] = '\n';

    fd = open(map_file, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "open %s: %s\n", map_file, strerror(errno));
        return -1;
    }

    if (write(fd, mapping, map_len) != map_len) {
        fprintf(stderr, "write %s: %s\n", map_file, strerror(errno));
        return -1;
    }

    close(fd);
	return 0;
}

int update_uid_map(pid_t pid, int inside_id, int outside_id)
{
    char map_path[PATH_MAX];
    char uid_map[512];
    sprintf(map_path, "/proc/%ld/uid_map", (long) pid);
    sprintf(uid_map, "%ld %ld 1", (long) inside_id, (long) outside_id);
    if (update_map(uid_map, map_path))
		return -1;
	return 0;
}

int update_gid_map(pid_t pid, int inside_id, int outside_id)
{
    char map_path[PATH_MAX];
    char gid_map[512];
    sprintf(map_path, "/proc/%ld/gid_map", (long) pid);
    sprintf(gid_map, "%ld %ld 1", (long) inside_id, (long) outside_id);
    if (update_map(gid_map, map_path))
		return -1;
	return 0;
}

void usage(char *pname)
{
    fprintf(stderr, "Usage: %s [options] app [arg...]\n\n", pname);
    fprintf(stderr, "Executes a shell command under a sandbox.\n\n");
    fprintf(stderr, "Options can be:\n\n");
#define fpe(str) fprintf(stderr, "    %s", str);
    fpe("-d          File system dir to sandbox\n");
    fpe("-u          User-defined mode BPF sandboxing program file path\n");
    fpe("-v          Display verbose messages\n");
    fpe("-k          The number of 'denied' events before kill the process\n");
    fpe("-l          Use log to record the operation on file of the elf\n");
    fpe("\n");
    exit(EXIT_FAILURE);
}

void check_access(char *dir)
{
	if (access(dir, R_OK))
	{
		fprintf(stderr, "File system dir %s is not accessible: %s!\n",
			dir, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void display_creds_and_caps(void)
{
    cap_t caps;
    char *s;

    caps = cap_get_proc();
    if (caps == NULL)
	{
		perror("cap_get_proc");
		return;
	}

    s = cap_to_text(caps, NULL);
    if (s == NULL)
	{
        perror("cap_to_text");
		return;
	}

	printf("Credentials and Capabilities %s\n\n", s);

    cap_free(caps);
    cap_free(s);
}

void print_mount_points(void)
{
	struct mntent *ent;
	FILE *aFile;

	aFile = setmntent("/proc/mounts", "r");
	if (aFile == NULL) {
		perror("setmntent");
		return;
	}

	printf("Mount Points:\n");
	while (NULL != (ent = getmntent(aFile)))
		printf("\t%s %s\n", ent->mnt_fsname, ent->mnt_dir);
	endmntent(aFile);
}

void print_status(const char *msg)
{
	printf("\n%s\n", msg);
	printf("UID: %d EUID: %d\n", getuid(), geteuid());
	printf("PID: %d TID: %ld\n", getpid(), syscall(SYS_gettid));
	print_mount_points();
	display_creds_and_caps();
}