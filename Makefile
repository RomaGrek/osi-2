obj-m += kernel_mode.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load: all
	sudo insmod kernel_mode.ko
	sudo dmesg -c

unload:
	sudo rmmod kernel_mode.ko
	sudo dmesg -c
