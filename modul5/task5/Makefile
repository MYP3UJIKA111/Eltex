obj-m += netlink_mod.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
	gcc -o netlink_user netlink_user.c

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f netlink_user