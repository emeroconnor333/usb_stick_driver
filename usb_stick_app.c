#include <fcntl.h>    // Provides open(), O_RDWR
#include <unistd.h>   // Provides close(), read(), write()

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <libudev.h>

#define BUFFER_SIZE 1024

char virtual_buffer[BUFFER_SIZE];  // Simulated USB storage
int shift; // Shift value for Caesar cipher
int fd;  //file descriptor
// Function to wait for a USB device to be plugged in
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
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "u>
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

// Function to encrypt text using Caesar cipher
void encrypt(char *text, int shift) {
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] >= 'A' && text[i] <= 'Z') {
            text[i] = 'A' + (text[i] - 'A' + shift) % 26;
        } else if (text[i] >= 'a' && text[i] <= 'z') {
            text[i] = 'a' + (text[i] - 'a' + shift) % 26;
        }
    }
}

// Function to decrypt text using Caesar cipher
void decrypt(char *text, int shift) {
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] >= 'A' && text[i] <= 'Z') {
            text[i] = 'A' + (text[i] - 'A' - shift + 26) % 26;
        } else if (text[i] >= 'a' && text[i] <= 'z') {
            text[i] = 'a' + (text[i] - 'a' - shift + 26) % 26;
        }
    }
}

// Write thread (simulated write to buffer)
void *write_thread(void *arg) {
    char write_buffer[BUFFER_SIZE];

    // Prompt the user for input
    printf("Enter data to write to the virtual USB: ");
    fgets(write_buffer, BUFFER_SIZE, stdin);
    write_buffer[strcspn(write_buffer, "\n")] = '\0'; // Remove ne>

    // Encrypt before writing
    encrypt(write_buffer, shift);

    // Simulate writing by storing in virtual buffer
    strncpy(virtual_buffer, write_buffer, BUFFER_SIZE);

    printf("Written (Encrypted): %s\n", virtual_buffer);
    return NULL;
}

// Read thread (simulated read from buffer)
void *read_thread(void *arg) {
    char read_buffer[BUFFER_SIZE];

    // Simulate reading from virtual buffer
    strncpy(read_buffer, virtual_buffer, BUFFER_SIZE);

   // Decrypt after reading
    decrypt(read_buffer, shift);

    printf("Read (Decrypted): %s\n", read_buffer);
    return NULL;
}

int main() {
    pthread_t writer, reader;
    int choice;

    // Wait for USB device to be plugged in
    wait_for_usb_device();

    printf("Running USB application...\n");

    // Ask the user for a shift value
    printf("Enter the shift amount for the Caesar cipher: ");
    if (scanf("%d", &shift) != 1) {
        printf("Invalid input. Please enter a number.\n");
        return 1;
    }
    getchar(); // Consume newline left in input buffer

fd = open("/dev/usb_stick", O_RDWR);
    if (fd == -1) {
        perror("Failed to open USB stick");
        return 1;
    }
    printf("USB stick opened successfully\n");


    while (1) {
       printf("\nUSB Stick Menu:\n");
        printf("1. Write to USB (Encrypted)\n");
        printf("2. Read from USB (Decrypted)\n");
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
                return 0;
            default:
                printf("Invalid choice. Please enter 1, 2, or 3.\n>
        }
    }
}

