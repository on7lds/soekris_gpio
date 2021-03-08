/*
 Soekris Netxxxx GPIO driver
 Common functions shared between the net4501, net4801 and net5501 driver

  (c) Copyright 2003-2004   Martin Hejl <martin@hejl.de>
                            G&H Softwareentwicklung GmbH
 Modifications for Net5501 and kernel >2.6.32 (see changelog.txt)
   (c) Copyright 2010        Lieven De Samblanx <on7lds@amsat.org>


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

#define GPIO_PROC_FILE 1
#define LED_PROC_FILE 2
#define SETTINGS_PROC_FILE 4
#define TEMP_PROC_FILE 8
#define VOLT_PROC_FILE 16

#include "common.h"

#define KBUILD_MODNAME KBUILD_STR(gpio)

struct gpio_operations *driver_ops;
static int gpio_major =  GPIO_MAJOR;
int __number_of_pins;
static unsigned long init_map;
struct semaphore gpio_sema;

extern int net4501_init(struct gpio_operations **driver_ops);
extern int net4801_init(struct gpio_operations **driver_ops);
extern int net5501_init(struct gpio_operations **driver_ops);
void common_cleanup(void);

int toString(unsigned long value, char* buffer, int number_of_bits) {
	static int i;

	/* convert it into a string */
	for(i=number_of_bits;i>0;i--){
		buffer[number_of_bits-i]=test_bit(i-1,&value)?'1':'0';
	}

	buffer[number_of_bits] = '\n';
	buffer[number_of_bits+1] = 0;

	return number_of_bits+1;
}

unsigned long  fromString(char* buffer, int number_of_bits) {
	static int i;
	static unsigned long ret_val=0;

	ret_val = 0;
	/* Create WORD to write from the string */
	for(i=0;i<number_of_bits;i++){
		if (buffer[i] == 0) break;

		if (buffer[i] == '0' || buffer[i] == '1') {
			ret_val=ret_val<<1;
		}

		if (buffer[i] == '1') {
			ret_val |= 1;
		}
	}
	return(ret_val);
}


//------------------------------------------------
static ssize_t net4xxx_gpio_write(struct file *file, const char *data,
                                 size_t count, loff_t *ppos)
{
	unsigned m;
	size_t        i;
	char          port_status=0;
	ssize_t       ret = 0;
	unsigned int  value=0;
	unsigned int  temp_value;

	m = MINOR(file->f_dentry->d_inode->i_rdev);

	if (ppos != &file->f_pos)
		return -ESPIPE;

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	value = 0;
	for (i = 0; i < count; ++i) {
		port_status = 0;
		ret = __get_user(port_status, (char *)data);
		if (ret != 0) {
			//ret = -EFAULT;
			goto out;
		}

		switch(m) {
			case MINOR_BYTE:
				driver_ops->write8Bit(port_status);
				break;

			case MINOR_FULL:
				if ((i&1) == 0) {
					value = ((unsigned char)port_status);
				} else {
					temp_value = ((unsigned char)port_status);
					value |= ((temp_value<<8)&0xF00);
					driver_ops->write16Bit(value);
				}
				break;

			case MINOR_LED:
				driver_ops->writeErrorLed(port_status);
				break;

			default:
				ret=-EINVAL;
				goto out;
		}
		data++;
		*ppos = *ppos+1;
	}
	ret = count;

out:
	up(&gpio_sema);
	return ret;
}

