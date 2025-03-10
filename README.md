# usb_stick_driver

### Instructions
- cd usb_stick_driver
- make
- sudo insmod usb_stick_driver
- sudo dmesg | tail -5
- you should see "USB Stick inserted! Vendor: 0xABCD, Product: 0x1234" after plugging in USB stick 

#### If that doesn't work
It's probably because the default Linux driver has control
- unbind usb-storage: 
- echo "3-2:1.0" | sudo tee /sys/bus/usb/drivers/usb-storage/unbind 
- and replace port number in quotations according to the command: 
- ls /sys/bus/usb/drivers/usb-storage/

- bind your driver: 
- echo "abcd 1234" | sudo tee /sys/bus/usb/drivers/usb_stick_driver/new_id 

Now your driver should be the one in control of the USB stick

### To run the app
- first insmod the driver
- set up udev rules so you have read/ write permission:
- sudo nano /etc/udev/rules.d/99-usb_stick.rules
- (add this line) KERNEL=="usb_stick", MODE="0666"
- save and exit
- sudo udevadm control --reload-rules
- sudo udevadm trigger
- gcc -o usb_stick_app usb_stick_app.c
- ./usb_stick_app

### To Check /proc/usb_stats
- cat /proc/usb_stats