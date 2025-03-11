you will have to add tabs to the makefile

install
sudo apt update
sudo apt install libudev-dev


to compile user space application run:
gcc -o p usb_stick_app.c -ludev -lpthread

