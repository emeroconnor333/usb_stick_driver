#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main() {
    int fd;
    ssize_t bytes_written, bytes_read;
    char write_buffer[] = "Hello, USB Stick!";
    char read_buffer[BUFFER_SIZE];

    // Open the USB stick device file
    fd = open("/dev/usb_stick", O_RDWR);
    if (fd == -1) {
        perror("Failed to open USB stick");
        return 1;
    }

    printf("USB stick opened successfully\n");

    // Write data to the device
    bytes_written = write(fd, write_buffer, strlen(write_buffer));
    if (bytes_written == -1) {
        perror("Failed to write to USB stick");
        close(fd);
        return 1;
    }

    printf("Written %zd bytes to USB stick: %s\n", bytes_written, write_buffer);

    // Read data from the device
    bytes_read = read(fd, read_buffer, BUFFER_SIZE);
    if (bytes_read == -1) {
        perror("Failed to read from USB stick");
        close(fd);
        return 1;
    }

    // Null-terminate the read buffer
    read_buffer[bytes_read] = '\0';

    printf("Read %zd bytes from USB stick: %s\n", bytes_read, read_buffer);

    // Close the device file
    close(fd);
    return 0;
}