static ssize_t net4xxx_gpio_read(struct file *file, char *buf,
                                size_t count, loff_t *ppos)
{
	unsigned m = MINOR(file->f_dentry->d_inode->i_rdev);
	unsigned long value=0;
	size_t bytes_read=0;
	ssize_t ret = 0;
	char port_status;


	if (count == 0)
		return bytes_read;

	if (ppos != &file->f_pos)
		return -ESPIPE;

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	switch(m) {
		case MINOR_BYTE:
			value = driver_ops->read8Bit();
			port_status = (char)(value & 0xFF);
			if (copy_to_user(buf, &port_status, sizeof(char))) {
				ret = -EFAULT;
				goto out;
			}
			bytes_read++;
			buf++;
			*ppos = *ppos+1;
			ret = bytes_read;
			break;

		case MINOR_FULL:
			value = driver_ops->read16Bit();
			port_status = (char)(value & 0xFF);
			if (copy_to_user(buf, &port_status, sizeof(char))) {
				ret = -EFAULT;
				goto out;
			}
			bytes_read++;
			buf++;
			*ppos = *ppos+1;

			if (count>1) {
				port_status = (char)((value>>8) & 0xF);
				if (copy_to_user(buf, &port_status, sizeof(char))) {
					ret = -EFAULT;
					goto out;
				}
				bytes_read++;
				buf++;
				*ppos = *ppos+1;
			}

			ret = bytes_read;
			break;

		case MINOR_LED:
			port_status = driver_ops->readErrorLed();
			if (copy_to_user(buf, &port_status, sizeof(char))) {
				ret = -EFAULT;
				goto out;
			}
			bytes_read++;
			buf++;
			*ppos = *ppos+1;
			ret = bytes_read;
			break;

		default:
			ret = -EFAULT;
			goto out;
	}


out:
printk(OUR_NAME ": net4xxx_gpio_read( [%s] %i)\n",buf,count);
	up(&gpio_sema);
	return ret;
}


static int net4xxx_gpio_open(struct inode *inode, struct file *file)
{
	unsigned m = MINOR(inode->i_rdev);
	if (m!=MINOR_BYTE && m!=MINOR_FULL && m!=MINOR_LED )
		return -EINVAL;

	return 0;
}

static int net4xxx_gpio_release(struct inode *inode, struct file *file)
{
	return 0;
}

//------------------------------------------------

// PROC file
static int procfile_gpio_read( char *buffer,
                               __attribute__ ((unused)) char **start,
                               off_t offset,
                               int buffer_length,
                               int *eof,
                               __attribute__ ((unused)) void *data)

{
	int len; /* The number of bytes actually used */
	unsigned int port_status=0;

	/* We give all of our information in one go, so if the
	 * user asks us if we have more information the
	 * answer should always be no.
	 *
	 * This is important because the standard read
	 * function from the library would continue to issue
	 * the read system call until the kernel replies
	 * that it has no more information, or until its
	 * buffer is filled.
	 */
	if (offset > 0 || buffer_length<__number_of_pins+2) {
		return 0;
	}

	len = buffer_length;
	if (len > __number_of_pins+1) {
		len = __number_of_pins+1;
	}

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	/* Get the status of the gpio ports */

	if (driver_ops->read16Bit == NULL) {
		port_status = driver_ops->read8Bit();
	} else {
		port_status = driver_ops->read16Bit();
	}

	len = toString(port_status,buffer,__number_of_pins);

	//*start = buffer;
	*eof = 1;

	/* Return the length */
	up(&gpio_sema);

	return len;
}

static int procfile_gpio_write(
		__attribute__ ((unused)) struct file *file,
		const char *buf,
		unsigned long count,
		__attribute__ ((unused)) void *data)
{
	int len;
	char new_gpio_state[MAX_NUMBER_OF_PINS+1];
	unsigned int  gpio_state;

	if (count==0) return 0;

	if(count > __number_of_pins) {
		len = __number_of_pins;
	} else {
		len = count;
	}

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	if(copy_from_user(new_gpio_state, buf, len)) {
		up(&gpio_sema);
		return -EFAULT;
	}

	gpio_state = fromString(new_gpio_state,__number_of_pins);

	if (driver_ops->write16Bit==NULL) {
		driver_ops->write8Bit(gpio_state);
	} else {
		driver_ops->write16Bit(gpio_state);
	}

	up(&gpio_sema);

	return len;
}

static int procfile_settings_read(
                               char *buffer,
                               char **start,
                               off_t offset,
                               int buffer_length,
                               int *eof,
                               __attribute__ ((unused)) void *data)

{
	int len; /* The number of bytes actually used */
	unsigned int port_status=0;

	if (offset > 0 || buffer_length<__number_of_pins+2)
		return 0;

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	if (driver_ops->set16BitDirection == NULL) {
		port_status = driver_ops->get8BitDirection();
	} else {
		port_status = driver_ops->get16BitDirection();
	}

	len = toString(port_status,buffer,__number_of_pins);

	// *start = buffer;
	*eof=1;

	up(&gpio_sema);

	return len;
}

