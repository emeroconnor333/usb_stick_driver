#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/fs.h>        // For file operations
#include <linux/uaccess.h>   // For copy_to_user, copy_from_user
#include <linux/device.h>    // For class_create, device_create

#define MAJOR_NUMBER 42 // MAJOR_NUMBER number for the character device

// Define the file operations structure
static int usb_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "USB Stick: Device opened\n");
    return 0;
}

static int usb_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "USB Stick: Device closed\n");
    return 0;
}

static ssize_t usb_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset)
{
    printk(KERN_INFO "USB Stick: Read called\n");
    return 0;  // Placeholder for reading logic
}

static ssize_t usb_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *offset)
{
    printk(KERN_INFO "USB Stick: Write called\n");
    return len;  // Placeholder for writing logic
}

// Define the file operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = usb_open,
    .release = usb_release,
    .read = usb_read,
    .write = usb_write,
};

// Declare the USB driver
static struct usb_device_id usb_table[] = {
    { USB_DEVICE(0x06, 0x50) },  // Vendor: 0x06, Product: 0x50 for Mass Storage Class
    { } // Terminating entry
};
MODULE_DEVICE_TABLE(usb, usb_table);

// Declare the USB driver
static struct usb_driver usb_driver = {
    .name = "usb_stick_driver",
    .id_table = usb_table,
    .probe = NULL,  // Placeholder probe function
    .disconnect = NULL,  // Placeholder disconnect function
};

// Declare the device class
static struct class *usb_class;

// init function: Register the character device, create a device class, and register the USB driver
static int __init usb_init(void)
{
    int result_char;

    // Register the character device
    result_char = register_chrdev(MAJOR_NUMBER, "usb_stick", &fops);
    if (result_char < 0) {
        pr_err("Failed to register character device\n");
        return result_char;
    }
    printk(KERN_INFO "Character device registered with major number %d\n", MAJOR_NUMBER);

    // Create a device class
    usb_class = class_create("usb_stick_class");  // Pass only the class name as a string
    if (IS_ERR(usb_class)) {
        pr_err("Failed to create class\n");
        unregister_chrdev(MAJOR_NUMBER, "usb_stick"); // Deregister the char device
        return PTR_ERR(usb_class);
    }

    // Register the USB driver
    if (usb_register(&usb_driver) < 0) {
        pr_err("usb_register failed for the %s driver\n", usb_driver.name);
        unregister_chrdev(MAJOR_NUMBER, "usb_stick");
        class_destroy(usb_class);  // Clean up the class if USB registration fails
        return -1;
    }
    printk(KERN_INFO "USB driver for %s initialised successfully.\n", usb_driver.name);

    // Create the device file under /dev
    device_create(usb_class, NULL, MKDEV(MAJOR_NUMBER, 0), NULL, "usb_stick");

    return 0;
}


// Exit the driver and deregister the character device
static void __exit usb_exit(void)
{
    // Deregister the character device
    unregister_chrdev(MAJOR_NUMBER, "usb_stick");
    printk(KERN_INFO "Character device deregistered.\n");

    // Deregister the USB driver
    usb_deregister(&usb_driver);
    printk(KERN_INFO "USB driver for %s deregistered.\n", usb_driver.name);

    // Destroy the device file under /dev
    device_destroy(usb_class, MKDEV(MAJOR_NUMBER, 0));  // Removes the device file from /dev
    printk(KERN_INFO "USB device file removed.\n");

    // Destroy the device class
    class_destroy(usb_class);  // Removes the class from /sys/class/
    printk(KERN_INFO "USB device class destroyed.\n");
}


module_init(usb_init);
module_exit(usb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("USB Stick Driver");
MODULE_VERSION("1.0");
