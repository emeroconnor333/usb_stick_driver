#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "usb_stick"
#define IOCTL_GET_SHIFT _IOR('u', 1, int)
#define IOCTL_SET_SHIFT _IOW('u', 2, int)

static int shift_amount = 0; // Default shift value
static int major_number;
static struct proc_dir_entry *proc_entry;
static char device_buffer[1024];

// Open function
static int device_open(struct inode *inode, struct file *file) {
    pr_info("USB Stick device opened\n");
    return 0;
}

// Read function
static ssize_t device_read(struct file *file, char __user *buffer, size_t len, loff_t *off>
    return simple_read_from_buffer(buffer, len, offset, device_buffer, strlen(device_buffe>
}

// Write function
static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff>
    if (len > sizeof(device_buffer) - 1) 
        return -EINVAL;
    if (copy_from_user(device_buffer, buffer, len)) 
        return -EFAULT;
    device_buffer[len] = '\0';
    return len;
}

// IOCTL function
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int user_shift;
    switch (cmd) {
        case IOCTL_GET_SHIFT:
  if (copy_to_user((int __user *)arg, &shift_amount, sizeof(shift_amount))) {
                return -EFAULT;
            }
            return 0;
        case IOCTL_SET_SHIFT:
            if (copy_from_user(&user_shift, (int __user *)arg, sizeof(user_shift))) {
                return -EFAULT;
            }
            shift_amount = user_shift;
            pr_info("Caesar cipher shift updated to: %d\n", shift_amount);
            return 0;
        default:
            return -EINVAL;
    }
}

// Proc file read function
static ssize_t proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)>
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "Shift: %d\n", shift_amount);
    return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
};

static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
};

// Module initialization
static int __init usb_stick_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        pr_err("Failed to register character device\n");
 return major_number;
    }

    proc_entry = proc_create("usb_shift", 0666, NULL, &proc_fops);
    if (!proc_entry) {
        unregister_chrdev(major_number, DEVICE_NAME);
        pr_err("Failed to create /proc/usb_shift\n");
        return -ENOMEM;
    }

    pr_info("USB Stick Driver Loaded (Major: %d), Initial Shift: %d\n", major_number, shif>
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
MODULE_AUTHOR("Conor, Fionn, Emer");