static int procfile_settings_write(
                               __attribute__ ((unused)) struct file *file,
                               const char *buf,
                               unsigned long count,
                               __attribute__ ((unused)) void *data)
{
	int len;
	int p1,p2;
	char ch;
	char new_gpio_state[MAX_NUMBER_OF_PINS+1];
	unsigned int  gpio_state;

	if (count==0) return 0;

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	if(count > __number_of_pins) {
		len = __number_of_pins;
	} else {
	len = count;
	}

	if(copy_from_user(new_gpio_state, buf, len)) {
		up(&gpio_sema);
		return -EFAULT;
	}
	new_gpio_state[len]=0;
	
	/* make sure our string only contains 1 and 0 */
	p1=0;
	p2=0;
	while (p2<=len) {
		ch=new_gpio_state[p2];

		if (ch == '0' || ch == '1' || ch == '\0') {
			new_gpio_state[p1] = ch;
			p1++;
		}

		if (ch==0) break;
			p2++;
	}
	new_gpio_state[p1] = 0;

	if (strlen(new_gpio_state)>0) {
		gpio_state = fromString(new_gpio_state,__number_of_pins);

		if (driver_ops->set16BitDirection==NULL) {
			driver_ops->set8BitDirection(gpio_state);
		} else {
			driver_ops->set16BitDirection(gpio_state);
		}
	}

	up(&gpio_sema);
	return len;
}

static int procfile_led_read(
					char *buffer,
					__attribute__ ((unused)) char **start,
					off_t offset,
					int len,
					int *eof,
					__attribute__ ((unused)) void *data)

{
	unsigned int error_led_status;

	if (offset > 0 || len<3) {
		return 0;
	}

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	error_led_status = driver_ops->readErrorLed();

	if (error_led_status) {
		buffer[0] = '1';
	} else {
		buffer[0] = '0';
	}

	buffer[1] = '\n';
	buffer[2] = 0;

	/* *start = buffer;*/
	*eof=1;

	up(&gpio_sema);

	return 2;
}

static int procfile_led_write( __attribute__ ((unused)) struct file *file,
                               const char *buf,
                               unsigned long count,
                               __attribute__ ((unused)) void *data)
{
	int len;
	char new_led_state[MAX_NUMBER_OF_PINS+1];

	if (count==0) return 0;

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	if(count > __number_of_pins) {
		len = __number_of_pins;
	} else {
		len = count;
	}

	if(copy_from_user(new_led_state, buf, len)) {
		return -EFAULT;
	}

	if (new_led_state[0] == '1') {
		driver_ops->writeErrorLed(1);
	} else {
		driver_ops->writeErrorLed(0);
	}

	up(&gpio_sema);

	return len;
}


static int procfile_temperature_read(
                               char *buffer,
                               __attribute__ ((unused)) char **start,
                               off_t offset,
                               int buffer_length,
                               int *eof,
                               __attribute__ ((unused)) void *data)
{
	int len;

	if (driver_ops->readTemperature==NULL ||
                               offset > 0 ||
                               buffer_length<TEMPERATURE_BUFFER_SIZE)
	{
		return 0;
	}

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	len = driver_ops->readTemperature(buffer, buffer_length);

	*eof=1;

	up(&gpio_sema);
	return len;
}

static int procfile_voltage_read(
                               char *buffer,
                               __attribute__ ((unused)) char **start,
                               off_t offset,
                               int buffer_length,
                               int *eof,
                               __attribute__ ((unused)) void *data)
{
	int len;

	if (driver_ops->readVoltage==NULL ||
          offset > 0 ||
          buffer_length<VOLTAGE_BUFFER_SIZE)
	{
		return 0;
	}

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	len = driver_ops->readVoltage(buffer, buffer_length);

	*eof=1;

	up(&gpio_sema);

	return len;
}

//------------------------------------------------


