#include <fcntl.h>  // file control
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
#define IOCTL_GET_PLUGGED_STATUS _IOR('u', 3, int)

#define PROC_FILE "/proc/usb_stats"
#define DEVICE_PATH "/dev/usb_stick"

int fd;

int wait_for_usb_device() {
    int fd;
    int plugged_in_status = 0;

    // open the device file
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd == -1) {
        perror("Failed to open USB stick device");
        return 1;
    }

    // loop until the USB stick is plugged in
    while (1) {
        // find if usb plugged in
        if (ioctl(fd, IOCTL_GET_PLUGGED_STATUS, &plugged_in_status) == -1) {
            perror("ioctl failed");
            close(fd);
            return 1;
        }

        // break from loop when plugged in
        if (plugged_in_status == 1) {
            printf("USB Stick Plugged In\n");
            break;
        }

        // sleep then check again 
        usleep(500000);
    }

    // close the device file
    close(fd);
    return 0; 
}

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

void *set_cipher_shift(void *arg) {
    int shift;
    printf("Enter Caesar cipher shift value (1-25): ");
    scanf("%d", &shift);
    getchar(); 
    
    if (ioctl(fd, IOCTL_SET_SHIFT, &shift) == -1) {
        perror("Failed to set shift value");
    } else {
        printf("Shift value set to %d\n", shift);
    }
    return NULL;  
}

void *get_cipher_shift(void *arg) {
    int shift;
    if (ioctl(fd, IOCTL_GET_SHIFT, &shift) == -1) {
        perror("Failed to get shift value");
    } else {
        printf("Current shift value is %d\n", shift);
    }
    return NULL;  
}

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

int main() {
    // wait for the usb stick to be plugged in
    if (wait_for_usb_device() == 0) {
        fd = open("/dev/usb_stick", O_RDWR);
        if (fd == -1) {
            perror("Failed to open USB stick device");
            return 1;
        }

        int choice;
        pthread_t thread;
        while (1) {
            system("clear"); 
            printf(" __ __  _____ ____       ___ ___   ____  ____    ____   ____    ___  ____  \n");
            printf("|  |  |/ ___/|    \\     |   |   | /    ||    \\  /    | /    |  /  _]|    \\ \n");
            printf("|  |  (   \\_ |  o  )    | _   _ ||  o  ||  _  ||  o  ||   __| /  [_ |  D  )\n");
            printf("|  |  |\\__  ||     |    |  \\_/  ||     ||  |  ||     ||  |  ||    _]|    / \n");
            printf("|  :  |/  \\ ||  O  |    |   |   ||  _  ||  |  ||  _  ||  |_ ||   [_ |    \\ \n");
            printf("|     |\\    ||     |    |   |   ||  |  ||  |  ||  |  ||     ||     ||  .  \\ \n");
            printf(" \\__,_| \\___||_____|    |___|___||__|__||__|__||__|__||___,_||_____||__|\\_| \n\n");
            printf("1. Set Cipher Shift\n");
            printf("2. Get Cipher Shift\n");
            printf("3. Write Data\n");
            printf("4. Read Data\n");
            printf("5. Encrypt Data\n");
            printf("6. Decrypt Data\n");
            printf("7. Exit\n");
            printf("Enter choice: ");
            scanf("%d", &choice);
            getchar();

            switch (choice) {
                case 1: pthread_create(&thread, NULL, set_cipher_shift, NULL); break;
                case 2: pthread_create(&thread, NULL, get_cipher_shift, NULL); break;
                case 3: pthread_create(&thread, NULL, write_data, NULL); break;
                case 4: pthread_create(&thread, NULL, read_data, NULL); break;
                case 5: pthread_create(&thread, NULL, encrypt_data, NULL); break;
                case 6: pthread_create(&thread, NULL, decrypt_data, NULL); break;
                case 7: close(fd); return 0;
                default: printf("Invalid choice. Try again.\n");
            }
            pthread_join(thread, NULL);
            getchar();  // waits for the user to press enter
            
        }
    } else {
        printf("Failed to detect USB device.\n");
    }

    return 0;
}
