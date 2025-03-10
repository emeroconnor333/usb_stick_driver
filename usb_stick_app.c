#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int fd;

void *write_thread(void *arg) {
    char write_buffer[BUFFER_SIZE];
    ssize_t bytes_written;

  
        // Prompt the user for input
        printf("Enter data to write to the USB stick: ");
        fgets(write_buffer, BUFFER_SIZE, stdin);  // Read user input

        // Remove newline character if it exists
        write_buffer[strcspn(write_buffer, "\n")] = '\0';

        // Write data to the device
        bytes_written = write(fd, write_buffer, strlen(write_buffer));
        if (bytes_written == -1) {
            perror("Failed to write to USB stick");
            return NULL;
        }

        printf("Written %zd bytes: %s\n", bytes_written, write_buffer);
        return NULL;
    }


void *read_thread(void *arg) {
    char read_buffer[BUFFER_SIZE];
    ssize_t bytes_read;

        bytes_read = read(fd, read_buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Failed to read from USB stick");
            return NULL;
        }

        // Null-terminate the read buffer
        read_buffer[bytes_read] = '\0';
        printf("Read %zd bytes: %s\n", bytes_read, read_buffer);
        return NULL;
    }


int main() {
    pthread_t writer, reader;
    int choice;

    // Open the USB stick device file
    fd = open("/dev/usb_stick", O_RDWR);
    if (fd == -1) {
        perror("Failed to open USB stick");
        return 1;
    }

    printf("USB stick opened successfully\n");

       while (1) {
        printf("\nUSB Stick Menu:\n");
        printf("1. Write to USB\n");
        printf("2. Read from USB\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        getchar(); // Consume newline left in input buffer

        switch (choice) {
            case 1:
                pthread_create(&writer, NULL, write_thread, NULL);
                pthread_join(writer, NULL);
                break;
            case 2:
                pthread_create(&reader, NULL, read_thread, NULL);
                pthread_join(reader, NULL);
                break;
            case 3:
                printf("Exiting...\n");
                close(fd);
                return 0;
            default:
                printf("Invalid choice. Please enter 1, 2, or 3.\n");
        }
    }
 
}
