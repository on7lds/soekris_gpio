/*
 **********************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 **********************************************************************

 Definitions common to 4501 and 4801 driver

  (c) Copyright 2003-2004   Martin Hejl <martin@hejl.de>
                            G&H Softwareentwicklung GmbH

*/

#ifndef __COMMON_GPIO_H__
#define __COMMON_GPIO_H__

//#define KBUILD_MODNAME KBUILD_STR(gpio)  
    // moved to common.c to be able to compile common_writelcd

#include <linux/autoconf.h>
#include <config/modversions.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/pci.h>

#define GPIO_PROC_FILENAME      "driver/soekris_gpio"
#define LED_PROC_FILENAME       "driver/soekris_error_led"
#define SETTINGS_PROC_FILENAME  "driver/soekris_io_settings"
#define TEMPERATURE_PROC_FILENAME "driver/soekris_temp"
#define VOLTAGE_PROC_FILENAME 	"driver/soekris_voltage"
#define VERSION                 "1.3.3"

/* by default, we use dynamic allocation of major numbers */
#define GPIO_MAJOR 0
#define GPIO_MAX_MINOR 254

/* minor numbers used*/
#define MINOR_BYTE 0        /* access to GPIO0-GPIO7 */
#define MINOR_FULL 1        /* access to GPIO0-GPIOxx (driver dependant) */
#define MINOR_LED  254      /* access to the error LED */

/* Read/write bitmask that determines input/output pins (1 means output, 0 input) */
#define GPIORDDIRECTION _IOR(PP_IOCTL, 0xF0, int)
#define GPIOWRDIRECTION _IOW(PP_IOCTL, 0xF1, int)

/* Read/write data */
#define GPIORDDATA _IOR(PP_IOCTL, 0xF2, int)
#define GPIOWRDATA _IOW(PP_IOCTL, 0xF3, int)

#define MAX_NUMBER_OF_PINS 32
#define TEMPERATURE_BUFFER_SIZE 256
#define VOLTAGE_BUFFER_SIZE 512

#define OUR_NAME "gpio"

// Pretend we're PPDEV for IOCTL
#include <linux/ppdev.h>

struct gpio_operations {
	void (*writeErrorLed)(unsigned int);
	unsigned int (*readErrorLed)(void);
	void (*write8Bit)(unsigned char);
	unsigned char (*read8Bit)(void);
	void (*set8BitDirection)(unsigned long);
	unsigned char (*get8BitDirection)(void);
	void (*write16Bit)(unsigned int);
	unsigned int (*read16Bit)(void);
	void (*set16BitDirection)(unsigned long);
	unsigned int (*get16BitDirection)(void);
	int (*readTemperature)(char*,int);
	int (*readVoltage)(char*,int);
	int (*init)(struct gpio_operations**);
	void (*cleanup)(void);
};


#endif
