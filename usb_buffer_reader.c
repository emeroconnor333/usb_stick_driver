include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/fs.h>        // For file operations
#include <linux/uaccess.h>   // For copy_to_user, copy_from_user
#include <linux/device.h>    // For class_create, device_create
#include <linux/wait.h>      // For wait queues
#include <linux/sched.h>     // For TASK_INTERRUPTIBLE, TASK_RUNNI>
#include <linux/proc_fs.h>   // For /proc file
#include <linux/seq_file.h>  // For seq_file

#define MAJOR_NUMBER 42 // the answer to life, the universe, and e>
#define BUFFER_SIZE 1024

static char buffer[BUFFER_SIZE];  // Buffer to store data
static int data_available = 0;    // Flag to track if data is avai>
static int is_plugged_in = 0;     // 1 if USB stick is plugged in,>
static DECLARE_WAIT_QUEUE_HEAD(wait_queue); // Declare the wait qu>

static int proc_show(struct seq_file *m, void *v) {
    seq_printf(m, "USB Stick Statistics\n");
    seq_printf(m, "Plugged in: %s\n", (is_plugged_in == 1) ? "Yes">
    seq_printf(m, "Buffer space left: %d bytes\n", BUFFER_SIZE - d>
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

// Define the file operations structure
static int usb_open(struct inode *inode, struct file *file)
{
    // Check if the device is already open by another process
    if (data_available > 0) {
        printk(KERN_WARNING "USB Stick is already in use!\n");
        return -EBUSY;  // Device is busy
    }

    printk(KERN_INFO "USB Stick: Device opened\n");

    // Initialize the buffer and data flag (e.g., reset buffer on >
    memset(buffer, 0, BUFFER_SIZE);  // Clear the buffer
    data_available = 0;  // Reset data availability

    return 0;  // Success
}

static int usb_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "USB Stick: Device closed\n");
    data_available = 0;  // Reset data availability on close
    return 0;
}

// Blocking read function: waits if no data is available
static ssize_t usb_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset) {
   int bytes_to_read;
    
    printk(KERN_INFO "USB Stick: Read called\n");

    // Block until data is available in the buffer
    if (data_available == 0) {
        printk(KERN_INFO "USB Stick: No data, waiting...\n");
        wait_event_interruptible(wait_queue, data_available > 0);
    }

    bytes_to_read = min(len, (size_t)data_available);
    if (copy_to_user(user_buffer, buffer, bytes_to_read)) {
        return -EFAULT;
    }

    data_available = 0; // Mark buffer as empty after reading

    return bytes_to_read;
}

// Blocking write function: waits if buffer is full
static ssize_t usb_write(struct file *file, const char __user *use>
    int bytes_to_write;

    printk(KERN_INFO "USB Stick: Write called\n");

    // Block until space is available in the buffer
    if (data_available == BUFFER_SIZE) {
        printk(KERN_INFO "USB Stick: Buffer full, waiting...\n");
        wait_event_interruptible(wait_queue, data_available < BUFF>
    }

    bytes_to_write = min(len, sizeof(buffer) - data_available);
   if (copy_from_user(buffer + data_available, user_buffer, bytes>
        return -EFAULT;
    }

    data_available += bytes_to_write;  // Update data available in>

    // Wake up any waiting readers
    wake_up_interruptible(&wait_queue);

    return bytes_to_write;
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
static int usb_probe(struct usb_interface *interface, const struct>
    printk(KERN_INFO "USB Stick inserted! Vendor: 0x%X, Product: 0>
           id->idVendor, id->idProduct);
    is_plugged_in = 1; // USB stick is plugged in
    return 0;
}

// Disconnect function: called when USB device removed
static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "USB Stick removed!\n");
    is_plugged_in = 0;
}

// Declare the USB driver
static struct usb_device_id usb_table[] = {
    { USB_DEVICE(0x06, 0x50) },  // Vendor: 0x06, Product: 0x50 fo>
    { } // Terminating entry
};
MODULE_DEVICE_TABLE(usb, usb_table);

// Declare the USB driver
static struct usb_driver usb_driver = {
    .name = "usb_stick_driver",
    .id_table = usb_table,
    .probe = usb_probe,  // Placeholder probe function
    .disconnect = usb_disconnect,  // Placeholder disconnect funct>
};

// Declare the device class
static struct class *usb_class;

// init function: Register the character device, create a device c>
static int __init usb_init(void)
{
    int result_char;

    // Register the character device
    result_char = register_chrdev(MAJOR_NUMBER, "usb_stick", &fops>
    if (result_char < 0) {
        pr_err("Failed to register character device\n");
        return result_char;
    }
    printk(KERN_INFO "Character device registered with major numbe>
    // Create the device file under /dev
    device_create(usb_class, NULL, MKDEV(MAJOR_NUMBER, 0), NULL, ">

    // Create the /proc file
    if (!proc_create("usb_stats", 0444, NULL, &proc_fops)) {
        pr_err("Failed to create /proc/usb_stats\n");
        return -ENOMEM;
    }

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
    printk(KERN_INFO "USB driver for %s deregistered.\n", usb_driv>

    // Destroy the device file under /dev
    device_destroy(usb_class, MKDEV(MAJOR_NUMBER, 0));  // Removes>
    printk(KERN_INFO "USB device file removed.\n");

    // Destroy the device class
    class_destroy(usb_class);  // Removes the class from /sys/clas>
    printk(KERN_INFO "USB device class destroyed.\n");

    // Remove the /proc file
   remove_proc_entry("usb_stats", NULL);
    printk(KERN_INFO "/proc/usb_stats file removed.\n");
}


module_init(usb_init);
module_exit(usb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emer, Fionn, Conor");
MODULE_DESCRIPTION("USB Stick Driver");
MODULE_VERSION("1.0");




