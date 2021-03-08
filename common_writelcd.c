/*

 minimalistic driver that displays "Booting..." on an HD44780 compatible LCD
 connected to the gpio pins of a Soekris Net4501/4801

  (c) Copyright 2003-2004   Martin Hejl <martin@hejl.de>
                            G&H Softwareentwicklung GmbH

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
 *
 */
#include "common.h"
#include <linux/delay.h>

struct gpio_operations *driver_ops;
struct semaphore gpio_sema;

int __number_of_pins;
extern char* __our_name;


#define WRITELCD_OUR_NAME       "writelcd"

#define SIGNAL_ENABLE 0x40
#define SIGNAL_RS 0x10

#define UPDATE_INTERVAL (HZ/5+1)

static int ledstate;
static int leddirection;


static int init_dummy_module(void);
static void cleanup_dummy_module(void);
extern int net4501_init(struct gpio_operations **driver_ops);
extern int net4801_init(struct gpio_operations **driver_ops);
extern int net5501_init(struct gpio_operations **driver_ops);


static void HD_nibble(unsigned char nibble)
{
	// clear ENABLE
	// put data on DB1..DB4
	// nibble already contains RS bit!
	driver_ops->write8Bit(nibble);

	// Address set-up time
	udelay(1);

	// rise ENABLE
	driver_ops->write8Bit(nibble | SIGNAL_ENABLE);

	// Enable pulse width
	udelay(1);

	// lower ENABLE
	driver_ops->write8Bit(nibble);
}


static void HD_byte (unsigned char data, unsigned char RS)
{
	// send high nibble of the data
	HD_nibble (((data>>4)&0x0f)|RS);

	// Make sure we honour T_CYCLE
	udelay(1);

	// send low nibble of the data
	HD_nibble((data&0x0f)|RS);
}


static void HD_command (unsigned char cmd, int delay)
{
	HD_byte (cmd, 0);
	udelay(delay);
}


static void HD_write (char *string, int len, int delay)
{
	while (len--) {
		// send data with RS enabled
		HD_byte (*(string++), SIGNAL_RS);

		// wait for command completion
		udelay(delay);
	}
}



static int __init init_dummy_module(void)
{
	int result;
	/* initialize, so we get rid of the compiler warning */
	__number_of_pins = 0;


    result = net5501_init(&driver_ops);
    if (result!=0) {
        result = net4801_init(&driver_ops);
	if (result!=0) {
    	    result = net4501_init(&driver_ops);
	    if (result!=0) {
    		return(result);
    	    }
	}
    }

	sema_init(&gpio_sema, 1);

	/* set gpio0-7 to output */
	driver_ops->set8BitDirection(0xFF);

	// Init the LCD
	HD_nibble(0x03); udelay(100);mdelay(4); // 4 Bit mode, wait 4.1 ms
	HD_nibble(0x03); udelay(100);           // 4 Bit mode, wait 100 us
	HD_nibble(0x03); udelay(100);mdelay(4); // 4 Bit mode, wait 4.1 ms
	HD_nibble(0x02); udelay(100);           // 4 Bit mode, wait 100 us
	HD_command (0x28, 40);                  // 4 Bit mode,
                                            // 1/16 duty cycle, 5x8 font


	HD_command (0x08, 40);              // Display off, cursor off, blink off
	HD_command (0x0F, 640); mdelay(1);  // Display on, cursor on, blink on, wait 1.64 ms
	HD_command (0x06, 40);              // cursor moves to right, no shift

	HD_command (0x01, 640);mdelay(1);   // clear display

	mdelay(5);

	//goto 0/0
	HD_command ((0x80), 40);

	// Now send our string
	HD_write("Booting...",10,40);
	ledstate = 7;
	leddirection=1;

	printk(WRITELCD_OUR_NAME ": Version " VERSION " (C) 2004 Martin Hejl - (c) 2010 Lieven De Samblanx\n");

	return(0);
}

static void __exit cleanup_dummy_module(void)
{
	HD_command (0x08, 40); // Display off, cursor off, blink off
	driver_ops->cleanup();
}

module_init(init_dummy_module);
module_exit(cleanup_dummy_module);

MODULE_AUTHOR("Martin Hejl / Lieven De Samblanx");
MODULE_DESCRIPTION("Soekris net4501/4801/5501 GPIO LCD driver");
MODULE_LICENSE("GPL");
