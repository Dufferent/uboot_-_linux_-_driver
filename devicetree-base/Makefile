KERNELDIR := /home/ts/qemu/kernel/linux-4.4.248
CURRENT_PATH := $(shell pwd)
obj-m := devicetree-base.o

build:kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean