#ifndef __INTERACT_H__
#define __INTERACT_H__

typedef struct{
    pid_t pid;
    pid_t exec_pid;
    int fd;
    struct pid_list *next;
} pid_list;


void interactive(void);

#endif /* __INTERACT_H__ */