#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

// Global variables for statistics
static int is_plugged_in = 0; // 1 if USB device is plugged in, 0 otherwise
static int buffer_space_left = 1024; // Example buffer space (in bytes)

// /proc file show function
static int proc_show(struct seq_file *m, void *v) {
    seq_printf(m, "USB Device Statistics\n");
    seq_printf(m, "Plugged in: %d\n", is_plugged_in);
    seq_printf(m, "Buffer space left: %d bytes\n", buffer_space_left);
    return 0;
}

// /proc file open function
static int proc_open(struct inode *inode, struct file *file) {
    return single_open(file, proc_show, NULL);
}

// File operations for /proc file
static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// USB probe function
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    printk(KERN_INFO "USB device detected: Vendor ID = 0x%04X, Product ID = 0x%04X\n",
           id->idVendor, id->idProduct);
    is_plugged_in = 1;
    printk(KERN_INFO "USB device connected\n");
    return 0;
}

// USB disconnect function
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

// Global variables for statistics
static int is_plugged_in = 0; // 1 if USB device is plugged in, 0 otherwise
static int buffer_space_left = 1024; // Example buffer space (in bytes)

// /proc file show function
static int proc_show(struct seq_file *m, void *v) {
    seq_printf(m, "USB Device Statistics\n");
    seq_printf(m, "Plugged in: %d\n", is_plugged_in);
    seq_printf(m, "Buffer space left: %d bytes\n", buffer_space_left);
    return 0;
}

// /proc file open function
static int proc_open(struct inode *inode, struct file *file) {
    return single_open(file, proc_show, NULL);
}

// File operations for /proc file
static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// USB probe function
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    printk(KERN_INFO "USB device detected: Vendor ID = 0x%04X, Product ID = 0x%04X\n",
           id->idVendor, id->idProduct);
    is_plugged_in = 1;
    printk(KERN_INFO "USB device connected\n");
    return 0;
}

// USB disconnect function
static void usb_disconnect(struct usb_interface *interface) {
    is_plugged_in = 0;
    printk(KERN_INFO "USB device disconnected\n");
}

// USB device table
static struct usb_device_id usb_table[] = {
    { USB_DEVICE(0xabcd, 0x1234) }, // Replace with your USB stick's VID and PID
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, 6, 80) }, // Match Mass Storage devices
    { } // Terminating entry
};
MODULE_DEVICE_TABLE(usb, usb_table);

// USB driver structure
static struct usb_driver usb_driver = {
    .name = "usb_device_driver",
    .id_table = usb_table,
    .probe = usb_probe,
    .disconnect = usb_disconnect,
};

// Driver initialization
static int __init usb_driver_init(void) {
    // Create the /proc file
    if (!proc_create("usb_stats", 0444, NULL, &proc_fops)) {
        printk(KERN_ERR "Failed to create /proc/usb_stats\n");
        return -ENOMEM;
  }
    usb_register(&usb_driver);
    printk(KERN_INFO "USB driver loaded\n");
    return 0;
}

// Driver cleanup
static void __exit usb_driver_exit(void) {
    remove_proc_entry("usb_stats", NULL);
    usb_deregister(&usb_driver);
    printk(KERN_INFO "USB driver unloaded\n");
}

module_init(usb_driver_init);
module_exit(usb_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("USB Device Driver with /proc File");
