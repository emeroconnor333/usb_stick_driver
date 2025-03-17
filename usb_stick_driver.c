#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>      // for the usb driver
#include <linux/fs.h>       // for file operations (like open, close, read)
#include <linux/uaccess.h>  // for copy_to_user, copy_from_user
#include <linux/device.h>    // for class_create, device_create
#include <linux/wait.h>      // for wait queues
#include <linux/sched.h>     // linux kernel scheduler
#include <linux/proc_fs.h>   // for /proc file
#include <linux/seq_file.h>  // used for /proc file
#include <linux/ioctl.h>     // for ioctls

#define MAJOR_NUMBER 42 // the answer to life, the universe, and everything
#define BUFFER_SIZE 1024
// ioctl commands
#define IOCTL_GET_SHIFT _IOR('u', 1, int)  // get the shift value
#define IOCTL_SET_SHIFT _IOW('u', 2, int)  // set the shift value
#define IOCTL_GET_PLUGGED_STATUS _IOR('u', 3, int) // check if usb is plugged in

static char buffer[BUFFER_SIZE];
static int data_available = 0;    // flag to track if something is in buffer
static int is_plugged_in = 0;     // 1 if usb is plugged in, 0 not
static DECLARE_WAIT_QUEUE_HEAD(wait_queue); // wait queue
static int shift_amount = 0; // shift value

// ioctl function (get shift, set shift, usb plugged in)
static long usb_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int user_shift; // shift amount entered by user

    switch (cmd) {
        case IOCTL_GET_SHIFT:
            // shift amount is copied to user space
            if (copy_to_user((int __user *)arg, &shift_amount, sizeof(shift_amount))) {
                pr_err("Failed to copy shift value to user\n");
                return -EFAULT;
            }
            return 0;

        case IOCTL_SET_SHIFT:
            // copy shift amount from user space
            if (copy_from_user(&user_shift, (int __user *)arg, sizeof(user_shift))) {
                pr_err("Failed to copy shift value from user\n");
                return -EFAULT;
            }
            shift_amount = user_shift; // update shift
            pr_info("Caesar cipher shift updated to: %d\n", shift_amount);
            return 0;

        case IOCTL_GET_PLUGGED_STATUS:
            // copy whether usb is plugged in to user space
            if (copy_to_user((int __user *)arg, &is_plugged_in, sizeof(is_plugged_in))) {
                pr_err("Failed to copy plugged status to user\n");
                return -EFAULT;
            }
            return 0;

        default:
            pr_err("Invalid ioctl command\n");
            return -EINVAL;
    }
}

// /proc file
static int proc_show(struct seq_file *m, void *v) {
    seq_printf(m, " __ __  _____ ____        _____ ______   ____  ______  _____\n");
    seq_printf(m, "|  |  |/ ___/|    \\      / ___/|      | /    ||      |/ ___/\n");
    seq_printf(m, "|  |  (   \\_ |  o  )    (   \\_ |      ||  o  ||      (   \\_ \n");
    seq_printf(m, "|  |  |\\__  ||     |     \\__  ||_|  |_||     ||_|  |_|\\" "__  |\n");
    seq_printf(m, "|  :  |/  \\ ||  O  |     /  \\ |  |  |  |  _  |  |  |  /  \\ |\n");
    seq_printf(m, "|     |\\    ||     |     \\    |  |  |  |  |  |  |  |  \\    |\n");
    seq_printf(m, " \\__,_| \\___||_____|      \\___|  |__|  |__|__|  |__|   \\___|\n");
    seq_printf(m, "\n");
    seq_printf(m, "Plugged in: %s\n", (is_plugged_in == 1) ? "Yes" : "No");
    seq_printf(m, "Buffer space left: %d bytes\n", BUFFER_SIZE - data_available);
    seq_printf(m, "Current shift value: %d\n", shift_amount);
    return 0;
}

static int proc_open(struct inode *inode, struct file *file) {
    return single_open(file, proc_show, NULL);
}

// defining /proc file operations
static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// called when the usb device file is opened
static int usb_open(struct inode *inode, struct file *file)
{
    // checking if the device is already open by another process
    if (data_available > 0) {
        printk(KERN_WARNING "USB Stick is already being used.\n");
        return -EBUSY;  // device is busy
    }

    printk(KERN_INFO "USB Stick: Device opened\n");

    memset(buffer, 0, BUFFER_SIZE);  // clear the buffer
    data_available = 0;  // reset data availabe

    return 0; 
}

// usb driver being closed, clean up
static int usb_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "USB Stick: Device closed\n");
    data_available = 0;  // reset data availability
    return 0;
}

