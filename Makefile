obj-m += usb_stick_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
        $(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
        $(MAKE) -C $(KDIR) M=$(PWD) clean

load:
        sudo insmod usb_stick_driver.ko

unload:
        sudo rmmod usb_stick_driver

reload: unload load

dmesg:
        dmesg | tail -30



