include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h> // For kmalloc and kfree
#include <linux/string.h>


#define USB_STORAGE_CLASS 0x08  // Mass Storage Class
#define BUFFER_SIZE 1024

static char *local_buffer;

ssize_t store_text(const char *text);


static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    printk(KERN_INFO "USB Stick inserted! Vendor: 0x%X, Product: 0x%X\n",
           id->idVendor, id->idProduct);
    
    // Allocate memory for local buffer
    local_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!local_buffer) {
        printk(KERN_ERR "Failed to allocate memory for local buffer\n");
        return -ENOMEM;
    }
    memset(local_buffer, 0, BUFFER_SIZE);
    printk(KERN_INFO "Local buffer initialized\n");

    // Store a predefined message
    store_text("Hello, world! USB device detected.");

    return 0;
}
tatic void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "USB Stick removed!\n");
    
    // Free allocated buffer memory
    if (local_buffer) {
        kfree(local_buffer);
        printk(KERN_INFO "Local buffer freed\n");
    }
}

// Store text in the local buffer
ssize_t store_text(const char *text) {
    if (!local_buffer) return -ENOMEM;
    strncpy(local_buffer, text, BUFFER_SIZE - 1);
    local_buffer[BUFFER_SIZE - 1] = '\0'; // Ensure null termination
    printk(KERN_INFO "Stored text: %s\n", local_buffer);
    return strlen(local_buffer);
}

// Retrieve text from the local buffer
const char* retrieve_text(void) {
    if (!local_buffer) return NULL;
    return local_buffer;
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
MODULE_DESCRIPTION("USB Stick Driver with Local Buffer Storage");

