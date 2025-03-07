#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/fs.h>        // For file operations
#include <linux/uaccess.h>   // For copy_to_user, copy_from_user

#define MAJOR 42 // the answer to life, the universe, and everything (and the major number)

// Simple buffer for our device
#define BUFFER_SIZE 1024
static char buffer[BUFFER_SIZE];  // Buffer to store data
static int data_available = 0;    // Flag to track if data is available

// Define the file operations structure
static int usb_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "USB Stick: Device opened\n");
    // Add logic for device opening later
    return 0;
}

static int usb_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "USB Stick: Device closed\n");
    // Add logic for device release later
    return 0;
}

static ssize_t usb_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset)
{
    printk(KERN_INFO "USB Stick: Read called\n");
    // Add logic to read from the buffer later
    return 0;  // Return number of bytes read (update later)
}

static ssize_t usb_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *offset)
{
    printk(KERN_INFO "USB Stick: Write called\n");
    // Add logic to write to the buffer later
    return len;  // Return number of bytes written (update later)
}

// Define the file operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = usb_open,
    .release = usb_release,
    .read = usb_read,
    .write = usb_write,
};

// Probe function: called when USB device inserted
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    printk(KERN_INFO "USB Stick inserted! Vendor: 0x%X, Product: 0x%X\n",
           id->idVendor, id->idProduct);
    return 0;
}

// Disconnect function: called when USB device removed
static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "USB Stick removed!\n");
}

// Device table: to declare what devices the driver supports
static struct usb_device_id usb_table[] = {
    { USB_DEVICE(0x06, 0x50) },  // vendor: 0x06, product: 0x50 for Mass Storage Class
    { } // terminating entry
};
MODULE_DEVICE_TABLE(usb, usb_table);

// Function callbacks for the USB driver
static struct usb_driver usb_driver = {
    .name = "usb_stick_driver",
    .id_table = usb_table,
    .probe = usb_probe,
    .disconnect = usb_disconnect,
};

// init function: register the device
static int __init usb_init(void)
{
    int result_char = register_chrdev(MAJOR, "usb_stick", &fops);
    if (result_char < 0) {
        pr_err("Failed to register character device\n");
        return result_char;
    }

    int result_usb = usb_register(&usb_driver);
    if (result_usb < 0) {
        pr_err("usb_register failed for the %s driver. Error number %d\n", usb_driver.name, result_usb);
        unregister_chrdev(MAJOR, "usb_stick");  // Deregister the char device
        return -1;
    }
    pr_info("USB driver for %s initialised successfully.\n", usb_driver.name);
    return 0;
}

// exit function: deregister the device 
static void __exit usb_exit(void)
{
    // Deregister the character device
    unregister_chrdev(MAJOR, "usb_stick");
    printk(KERN_INFO "USB Stick driver deregistered.\n");

    // Deregister the USB driver
    usb_deregister(&usb_driver);
    pr_info("USB driver for %s deregistered.\n", usb_driver.name);
}

// Register the init and exit functions
module_init(usb_init);
module_exit(usb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emer, Conor, Fionn");
MODULE_DESCRIPTION("USB Stick Driver with Character Device Interface");
MODULE_VERSION("1.0");
