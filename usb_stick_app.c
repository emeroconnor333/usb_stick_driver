#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <libudev.h>

#define BUFFER_SIZE 1024
#define IOCTL_GET_SHIFT _IOR('u', 1, int)
#define IOCTL_SET_SHIFT _IOW('u', 2, int)

int fd;

// Function to perform Caesar cipher encryption/decryption
void caesar_cipher(char *text, int shift, int decrypt) {
    for (int i = 0; text[i] != '\0'; i++) {
        char ch = text[i];
        if (ch >= 'A' && ch <= 'Z') {
            text[i] = ((ch - 'A' + (decrypt ? -shift : shift) + 26) % 26) + 'A';
        } else if (ch >= 'a' && ch <= 'z') {
            text[i] = ((ch - 'a' + (decrypt ? -shift : shift) + 26) % 26) + 'a';
        }
    }
}

// Function to set cipher shift
void *set_cipher_shift(void *arg) {
    int shift;
    printf("Enter Caesar cipher shift value (1-25): ");
    scanf("%d", &shift);
    getchar(); // Consume newline
    
    if (ioctl(fd, IOCTL_SET_SHIFT, &shift) == -1) {
        perror("Failed to set shift value");
    } else {
        printf("Shift value set to %d\n", shift);
    }
    return NULL;
}

void wait_for_usb_device() {
    struct udev *udev;
    struct udev_monitor *mon;
    struct udev_device *dev;

    // Create udev object
    udev = udev_new();
    if (!udev) {
        printf("Cannot create udev\n");
        return;
    }

    // Create monitor for USB events
    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
    udev_monitor_enable_receiving(mon);

    printf("Waiting for USB device...\n");

    // Loop until a device is plugged in
    while (1) {
        dev = udev_monitor_receive_device(mon);
        if (dev) {
            const char *action = udev_device_get_action(dev);
            if (action && strcmp(action, "add") == 0) {
                printf("USB device plugged in!\n");
                udev_device_unref(dev);
                break;
            }
            udev_device_unref(dev);
        }
    }

    // Cleanup
    udev_monitor_unref(mon);
    udev_unref(udev);
}

// Function to write data
void *write_data(void *arg) {
    char write_buffer[BUFFER_SIZE];
    printf("Enter data to write: ");
    fgets(write_buffer, BUFFER_SIZE, stdin);
    write_buffer[strcspn(write_buffer, "\n")] = '\0';
    
    ssize_t bytes_written = write(fd, write_buffer, strlen(write_buffer));
    if (bytes_written == -1) {
        perror("Write failed");
    } else {
        printf("Data written successfully (%zd bytes).\n", bytes_written);
    }
    return NULL;
}

// Function to read data
void *read_data(void *arg) {
    char read_buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd, read_buffer, BUFFER_SIZE);
    if (bytes_read == -1) {
        perror("Read failed");
        return NULL;
    }
    read_buffer[bytes_read] = '\0';
    printf("Data read (%zd bytes): %s\n", bytes_read, read_buffer);
    return NULL;
}

// Function to encrypt data
void *encrypt_data(void *arg) {
    int shift;
    if (ioctl(fd, IOCTL_GET_SHIFT, &shift) == -1) {
        perror("Failed to get shift value");
        return NULL;
    }
    
    char buffer[BUFFER_SIZE];
    printf("Enter text to encrypt: ");
    fgets(buffer, BUFFER_SIZE, stdin); buffer[strcspn(buffer, "\n")] = '\0';
    caesar_cipher(buffer, shift, 0);
    printf("Encrypted text: %s\n", buffer);
    return NULL;
}

// Function to decrypt data
void *decrypt_data(void *arg) {
    int shift;
    if (ioctl(fd, IOCTL_GET_SHIFT, &shift) == -1) {
        perror("Failed to get shift value");
        return NULL;
    }
    
    char buffer[BUFFER_SIZE];
    printf("Enter text to decrypt: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    caesar_cipher(buffer, shift, 1);
    printf("Decrypted text: %s\n", buffer);
    return NULL;
}

// Main menu
int main() {
    wait_for_usb_device();
    fd = open("/dev/usb_stick", O_RDWR);
    if (fd == -1) {
        perror("Failed to open USB stick");
        return 1;
    }
    
    int choice;
    pthread_t thread;
    while (1) {
        printf("\nUSB Stick Menu:\n");
        printf("1. Set Cipher Shift\n");
        printf("2. Write Data\n");
        printf("3. Read Data\n");
        printf("4. Encrypt Data\n");
        printf("5. Decrypt Data\n");
        printf("6. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline

        switch (choice) {
            case 1: pthread_create(&thread, NULL, set_cipher_shift, NULL); break;
            case 2: pthread_create(&thread, NULL, write_data, NULL); break;
            case 3: pthread_create(&thread, NULL, read_data, NULL); break;
            case 4: pthread_create(&thread, NULL, encrypt_data, NULL); break;
            case 5: pthread_create(&thread, NULL, decrypt_data, NULL); break;
            case 6: close(fd); return 0;
            default: printf("Invalid choice. Try again.\n");
        }
        pthread_join(thread, NULL);
    }
}
