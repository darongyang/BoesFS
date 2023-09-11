MODULE_NAME := mid_test
MODULE_CFILES += main.c \
				supporter.c \

MODULE_OBJS := $(MODULE_CFILES:.c=.o)
$(MODULE_NAME)-objs := $(MODULE_OBJS)
obj-m += $(MODULE_NAME).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean