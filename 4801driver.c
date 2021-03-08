/*
 Soekris Net4801 GPIO driver
 Provides basic control (read and write) over the 12 general purpose
 IO-pins of the Soekris Net4801 and the error-led (which happens to
 be connected to another general purpose IO pin)
 Also provides read access to the termperature and voltage sensors

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

#include "4801driver.h"
static void _net4801_cleanup(void);
int net4801_init(struct gpio_operations **);

extern int __number_of_pins;
extern unsigned long  fromString(char* , int );
extern int toString(unsigned long , char* , int );

/* From http://phk.freebsd.dk/soekris/env4801/env4801.c 
According to the author the values in the following two arrays were
"guessed and measured in my lab."
But the data they produce looks sensible, so we'll use this until 
there's official documentation from Soekris Engineering
*/

static long sc[14] = {
	1, 1, 2, 20,
	1, 1, 1, 2,
	2, 1, 2, 1,
	1, 1
};

static char const *nm[14] = {
	"avi0",
	"avi1",
	"VCC",
	"VPWR",
	"avi4",
	"avi5",
	"avi6",
	"Vsb",
	"Vdd",
	"Vbat",
	"AVdd",
	"ts0",
	"ts1",
	"ts2"
};

/* 
variable which holds the currently selected device on the PC87366 
(GPIO, TMS, VLM) so we don't keep selecting an already selected device
*/
static unsigned int _selected_device = 0;

static void device_select(unsigned int dev) {
	if (dev==_selected_device) return;

	outb(SIO_DEV_SEL, SIO_INDEX);
	outb(dev, SIO_DATA);
	_selected_device = dev;
}


static void _net4801_write_error_led(unsigned int on) {
	static unsigned long value;

	value = inb(geode_gpio_base+GEODE_GPIO_DATA_OUT+2);
	if (on!=0) {
		set_bit(4, &value);
	} else {
		clear_bit(4, &value);
	};
	outb(value,geode_gpio_base+GEODE_GPIO_DATA_OUT+2);
}

static unsigned int _net4801_read_error_led(void) {
	return ((inb(geode_gpio_base+GEODE_GPIO_DATA_IN+2)&0x10)!=0?1:0);
}

static void _net4801_select_pin(unsigned short port, unsigned short pin) {
	/* select port/pin */
	device_select(SIO_GPIO_DEV);

	outb(SIO_GPIO_PIN_SELECT,SIO_INDEX);
	outb(((port & 0xF)<<4) | (pin & 0xF), SIO_DATA);
}

static void _net4801_write_config(unsigned short port, unsigned short pin, unsigned char setting) {

	device_select(SIO_GPIO_DEV);
	_net4801_select_pin(port, pin);

	/* configure selected port/pin */
	outb(SIO_GPIO_PIN_CONFIGURE,SIO_INDEX);

	outb(setting, SIO_DATA);
}

static unsigned char _net4801_read_config(unsigned short port, unsigned short pin) {
	device_select(SIO_GPIO_DEV);
	_net4801_select_pin(port, pin);

	/* configure selected port/pin */
	outb(SIO_GPIO_PIN_CONFIGURE,SIO_INDEX);

	return (inb(SIO_DATA));
}

static void _net4801_writeGPIO07(unsigned char new_value) {
	device_select(SIO_GPIO_DEV);
	outb(new_value, SIO_GPIO_BANK2_OUT);
}

static unsigned char _net4801_readGPIO07(void) {
	device_select(SIO_GPIO_DEV);
	return inb(SIO_GPIO_BANK2_IN);
}


static void _net4801_setGPIO07Direction(unsigned long new_direction) {
	static int pin;

	for (pin=0;pin<=7;pin++) {
		if (((new_direction>>pin)&1) != 0)
			_net4801_write_config(2,pin,CONFIG_GPIO_OUTPUT);
		else
			_net4801_write_config(2,pin,CONFIG_GPIO_INPUT);
	}
}


static unsigned char _net4801_getGPIO07Direction(void) {
	static int pin;
	static unsigned char value;

	value  = 0;
	for (pin=0;pin<=7;pin++) {
		value |= ((char)(_net4801_read_config(2,pin)) & 0x01)<<pin;
	}
	return (value);
}

static void _net4801_writeGPIO(unsigned int new_value) {
	/* we can safely ignore the direction since the spec explicitly
	states that writing to a GPIO set to input will do nothing */
	device_select(SIO_GPIO_DEV);

	outb((char)((new_value & 0x300)>>4), SIO_GPIO_BANK0_OUT);
	outb((char)((new_value & 0x400)>>7 | ((new_value & 0x800)>>9)), SIO_GPIO_BANK1_OUT);
	outb((char)(new_value & 0xFF), SIO_GPIO_BANK2_OUT);
}

static unsigned int _net4801_readGPIO(void) {
	static unsigned char value;
	static unsigned long ret_val;

		device_select(SIO_GPIO_DEV);

	ret_val = inb(SIO_GPIO_BANK2_IN);
	value = inb(SIO_GPIO_BANK1_IN);
	ret_val |= (value & 0x8)<<7;
	ret_val |= (value & 0x4)<<9;

	value = inb(SIO_GPIO_BANK0_IN);
	ret_val |= (value & 0x30)<<4;

	return (ret_val);
}