int net4xxx_gpio_ioctl(struct inode *inode, struct file *filp,
                unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int ret = 0;
	int value=0;

	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != PP_IOCTL) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
 	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (err) return -EFAULT;

	if (down_interruptible(&gpio_sema))
		return -ERESTARTSYS;

	switch(MINOR(inode->i_rdev)) {
		case MINOR_BYTE:        // /dev/gpio0 controls the 8 gpio ports
		case MINOR_FULL:        // /dev/gpio1 controls the 12 gpio ports
			switch(cmd) {
				case PPCLAIM:
				case PPRELEASE:
				case PPFCONTROL:
					break;

				case PPWDATA:
					ret = __get_user(value, (unsigned char *)arg);

					if (ret==0) {
						driver_ops->write8Bit(value);
					}
					break;

				case PPRDATA:
					value = driver_ops->read8Bit();
					ret = __put_user(value, (unsigned char *)arg);
					break;

				case PPDATADIR:
					ret = __get_user(value, (unsigned char *)arg);

					/* linux/ppdev.h define PPDATADIR as
					"Data line direction: non-zero for input mode."

					For gpios, the logic is reversed - bit=1 == output

					This is _not_ "generic" at all, but very much hard-wired
					towards being able to use an HD44780 LCD on the GPIO pins
					(in 4-bit mode) and being able to do so using generic
					ppdev instructions

					So, GPIO4-GPIO7 will _always_ be set to output for this
					call, only GPIO0-GPIO3 are changed*/

					if (ret==0) {
						if (value==0) {
							driver_ops->set8BitDirection(0xFF);
						} else {
							driver_ops->set8BitDirection(0xF0);
						}
					} else {
						printk(OUR_NAME ": ret=%x\n", ret);
					}

					break;

				case GPIORDDIRECTION:
					if (MINOR(inode->i_rdev)==MINOR_BYTE) {
						value = driver_ops->get8BitDirection();
					} else {
						value = driver_ops->get16BitDirection();
					}
					ret = __put_user(value, (unsigned int *)arg);
					break;

				case GPIOWRDIRECTION:
					ret = __get_user(value, (unsigned int *)arg);

					if (ret==0) {
						if (MINOR(inode->i_rdev)==MINOR_BYTE) {
							driver_ops->set8BitDirection(value);
						} else {
							driver_ops->set16BitDirection(value);
						}
					}

					break;

				case GPIORDDATA:
					if (MINOR(inode->i_rdev)==MINOR_BYTE) {
						value = driver_ops->read8Bit();
					} else {
						value = driver_ops->read16Bit();
					}

					ret = __put_user(value, (unsigned int *)arg);
					break;

				case GPIOWRDATA:
					ret = __get_user(value, (unsigned int *)arg);

					if (ret==0) {
						if (MINOR(inode->i_rdev)==MINOR_BYTE) {
							driver_ops->write8Bit(value);
						} else {
							driver_ops->write16Bit(value);
						}
					}
					break;

				default:
					return -ENOTTY;
			}
			break;

		case MINOR_LED:        // /dev/gpio254 controls the error led
			switch(cmd) {
				case PPCLAIM:
				case PPRELEASE:
				case PPFCONTROL:
				case PPDATADIR: /* Doesn't work the same way */
					break;

				case PPWDATA:
					ret = __get_user(value, (unsigned char *)arg);

					if (ret==0) {
						driver_ops->writeErrorLed(value);
					}

					break;

				case PPRDATA:
					value = driver_ops->readErrorLed();
					ret = __put_user(value, (unsigned char *)arg);
					break;

				default:
					return -ENOTTY;
			}
			break;
	}

	up(&gpio_sema);

	return ret;

}

static struct file_operations gpio_dispatch_fops = {
	.owner   = THIS_MODULE,
	.write   = net4xxx_gpio_write,
	.read    = net4xxx_gpio_read,
	.open    = net4xxx_gpio_open,
	.release = net4xxx_gpio_release,
	.ioctl   = net4xxx_gpio_ioctl,
};


