obj-m += boesfs.o
boesfs-objs := supporter.o log.o rlimit.o dentry.o file.o prog.o kill.o parse.o get_path.o ebpf_helper.o inode.o main.o super.o lookup.o mmap.o dir_tree.o 

KBUILD_CFLAGS_MODULE += -g

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean