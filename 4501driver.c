/*
 Soekris Net4501 general purpose IO and error-LED driver
 Provides basic control (read and write) over the 8 general purpose
 IO-pins of the Soekris Net4501 and the error-led (which happens to
 be connected to another general purpose IO pin)

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

#include "4501driver.h"
static void _net4501_cleanup(void);
int net4501_init(struct gpio_operations**);

/* global variables */
static unsigned int direction=0;
static char *mmcr_virt_addr;
extern int __number_of_pins;
extern unsigned long  fromString(char* , int );
extern int toString(unsigned long , char* , int );

static unsigned int read_error_led(void) {
	static unsigned int data;

	/* fetch the current status  */
	data = readb(mmcr_virt_addr + SC520_PIODATA15_0 + 1);

	if ((data & 0x2) ==0) return 0;
	return 1;
}

static void write_error_led(unsigned int new_status) {

	/* set/clear the error led */
	/* since we only need to set/clear one bit */
	/* we don't use the PIODATA port, but rather the PIOSET/PIOCLR port */
	if (new_status==0) {
		/* clear the error led */
		writeb(2, mmcr_virt_addr + SC520_PIOCLR15_8 );
	} else {
		/* set the error led */
		writeb(2, mmcr_virt_addr + SC520_PIOSET15_8 );
	}

}

static unsigned int _generic_read_gpio(int io_offset) {
	static unsigned long int data;
	static unsigned int return_value;

	/* fetch the current status  */
	data = readl(mmcr_virt_addr + io_offset );

	return_value=0;
	return_value |= ((data & 0x1000)    !=0 ? 0x80 : 0);  /* GPIO7->PIO12 */
	return_value |= ((data & 0x800)     !=0 ? 0x40 : 0);  /* GPIO6->PIO11 */
	return_value |= ((data & 0x400000)  !=0 ? 0x20 : 0);  /* GPIO5->PIO22 */
	return_value |= ((data & 0x200000)  !=0 ? 0x10 : 0);  /* GPIO4->PIO21 */
	return_value |= ((data & 0x100)     !=0 ? 0x08 : 0);  /* GPIO3->PIO8  */
	return_value |= ((data & 0x80)      !=0 ? 0x04 : 0);  /* GPIO2->PIO7  */
	return_value |= ((data & 0x40)      !=0 ? 0x02 : 0);  /* GPIO1->PIO6  */
	return_value |= ((data & 0x20)      !=0 ? 0x01 : 0);  /* GPIO0->PIO5  */

	return(return_value & 0xFF);

}

