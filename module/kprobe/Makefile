KERNELDIR=/lib/modules/$(shell uname -r)/build

obj-m += son_jprobe.o

default:$(TARGETS)
	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd 
	rm -rf *.o
	rm -rf .tmp_versions
	rm -rf *.symvers
	rm -rf modules.order

