#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct ioctl_arg {
    unsigned int val;
};

#define IOC_MAGIC '\x66'
#define IOCTL_VALSET _IOW(IOC_MAGIC, 0, struct ioctl_arg)
#define IOCTL_VALGET _IOR(IOC_MAGIC, 1, struct ioctl_arg)

int main()
{
    int fd = open("/sys/module/ioctl", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct ioctl_arg arg;
    arg.val = 0x55;
    ioctl(fd, IOCTL_VALSET, &arg);

    ioctl(fd, IOCTL_VALGET, &arg);
    printf("val = 0x%x\n", arg.val);

    close(fd);
    return 0;
}