static void _net4801_setGPIODirection(unsigned long new_direction) {
	static int pin;

	for (pin=0;pin<=7;pin++) {
		_net4801_write_config(2,pin,test_bit(pin, &new_direction)?CONFIG_GPIO_OUTPUT:CONFIG_GPIO_INPUT);
	}

	_net4801_write_config(0,4,test_bit(8, &new_direction)?CONFIG_GPIO_OUTPUT:CONFIG_GPIO_INPUT);
	_net4801_write_config(0,5,test_bit(9, &new_direction)?CONFIG_GPIO_OUTPUT:CONFIG_GPIO_INPUT);

	_net4801_write_config(1,3,test_bit(10, &new_direction)?CONFIG_GPIO_OUTPUT:CONFIG_GPIO_INPUT);
	_net4801_write_config(1,2,test_bit(11, &new_direction)?CONFIG_GPIO_OUTPUT:CONFIG_GPIO_INPUT);

}


static unsigned int _net4801_getGPIODirection(void) {
	static int pin;
	static unsigned long value=0;

	value  = 0;
	for (pin=0;pin<=7;pin++) {
		if ((_net4801_read_config(2,pin) & 0x01) != 0) set_bit(pin,&value);
	}


	if ((_net4801_read_config(0,4) & 0x01)!=0) set_bit(8,&value);
	if ((_net4801_read_config(0,5) & 0x01)!=0) set_bit(9,&value);
	if ((_net4801_read_config(1,3) & 0x01)!=0) set_bit(10,&value);
	if ((_net4801_read_config(1,2) & 0x01)!=0) set_bit(11,&value);

	return (value&0xFFF);
}

static void _net4801_readTemperature(
	int index,
	unsigned char* status,
	unsigned char* value)
{
	device_select(SIO_VLM_DEV);

	/* bank select */
	outb(index, sio_tms_base + SIO_TMSBS);

	/* Read the status */
	*status = inb(sio_tms_base + SIO_TCHCFST);

	/* Read the temperature */
	*value = inb(sio_tms_base + SIO_RDCHT);
}


static void _net4801_readVoltage(
	int index,
	unsigned char* value)
{

	device_select(SIO_VLM_DEV);

	/* bank select */
	outb(index, sio_vlm_base + SIO_VLMBS);

	/* Read the voltage */
	*value  = inb(sio_vlm_base + SIO_RDCHV);


}


static int _net4801_readTemperature_string(char* buffer,int len) {
	static int i;
	static unsigned char status;
	static unsigned char value;
	static int offs;

	offs = 0;
	status = 0;
	value = 0;

	for(i = 0; i < SIO_TMS_BANKS; i++) {
		_net4801_readTemperature(i,&status, &value);
		offs += sprintf(buffer+offs, "Temp %d 0x%x %d C\n", i, status, value);

		/* buffer overflow - should never happen!! */
		if (offs>=len-1)
			break;
	}
	buffer[offs]='\0';
	return offs+1;
}

static int _net4801_readVoltage_string(char* buffer,int len) {
	static int i;
	static unsigned char value;
	static int offs;
	static long a;
	static long d1;
	static long d2;
	static long d3;

	offs = 0;
	for(i = 0; i < SIO_LVM_BANKS; i++) {
		_net4801_readVoltage(i,&value);

		/* see 11.2.1 of PC87366 Spec, (page 177) */
		a = value*SIO_LVM_VREF*2450/256;

		/* convert it into two decimals */
		d1 = a*PRECISION*sc[i]/1000000;
		d2 = d1/PRECISION;
		d3 = d1 - PRECISION*d2;

		offs += sprintf(buffer+offs, "%s %d.%s%d V\n", nm[i], (int) d2,d3<10?"0":"",(int) d3);

		/* buffer overflow - should never happen!! */
		if (offs>=len)
			break;
	}

	return offs;
}


struct gpio_operations driver4801_ops = {
	.writeErrorLed       = _net4801_write_error_led,
	.readErrorLed        = _net4801_read_error_led,
	.write8Bit           = _net4801_writeGPIO07,
	.read8Bit            = _net4801_readGPIO07,
	.set8BitDirection    = _net4801_setGPIO07Direction,
	.get8BitDirection    = _net4801_getGPIO07Direction,
	.write16Bit          = _net4801_writeGPIO,
	.read16Bit           = _net4801_readGPIO,
	.set16BitDirection   = _net4801_setGPIODirection,
	.get16BitDirection   = _net4801_getGPIODirection,
	.init                = net4801_init,
	.cleanup             = _net4801_cleanup,
	.readTemperature     = _net4801_readTemperature_string,
	.readVoltage         = _net4801_readVoltage_string
};

