#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define USB_STORAGE_CLASS 0x08  // Mass Storage Class

static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    printk(KERN_INFO "USB Stick inserted! Vendor: 0x%X, Product: 0x%X\n",
           id->idVendor, id->idProduct);
    return 0;
}

static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "USB Stick removed!\n");
}

// Match only mass storage devices
static struct usb_device_id usb_table[] = {
    { USB_INTERFACE_INFO(USB_STORAGE_CLASS, 0x06, 0x50) }, // Bulk-Only Transport (BOT)
    { } // End of list
};
MODULE_DEVICE_TABLE(usb, usb_table);

static struct usb_driver usb_driver = {
    .name = "usb_stick_driver",
    .id_table = usb_table,
    .probe = usb_probe,
    .disconnect = usb_disconnect,
};

module_usb_driver(usb_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple USB Stick Driver");
