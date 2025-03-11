you will have to add tabs to the makefile

install
sudo apt update
sudo apt install libudev-dev


to compile user space application run:
gcc -o usb_stick_app usb_stick_app.c -ludev -lpthread
to run:
sudo ./usb_stick_app

check kernel log for caesar shift:
run user space app.
enter 1 and enter number from 1-25 for caesar shift
exit program with 6
enter "dmesg | tail -1" to see the last kernel log containing what the shift is
