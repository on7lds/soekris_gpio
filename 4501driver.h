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

Soekris Net45xx GPIO driver
Provides basic control (read and write) over the 8 general purpose
IO-pins of the Soekris Net4501 and the error-led (which happens to
be connected to another general purpose IO pin)
LEAF package by __PACKAGER__, __BUILDDATE__

  (c) Copyright 2003-2004   Martin Hejl <martin@hejl.de>
                            G&H Softwareentwicklung GmbH

*/

#ifndef __4501GPIO_H__
#define __4501GPIO_H__

#define __NO_VERSION__      /* This isn't "the" file 
                             * of the kernel module */
#include "common.h"							 
#include <linux/version.h>   /* Not included by 
                             * module.h because 
                              * of the __NO_VERSION__ */

/* 8 gpio pins  */
#define NUMBER_OF_PINS 8

/* defines for IO-Addresses/offsets */
#define SC520_PIOPFS15_0        0xC20
#define SC520_PIOPFS31_16       0xC22
#define SC520_CSPFS             0xC24
#define SC520_CLKSEL            0xC26
#define SC520_DSCTL             0xC28
#define SC520_PIODIR15_0        0xC2A
#define SC520_PIODIR15_8        0xC2B
#define SC520_PIODIR31_16       0xC2C
#define SC520_PIODIR31_24       0xC2D
#define SC520_PIODATA15_0       0xC30
#define SC520_PIODATA15_9       0xC31
#define SC520_PIODATA31_16      0xC32
#define SC520_PIODATA31_24      0xC33
#define SC520_PIOSET15_0        0xC34
#define SC520_PIOSET15_8        0xC35
#define SC520_PIOSET31_16       0xC36
#define SC520_PIOSET31_24       0xC37
#define SC520_PIOCLR15_0        0xC38
#define SC520_PIOCLR15_8        0xC39
#define SC520_PIOCLR31_16       0xC3A
#define SC520_PIOCLR31_24       0xC3B
#define SC520_CBAR              0xFFFC

#define SC520_MMIO_SIZE         0xC3C

#define MMCR_BASE_DEFAULT 0xFFFEF000


/*
Net45xx GPIO wiring

SC520 Pin   Function      | Pin Number | Function   SC520 Pin
                          +-----+------+
--          +3.3V  Power  |   1 | 2    | +5V Power  --
                          +-----+------+
PIO5, AD10  GPIO 0        |   3 | 4    | GPIO 1     PIO6, AE10
                          +-----+------+
PIO7, AF10  GPIO 2        |   5 | 6    | GPIO 3     PIO8, AF9
                          +-----+------+
PIO21, AF6  GPIO 4        |   7 | 8    ||GPIO 5     PIO22, AF5
                          +-----+------+
PIO11, AC9  GPIO 6        |   9 | 10   | GPIO 7     PIO12, AC8
                          +-----+------+
--          GND           |  11 | 12   | GND        --
                          +-----+------+
--          GND           |  13 | 14   | n-c        --
                          +-----+------+

*/

#endif
