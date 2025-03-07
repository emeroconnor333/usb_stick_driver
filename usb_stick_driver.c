#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>


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

// Function callbacks
static struct usb_driver usb_driver = {
    .name = "usb_stick_driver",
    .id_table = usb_table,
    .probe = usb_probe,
    .disconnect = usb_disconnect,
};

// init function: register the device
static int __init usb_init(void)
{
    int result = usb_register(&usb_driver);
    if (result < 0) {
        pr_err("usb_register failed for the %s driver. Error number %d\n", usb_driver.name, result);
        return -1;
    }
    pr_info("USB driver for %s initialised successfully.\n", usb_driver.name);
    return 0;
}

// exit function: deregister the device 
static void __exit usb_exit(void)
{
    usb_deregister(&usb_driver);  // deregister the driver
    pr_info("USB driver for %s deregistered.\n", usb_driver.name);
}

// Register the init and exit functions
module_init(usb_init);
module_exit(usb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emer, Conor, Fionn");
MODULE_DESCRIPTION("USB Stick Driver");
MODULE_VERSION("1.0");