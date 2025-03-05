# usb_stick_driver

## Instructions
cd usb_stick_driver
make
sudo insmod usb_stick_driver
sudo dmesg | tail -20
* you should see "USB Stick inserted! Vendor: 0xABCD, Product: 0x1234" after plugging in USB stick 

## If that doesn't work
It's probably because the default Linux driver has control

unbind usb-storage: 
echo "3-2:1.0" | sudo tee /sys/bus/usb/drivers/usb-storage/unbind 
and replace number in brackets according to previous command

bind your driver: 
echo "abcd 1234" | sudo tee /sys/bus/usb/drivers/usb_stick_driver/new_id 