int net4801_init(struct gpio_operations **driver_ops)
{
	static struct pci_dev *bridge;
	static unsigned base;
	static int i;

	if ((bridge = pci_find_device(SC1100_VENDOR_ID,
					SC1100_F0_DEVICE_ID ,
					NULL)) == NULL)
		return -ENODEV;

	base = pci_resource_start(bridge, 0);

	if (request_region(base, GEODE_GPIO_SIZE, "Soekris net4801 GPIO") == 0) {
		printk(KERN_ERR OUR_NAME ": can't allocate I/O for Geode GPIOs\n");
		return -EBUSY;
	}

	printk(OUR_NAME ": Soekris net4801 GPIO driver Version " VERSION " (C) 2003 Martin Hejl\n");

	geode_gpio_base = base;

	offset_87336 = 0;
	outb(SIO_SID,SIO_INDEX);
	if (inb(SIO_DATA) != SIO_SID_VALUE)
	{
		offset_87336 = SIO_BASE_OFFSET;
		outb(SIO_SID,SIO_INDEX);
		if (inb(SIO_DATA) != SIO_SID_VALUE)
		{
			printk(KERN_ERR OUR_NAME ": PC87336 not detected\n" );

			/* clean up */
			//unregister_chrdev(gpio_major, OUR_NAME);
			release_region(geode_gpio_base, GEODE_GPIO_SIZE);
			return -EBUSY;
		}
	}

	device_select(SIO_GPIO_DEV);

	outb(SIO_DEV_ENB, SIO_INDEX);
	if (inb(SIO_DATA)==0) {
		printk(KERN_ERR OUR_NAME ": GPIOs are disabled\n");
		/* clean up */
		//unregister_chrdev(gpio_major, OUR_NAME);
		release_region(geode_gpio_base, GEODE_GPIO_SIZE);
		return -ENODEV;
	}

	// read the gpio base address
	outb(SIO_BASE_HADDR,SIO_INDEX);
	sio_gpio_base = inb(SIO_DATA) << 8;

	outb(SIO_BASE_LADDR,SIO_INDEX);
	sio_gpio_base |= inb(SIO_DATA) ;


	// select temperature sensors
	device_select(SIO_TMS_DEV);

	// enable
	sio_tms_base = 0;
	outb(SIO_DEV_ENB,SIO_INDEX);
	if (inb(SIO_DATA)==0) {
		printk(OUR_NAME ": TMS disabled\n");
		driver4801_ops.readTemperature = NULL;
	} else {
		// read TMS base address
		outb(SIO_BASE_HADDR,SIO_INDEX );
		sio_tms_base = inb(SIO_DATA) << 8;

		outb(SIO_BASE_LADDR,SIO_INDEX);
		sio_tms_base |= inb(SIO_DATA);
	}

	// select voltage monitors
	device_select(SIO_VLM_DEV);

	// enable
	sio_vlm_base = 0;
	outb(SIO_DEV_ENB,SIO_INDEX);
	if (inb(SIO_DATA)==0) {
		printk(OUR_NAME ": VLM disabled\n");
		driver4801_ops.readVoltage = NULL;
	} else {
		outb(SIO_BASE_HADDR, SIO_INDEX);
		sio_vlm_base = inb(SIO_DATA) << 8;

		outb(SIO_BASE_LADDR, SIO_INDEX);
		sio_vlm_base  |= inb(SIO_DATA);
	}

	// obtain the specified VLM/TMS accuracy (PC87366 Spec, page 208)
	if (sio_tms_base!=0) {
		outb(0x00, sio_tms_base + 0x8 );
		outb(0x0f, sio_tms_base + 0x9 );
		outb(0x08, sio_tms_base + 0xa );
		outb(0x04, sio_tms_base + 0xb );
		outb(0x35, sio_tms_base + 0xc );
		outb(0x05, sio_tms_base + 0xd );
		outb(0x05, sio_tms_base + 0xe );
	}

	/* configure to
	 * Internal Vref,
	 * Monitoring operation enabled
	 */
	outb(0, sio_tms_base + SIO_TMSCFG);

	/* for all  banks */
	for(i = 0; i < SIO_TMS_BANKS; i++) {
		/* select bank */
		outb(i,sio_tms_base + SIO_TMSBS);

		/* enable channel */
		outb(1, sio_tms_base + SIO_TCHCFST);
	}

	/* set sampling delay to 160 us */
	outb(0x10, sio_vlm_base + SIO_VCNVR);

	/* configure to
	 * Internal Vref,
	 * Monitoring operation enabled
	 */
	outb(0, sio_vlm_base + SIO_VLMCFG);

	/* for all banks */
	for(i = 0; i < SIO_LVM_BANKS; i++) {

		/* select bank */
		outb(i, sio_vlm_base + SIO_VLMBS);

		/* enable channel */
		outb(1, sio_vlm_base + SIO_VCHCFST);
	}

	*driver_ops = &driver4801_ops;

	__number_of_pins=NUMBER_OF_PINS;

	return 0;
}

static void _net4801_cleanup(void)
{
	release_region(geode_gpio_base, GEODE_GPIO_SIZE);
}

EXPORT_SYMBOL(net4801_init);

