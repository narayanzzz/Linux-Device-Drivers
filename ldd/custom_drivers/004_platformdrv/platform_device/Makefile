obj-m := platformdev.o

ARCH=arm
CROSS_COMPILE=arm-none-linux-gnueabihf-
KERN_DIR=/home/narayan/linux_source/linux_bbb_4.14/

host:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	
target:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules	
clean_host:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

clean_target:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean

