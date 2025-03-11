#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/proc_fs.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "usb_stick"
#define IOCTL_GET_SHIFT _IOR('u', 1, int)

static int shift_amount;
static int major_number;
static struct proc_dir_entry *proc_entry;
static char device_buffer[1024];

// Function to generate a random shift amount
static void generate_shift(void) {
    get_random_bytes(&shift_amount, sizeof(int));
    shift_amount = (shift_amount % 25) + 1; // Ensure shift is between 1-25
}

// Open function
static int device_open(struct inode *inode, struct file *file) {
    pr_info("USB Stick device opened\n");
    return 0;
}

// Read function
static ssize_t device_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    return simple_read_from_buffer(buffer, len, offset, device_buffer, strlen(device_buffer));
}

// Write function
static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    if (len > sizeof(device_buffer) - 1) 
        return -EINVAL;
    if (copy_from_user(device_buffer, buffer, len)) 
        return -EFAULT;
    device_buffer[len] = '\0';
    return len;
}

// IOCTL function to return the shift amount
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    if (cmd == IOCTL_GET_SHIFT) {
        if (copy_to_user((int __user *)arg, &shift_amount, sizeof(shift_amount))) {
            return -EFAULT;
        }
        return 0;
    }
    return -EINVAL;
}

// Proc file read function to display shift amount
static ssize_t proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "Shift: %d\n", shift_amount);
    return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
    .open = device_open,  // Added open function
};

static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
};

// Module initialization
static int __init usb_stick_init(void) {
    generate_shift();

    // Dynamically register character device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        pr_err("Failed to register character device\n");
        return major_number;
    }

    // Create proc entry
    proc_entry = proc_create("usb_shift", 0666, NULL, &proc_fops);
    if (!proc_entry) {
        unregister_chrdev(major_number, DEVICE_NAME);
        pr_err("Failed to create /proc/usb_shift\n");
        return -ENOMEM;
    }

    pr_info("USB Stick Driver Loaded with shift %d (Major: %d)\n", shift_amount, major_number);
    return 0;
}

// Module cleanup
static void __exit usb_stick_exit(void) {
    remove_proc_entry("usb_shift", NULL);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("USB Stick Driver Unloaded\n");
}

module_init(usb_stick_init);
module_exit(usb_stick_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("USB Stick Driver with Caesar Cipher Shift");
MODULE_AUTHOR("Conor Lydon");
