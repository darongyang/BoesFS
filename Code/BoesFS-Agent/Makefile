# kbuild trick to avoid linker error. Can be omitted if a module is built.
obj- := dummy.o

# List of programs to build
hostprogs-y := boesfs

boesfs-objs := \
	src/main.o src/utils.o src/ebpf.o src/libbpf.o src/bpf_load.o src/non_interact.o src/interact.o

# Generate .c files based on kernel source
%.c:
	if [ -f $(objtree)/samples/bpf/libbpf.c ]; then \
		cp $(objtree)/samples/bpf/libbpf.c $(PWD)/src/.; \
	else \
		cp $(objtree)/tools/lib/bpf/bpf.c $(PWD)/src/libbpf.c; \
	fi;

# Tell kbuild to always build the programs
always := $(hostprogs-y)

EXTRA_CFLAGS += -I$(PWD)/include -I$(objtree)/samples/bpf

HOSTCFLAGS += -fPIC -I$(objtree)/usr/include -I$(PWD)/include
HOSTCFLAGS += -I$(objtree)/samples/bpf -I$(objtree)/tools/lib -I$(objtree)/tools/lib/bpf
HOSTCFLAGS_bpf_load.o += -I$(objtree)/usr/include -I$(objtree)/tools/perf -Wno-unused-variable
HOSTCFLAGS_libbpf.o += -I$(objtree)/usr/include -Wno-unused-variable
HOSTLOADLIBES_boesfs += -lelf -lcap -lpthread

TARGET_LIB := libboesfs.so
lib_objs := src/utils.o src/ebpf.o src/libbpf.o src/bpf_load.o

# Trick to allow make to be run from this directory
all:
	$(MAKE) -C /lib/modules/`uname -r`/build $$PWD/
# $(TARGET_LIB): $(lib_objs)
	gcc -shared -fPIC -o $(TARGET_LIB) $(lib_objs)

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build M=$$PWD clean
	rm -f src/*.o xdp/*.o *.a *.so boesfs
	rm -f include/bpf_helpers.h include/bpf_load.h include/libbpf.h
	rm -f src/libbpf.c