// blocking read function, waits if no data is available
static ssize_t usb_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset) {
    int bytes_to_read;
    
    printk(KERN_INFO "USB Stick: Read called\n");

    // block until data in buffer
    if (data_available == 0) {
        printk(KERN_INFO "USB Stick: No data, waiting...\n");
        wait_event_interruptible(wait_queue, data_available > 0);
    }

    bytes_to_read = min(len, (size_t)data_available);
    if (copy_to_user(user_buffer, buffer, bytes_to_read)) {
        return -EFAULT;
    }

    data_available = 0; // mark buffer as empty after reading

    return bytes_to_read;
}

// blocking write function, waits if the buffer is full
static ssize_t usb_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *offset) {
    int bytes_to_write;

    printk(KERN_INFO "USB Stick: Write called\n");

    // block until space is available in the buffer
    if (data_available == BUFFER_SIZE) {
        printk(KERN_INFO "USB Stick: Buffer full, waiting...\n");
        wait_event_interruptible(wait_queue, data_available < BUFFER_SIZE);
    }

    bytes_to_write = min(len, sizeof(buffer) - data_available);
    if (copy_from_user(buffer + data_available, user_buffer, bytes_to_write)) {
        return -EFAULT;
    }

    data_available += bytes_to_write;  // update data available in the buffer

    // wake up any waiting readers
    wake_up_interruptible(&wait_queue);

    return bytes_to_write;
}

// defining file operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = usb_open,
    .release = usb_release,
    .read = usb_read,
    .write = usb_write,
    .unlocked_ioctl = usb_ioctl,
};

// probe functio- called when USB device inserted
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    printk(KERN_INFO "USB Stick inserted! Vendor: 0x%X, Product: 0x%X\n",
           id->idVendor, id->idProduct);
    is_plugged_in = 1; // USB stick is plugged in
    return 0;
}

// disconnect function- called when USB device removed
static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "USB Stick removed!\n");
    is_plugged_in = 0;
}

// usb device id table
static struct usb_device_id usb_table[] = {
    { USB_DEVICE(0x06, 0x50) },  // vendor is 0x06, product is 0x50
    { } // Terminating entry
};
MODULE_DEVICE_TABLE(usb, usb_table);

// usb driver structure
static struct usb_driver usb_driver = {
    .name = "usb_stick_driver",
    .id_table = usb_table,
    .probe = usb_probe, 
    .disconnect = usb_disconnect,  
};

// declare the device class
static struct class *usb_class;

// init function: Register the character device, create a device class, and register the USB driver
static int __init usb_init(void)
{
    int result_char;

    // register the character device
    result_char = register_chrdev(MAJOR_NUMBER, "usb_stick", &fops);
    if (result_char < 0) {
        pr_err("Failed to register character device\n");
        return result_char;
    }
    printk(KERN_INFO "Character device registered with major number %d\n", MAJOR_NUMBER);

    // create a device class
    usb_class = class_create("usb_stick_class");
    if (IS_ERR(usb_class)) {
        pr_err("Failed to create class\n");
        unregister_chrdev(MAJOR_NUMBER, "usb_stick"); // deregister the char device
        return PTR_ERR(usb_class);
    }

    // register the USB driver
    if (usb_register(&usb_driver) < 0) {
        pr_err("usb_register failed for the %s driver\n", usb_driver.name);
        unregister_chrdev(MAJOR_NUMBER, "usb_stick");
        class_destroy(usb_class);  // clean up the class if usb registration fails
        return -1;
    }
    printk(KERN_INFO "USB driver for %s initialised successfully.\n", usb_driver.name);

    // create the device file under /dev
    device_create(usb_class, NULL, MKDEV(MAJOR_NUMBER, 0), NULL, "usb_stick");

    // create the /proc file
    if (!proc_create("usb_stats", 0444, NULL, &proc_fops)) {
        pr_err("Failed to create /proc/usb_stats\n");
        return -ENOMEM;
    }

    return 0;
}


// exit the driver and deregister the character device
static void __exit usb_exit(void)
{
    // deregister the character device
    unregister_chrdev(MAJOR_NUMBER, "usb_stick");
    printk(KERN_INFO "Character device deregistered.\n");

    // deregister the usb driver
    usb_deregister(&usb_driver);
    printk(KERN_INFO "USB driver for %s deregistered.\n", usb_driver.name);

    // destroy the device file under /dev
    device_destroy(usb_class, MKDEV(MAJOR_NUMBER, 0));  // Removes the device file from /dev
    printk(KERN_INFO "USB device file removed.\n");

    // destroy the device class
    class_destroy(usb_class);  // Removes the class from /sys/class/
    printk(KERN_INFO "USB device class destroyed.\n");

    // remove the /proc file
    remove_proc_entry("usb_stats", NULL);
    printk(KERN_INFO "/proc/usb_stats file removed.\n");
}


module_init(usb_init);
module_exit(usb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emer, Fionn, Conor");
MODULE_DESCRIPTION("USB Stick Driver");
MODULE_VERSION("1.0");