int __init common_init(void)
{
	static int result;
	static struct proc_dir_entry *GPIO_Proc_File;
	static struct proc_dir_entry *ErrorLED_Proc_File;
	static struct proc_dir_entry *Settings_Proc_File;
	static struct proc_dir_entry *Temperature_Proc_File;
	static struct proc_dir_entry *Voltage_Proc_File;

	init_map = 0;
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

	result = register_chrdev(gpio_major, OUR_NAME, &gpio_dispatch_fops);
	if (result < 0) {
		printk(KERN_WARNING OUR_NAME ": can't get major %d\n", gpio_major);
		return result;
	}

	if (gpio_major == 0) gpio_major = result; /* dynamic */

	GPIO_Proc_File = create_proc_entry(
			GPIO_PROC_FILENAME,
			S_IWUSR |S_IRUSR | S_IRGRP | S_IROTH, /*S_IFREG | S_IRUGO,*/
			NULL);

	if(GPIO_Proc_File == NULL) {
		printk(KERN_ERR OUR_NAME ": Could not register " GPIO_PROC_FILENAME  ". Terminating\n");
		common_cleanup();
		return -ENOMEM;
	} else {
		GPIO_Proc_File->read_proc=procfile_gpio_read;
		GPIO_Proc_File->write_proc=procfile_gpio_write;
		init_map |= GPIO_PROC_FILE;
	}

	ErrorLED_Proc_File = create_proc_entry(
			LED_PROC_FILENAME,
			S_IWUSR |S_IRUSR | S_IRGRP | S_IROTH,
			NULL);

	if(ErrorLED_Proc_File == NULL) {
		printk(KERN_ERR OUR_NAME ":Could not register " LED_PROC_FILENAME ". Terminating\n");
		common_cleanup();
		return -ENOMEM;
	} else {
		ErrorLED_Proc_File->read_proc=procfile_led_read;
		ErrorLED_Proc_File->write_proc=procfile_led_write;
		init_map |= LED_PROC_FILE;
	}

	Settings_Proc_File = create_proc_entry(
			SETTINGS_PROC_FILENAME,
			S_IWUSR |S_IRUSR | S_IRGRP | S_IROTH,
			NULL);

	if(Settings_Proc_File == NULL) {
		printk(KERN_ERR OUR_NAME ": Could not register " SETTINGS_PROC_FILENAME ". Terminating\n");
		common_cleanup();
		return -ENOMEM;
	} else {
		Settings_Proc_File->read_proc=procfile_settings_read;
		Settings_Proc_File->write_proc=procfile_settings_write;
		init_map |= SETTINGS_PROC_FILE;
	}

	if (driver_ops->readTemperature!=NULL) {
		Temperature_Proc_File = create_proc_entry(
				TEMPERATURE_PROC_FILENAME,
				S_IRUSR | S_IRGRP | S_IROTH,
				NULL);

		if(Temperature_Proc_File == NULL) {
			printk(KERN_ERR OUR_NAME ": Could not register " TEMPERATURE_PROC_FILENAME ". Terminating\n");
			common_cleanup();
			return -ENOMEM;
		} else {
			Temperature_Proc_File->read_proc=procfile_temperature_read;
			init_map |= TEMP_PROC_FILE;
		}
	}

	if (driver_ops->readVoltage!=NULL) {
		Voltage_Proc_File = create_proc_entry(
				VOLTAGE_PROC_FILENAME,
				S_IRUSR | S_IRGRP | S_IROTH,
				NULL);

		if(Voltage_Proc_File  == NULL) {
			printk(KERN_ERR OUR_NAME ": Could not register " VOLTAGE_PROC_FILENAME ". Terminating\n");
			common_cleanup();
			return -ENOMEM;
		} else {
			Voltage_Proc_File->read_proc=procfile_voltage_read;
			init_map |= VOLT_PROC_FILE;
		}
	}

	sema_init(&gpio_sema, 1);

	/* set gpio0-7 to output */
	driver_ops->set8BitDirection(0xFF);

	return(0);
}

void common_cleanup(void)
{
	if (init_map&VOLT_PROC_FILE)
		remove_proc_entry(VOLTAGE_PROC_FILENAME, NULL);

	if (init_map&TEMP_PROC_FILE)
		remove_proc_entry(TEMPERATURE_PROC_FILENAME, NULL);

	if (init_map&SETTINGS_PROC_FILE)
		remove_proc_entry(SETTINGS_PROC_FILENAME, NULL);

	if (init_map&LED_PROC_FILE)
		remove_proc_entry(LED_PROC_FILENAME, NULL);

	if (init_map&GPIO_PROC_FILE)
		remove_proc_entry(GPIO_PROC_FILENAME, NULL);

	unregister_chrdev(gpio_major, OUR_NAME);

	driver_ops->cleanup();

}

//------------------------------------------------

module_init(common_init);
module_exit(common_cleanup);

EXPORT_SYMBOL(__number_of_pins);
MODULE_AUTHOR("Martin Hejl");
MODULE_DESCRIPTION("Soekris net4xxx GPIO / Error LED driver");
MODULE_LICENSE("GPL");
