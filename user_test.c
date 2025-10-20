// Purpose: Minimal userspace test app for the driver

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int fd = open("/dev/simple_char", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    const char *msg = "hello kernel";
    ssize_t written = write(fd, msg, strlen(msg));
    if (written < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    lseek(fd, 0, SEEK_SET);
    char buf[64] = {0};
    ssize_t readn = read(fd, buf, sizeof(buf)-1);
    if (readn < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    printf("read back: %s\n", buf);
    close(fd);
    return 0;
}
