#obj-m += hello-1.o
#obj-m += hello-2.o
#obj-m += hello-3.o
#obj-m += hello-4.o
#
#obj-m += hello-5.o
#obj-m += startstop.o
#startstop-objs := start.o stop.o

#obj-m += chardev.o
#obj-m += procfs1.o
#obj-m += procfs2.o
#obj-m += procfs3.o
#obj-m += procfs4.o
#obj-m += hello-sysfs.o
#obj-m += ioctl.o
#obj-m += chardev2.o
#obj-m += kbleds.o
#obj-m += vinput.o
obj-m += vkbd.o

ccflags-y += -DDEBUG

PWD := $(CURDIR)

USERSPACEC = clang
USERSPACECFLAGS = -Wall -Wextra -g

all: kernel # userspace

kernel:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

userspace: userspace_ioctl

userspace_ioctl: userspace_ioctl.o
	$(USERSPACEC) $(USERSPACECFLAGS) userspace_ioctl.o -o userspace_ioctl

userspace_ioctl.o: userspace_ioctl.c
	$(USERSPACEC) $(USERSPACECFLAGS) -c userspace_ioctl.c

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f userspace_ioctl userspace_ioctl.o

indent:
	clang-format -i *.[ch]