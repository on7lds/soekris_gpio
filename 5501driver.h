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

 Soekris Net5501 GPIO driver
 Provides basic control (read and write) over the 12 general purpose
  IO-pins of the Soekris Net5501 and the error-led (which happens to
  be connected to another general purpose IO pin)
 Based on the Net4801 driver (c) Copyright 2003-2004 Martin Hejl 
  and the AMD CS5535/CS5536 GPIO driver (c) 2005 Ben Gardner

 (c) Copyright 2010        Lieven De Samblanx <on7lds@amsat.org>

*/

#ifndef __5501GPIO_H__
#define __5501GPIO_H__


#define __NO_VERSION__      /* This isn't "the" file 
                             * of the kernel module */
#include "common.h"							 
#include <linux/version.h>   /* Not included by 
                              * module.h because 
                              * of the __NO_VERSION__ */

/* 12 gpio pins  */
#define NUMBER_OF_PINS 12

#define SC1100_VENDOR_ID     0x1022
#define SC1100_F0_DEVICE_ID  0x2080

/* ofset to use if we can't find the SIO at 0x2E */
#define SIO_BASE_OFFSET 0x20

/* defines for IO-Addresses/offsets */
#define SIO_GPIO_BANK0_OUT sio_gpio_base
#define SIO_GPIO_BANK1_OUT sio_gpio_base+0x4
#define SIO_GPIO_BANK2_OUT sio_gpio_base+0x8

#define SIO_GPIO_BANK0_IN sio_gpio_base + 1
#define SIO_GPIO_BANK1_IN sio_gpio_base+0x5
#define SIO_GPIO_BANK2_IN sio_gpio_base+0x9

#define SIO_INDEX    (0x2E + offset_87336)
#define SIO_DATA     (0x2f + offset_87336)

#define SIO_TMS_BANKS 3	 /* number of temperature sensors*/
#define SIO_TMSCFG  0x08 /* offset of TMS Configuration */
#define SIO_TMSBS   0x09 /* offset of TMS Bank Select */
#define SIO_TCHCFST 0x0A /* offset Temperature Channel Configuration and Status */
#define SIO_RDCHT   0x0B /* offset Read Channel Temperature */

#define SIO_LVM_BANKS 14 /* number of voltage monitors*/
#define SIO_VCNVR   0x07 /* offset of Voltage Conversion Rate */
#define SIO_VLMCFG  0x08 /* offset VLM Configuration */
#define SIO_VLMBS   0x09 /* offset VLM Bank Select */
#define SIO_VCHCFST 0x0A /* offset Voltage Channel Configuration and Status */
#define SIO_RDCHV   0x0B /* offset Read Channel Voltage */

#define SIO_LVM_VREF 1235

#define SIO_DEV_SEL   0x7
#define SIO_DEV_ENB   0x30
#define SIO_GPIO_DEV  0x7
#define SIO_VLM_DEV   0x0D
#define SIO_TMS_DEV   0x0E

#define SIO_BASE_LADDR 0x61
#define SIO_BASE_HADDR 0x60
#define SIO_GPIO_PIN_SELECT    0xF0
#define SIO_GPIO_PIN_CONFIGURE 0xF1

#define PRECISION 100

#define CONFIG_GPIO_INPUT   0x04
#define CONFIG_GPIO_OUTPUT  0x03
#define SIO_SID       0x20    /* SuperI/O ID Register */
#define SIO_SID_VALUE 0xe9    /* Expected value in SuperI/O ID Register */

#define GEODE_GPIO_SIZE 0x2c  /* Size of GPIO register block */
#define GEODE_GPIO_DATA_IN 0x04
#define GEODE_GPIO_DATA_OUT 0x00

static unsigned short int sio_gpio_base=0;
static unsigned short int sio_tms_base=0;
static unsigned short int sio_vlm_base=0;
static unsigned short int geode_gpio_base=0;
static unsigned short int offset_87336=0;

/*

Pin assignment of the Net5501 GPIOs (connected to the 87366 GPIO pins
    and _NOT_ to the GPIO pins of the GEODE processor)

PC87366 Pin   Function         Function  PC87366 Pin
------------  --------         --------  ------------
                     +----+----+
               3,3 V |  1 |  2 | 5 V
                     +----+----+
GPIO 20, 117   GPIO0 |  3 |  4 | GPIO1   GPIO 21, 118
                     +----+----+
GPIO 22, 119   GPIO2 |  5 |  6 | GPIO3   GPIO 23, 120
                     +----+----+
GPIO 24, 121   GPIO4 |  7 |  8 | GPIO5   GPIO 25, 122
                     +----+----+
GPIO 26, 123   GPIO6 |  9 | 10 | GPIO7   GPIO 27, 124
                     +----+----+
                 GND | 11 | 12 | GPIO8   GPIO 4, 6
                     +----+----+
GPIO 5, 7      GPIO9 | 13 | 14 | GND
                     +----+----+
GPIO 13, 55   GPIO10 | 15 | 16 | GPIO11  GPIO 12, 54
                     +----+----+
                 GND | 17 | 18 | RXD     SIN 2, 105
                     +----+----+
SOUT 2, 107      TXD | 19 | 20 | GND
                     +----+----+

Error_LED => GEODE GPIO22

*/

#endif
