#
# Makefile for the NetXXXX drivers
#

REL=$(shell uname -r)


ifneq ($(KERNELRELEASE),)

obj-m	+= gpio.o
#obj-m	+= writelcd.o

gpio-objs	:= common.o 4501driver.o 4801driver.o 5501driver.o
#writelcd-objs	:= common_writelcd.o 4501driver.o 4801driver.o 5501driver.o

else
KDIR	:= /lib/modules/$(REL)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
endif

install:
	cp -a *.ko /lib/modules/$(REL)/kernel/drivers/misc/

clean:
	rm -rf Module.symvers *.order .tmp_versions ./.*.cmd *.mod.c *.o *.ko || true

drivertest:
	$(CC) -O2 -s -Wall drivertest.c -o drivertest