static void _generic_write_gpio(int io_offset, unsigned char new_value) {
	static unsigned long int set_data;
	static unsigned long int clear_data;
	static unsigned long int data;

	if (io_offset == SC520_PIODIR15_0) {
		direction = new_value;

		/* fetch the current status and get rid of the values we will set */
		/* this way we can simply "or" this mask with ne new values */
		/* mask: 1111 1111 1001 1111  1110 0110 0001 1111 */
		data = readl(mmcr_virt_addr + io_offset ) & 0xFF9FE61F;


		// insert the new values
		data |= ((new_value & 0x80)!=0)?0x1000:0;     /* GPIO7 -> PIO12 */
		data |= ((new_value & 0x40)!=0)?0x800 :0;     /* GPIO6 -> PIO11 */

		data |= ((new_value & 0x20)!=0)?0x400000 :0;  /* GPIO5 -> PIO22 */
		data |= ((new_value & 0x10)!=0)?0x200000 :0;  /* GPIO4 -> PIO21 */

		data |= ((new_value & 0x08)!=0)?0x100:0;      /* GPIO3 -> PIO08 */
		data |= ((new_value & 0x04)!=0)?0x80 :0;      /* GPIO2 -> PIO07 */
		data |= ((new_value & 0x02)!=0)?0x40 :0;      /* GPIO1 -> PIO06 */
		data |= ((new_value & 0x01)!=0)?0x20 :0;      /* GPIO0 -> PIO05 */

		//write the data
		writel(data, mmcr_virt_addr + io_offset);
		//printk(OUR_NAME ": wrote $ld to SC520_PIODIR15_0\n", new_value);
	} else {

		// insert the new values
		set_data=0;
		clear_data=0;

		// Make sure we honour the "direction" mask - so we don't write to pins that are set to input
		// the elan will probably simply ignore such requests, but it never hurts to be sure
		if ((direction & 0x80)!=0 && (new_value & 0x80)!=0) set_data|=0x1000; else  clear_data|=0x1000; /* GPIO7 -> PIO12 */
		if ((direction & 0x40)!=0 && (new_value & 0x40)!=0) set_data|=0x800;  else  clear_data|=0x800;  /* GPIO6 -> PIO11 */

		if ((direction & 0x20)!=0 && (new_value & 0x20)!=0) set_data|=0x400000; else  clear_data|=0x400000; /* GPIO5 -> PIO22 */
		if ((direction & 0x10)!=0 && (new_value & 0x10)!=0) set_data|=0x200000; else  clear_data|=0x200000; /* GPIO4 -> PIO21 */

		if ((direction & 0x08)!=0 && (new_value & 0x08)!=0) set_data|=0x100; else  clear_data|=0x100; /* GPIO3 -> PIO08 */
		if ((direction & 0x04)!=0 && (new_value & 0x04)!=0) set_data|=0x80;  else  clear_data|=0x80;  /* GPIO2 -> PIO07 */
		if ((direction & 0x02)!=0 && (new_value & 0x02)!=0) set_data|=0x40;  else  clear_data|=0x40;  /* GPIO1 -> PIO06 */
		if ((direction & 0x01)!=0 && (new_value & 0x01)!=0) set_data|=0x20;  else  clear_data|=0x20;  /* GPIO0 -> PIO05 */


		if (set_data!=0) writel(set_data, mmcr_virt_addr + SC520_PIOSET15_0 );
		if (clear_data!=0) writel(clear_data, mmcr_virt_addr + SC520_PIOCLR15_0 );
	}

}


static unsigned char read_gpio(void) {
	return(_generic_read_gpio(SC520_PIODATA15_0));
}

static void write_gpio(unsigned char new_value) {
	_generic_write_gpio(SC520_PIODATA15_0,new_value);
}

static unsigned char read_direction(void) {
	return(_generic_read_gpio(SC520_PIODIR15_0));
}

static void write_direction(unsigned long new_value) {
	_generic_write_gpio(SC520_PIODIR15_0,new_value);
}


struct gpio_operations driver4501_ops = {
	.writeErrorLed     = write_error_led,
	.readErrorLed      = read_error_led,
	.write8Bit         = write_gpio,
	.read8Bit          = read_gpio,
	.set8BitDirection  = write_direction,
	.get8BitDirection  = read_direction,
	.readTemperature   = NULL,
	.readVoltage       = NULL,
	.init              = net4501_init,
	.cleanup           = _net4501_cleanup
};


int net4501_init(struct gpio_operations **driver_ops)
{
	static unsigned long cbar;
	static unsigned long mmcrbase;


	printk(OUR_NAME ": Soekris net45xx GPIO driver Version " VERSION " (C) 2003 Martin Hejl\n");

	// find MMCR base the address */
	cbar = inl_p(SC520_CBAR);
	/*printk(OUR_NAME ": CBAR: 0x%08lx\n", cbar);*/
	/* check if MMCR aliasing bit is set */
	if (cbar & 0x80000000) {
		/*printk(OUR_NAME ": MMCR Aliasing enabled.\n");*/
		mmcrbase = cbar & 0x3fffffff;
	} else {
		printk(OUR_NAME "!!! WARNING !!!\n"
			"\t MMCR Aliasing found NOT enabled!\n"
			"\t Using default value of: %x\n"
			, MMCR_BASE_DEFAULT
		);
		mmcrbase = MMCR_BASE_DEFAULT;
	}

	/* map bus memory into CPU space   */
	mmcr_virt_addr = ioremap(mmcrbase, SC520_MMIO_SIZE);

	if (!mmcr_virt_addr) {
		printk(OUR_NAME ": Could not remap memory. Terminating\n");
		return -ENOMEM;
	}

		__number_of_pins=NUMBER_OF_PINS;

		*driver_ops = &driver4501_ops;

	return 0;
}



/* Cleanup - unregister our file from /proc */
static void _net4501_cleanup()
{
	iounmap(mmcr_virt_addr);
}

EXPORT_SYMBOL(net4501_init);

