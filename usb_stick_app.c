#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    // Open the USB stick device file
    int fd = open("/dev/usb_stick", O_RDWR);
    if (fd == -1) {
        perror("Failed to open USB stick");
        return 1;
    }

    printf("USB stick opened successfully\n");

    // Close the device file
    close(fd);
    return 0;
}
