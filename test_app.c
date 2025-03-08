#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("/proc/usb_stats", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open /proc/usb_stats");
        return 1;
    }

    char buffer[256];
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    close(fd);
    return 0;
}
