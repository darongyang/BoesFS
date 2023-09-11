#ifndef __UTILS_H__
#define __UTILS_H__

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

#define PATH_MAX_SUPP 256
#define STACK_SIZE (1024 * 1024)
enum {
	SETGROUPS_NONE = -1,
	SETGROUPS_DENY = 0,
	SETGROUPS_ALLOW = 1,
};

#define ROOT_UID "0"
#define ROOT_GID "0"

uint32_t hash_uint32(uint32_t integer);
uint32_t hash_str(char * str);
uint32_t s2d(char *str);
int check_op(char *str);
void str_handle(char *str);
int create_file(char *path);
void reset_caps(void);
void info(const char *str);
void get_command(char *dst);
int get_filename(char *command, char *filename);
void get_path(const char *str, char *path);
int get_program_dir(char *dir);
void dump_file(char *file);
int get_num(const char* str);
int get_yn(const char* str);
int get_choice(void);
void menu(void);
int setgroups_control(pid_t pid, int action);
int update_uid_map(pid_t pid, int inside_id, int outside_id);
int update_gid_map(pid_t pid, int inside_id, int outside_id);
int update_map(char *mapping, char *map_file);
void usage(char *pname);
void check_access(char *dir);
void print_status(const char *msg);
void display_creds_and_caps(void);
void print_mount_points(void);

#endif /* __UTILS_H__ */