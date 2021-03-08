/*
 * (c) Copyright 2004        Martin Hejl <martin@hejl.de>
 *                           G&H Softwareentwicklung GmbH
 *
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
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>

// Needed for the IOCTL defines
/* Read/write data */
#define PP_IOCTL	'p'

#define PPRDATA		_IOR(PP_IOCTL, 0x85, unsigned char)
#define PPWDATA		_IOW(PP_IOCTL, 0x86, unsigned char)

/* Read/write econtrol (not used) */
#define PPRECONTROL	OBSOLETE__IOR(PP_IOCTL, 0x87, unsigned char)
#define PPWECONTROL	OBSOLETE__IOW(PP_IOCTL, 0x88, unsigned char)

/* Claim the port to start using it */
#define PPCLAIM		_IO(PP_IOCTL, 0x8b)

/* Release the port when you aren't using it */
#define PPRELEASE	_IO(PP_IOCTL, 0x8c)

/* Data line direction: non-zero for input mode. */
#define PPDATADIR	_IOW(PP_IOCTL, 0x90, int)


#define GPIORDDIRECTION _IOR(PP_IOCTL, 0xF0, int)
#define GPIOWRDIRECTION _IOW(PP_IOCTL, 0xF1, int)

/* Read/write data */
#define GPIORDDATA _IOR(PP_IOCTL, 0xF2, int)
#define GPIOWRDATA _IOW(PP_IOCTL, 0xF3, int)



#define PROC_ERROR_LED "/proc/driver/soekris_error_led"
#define PROC_DIRECTION "/proc/driver/soekris_io_settings"
#define PROC_GPIO      "/proc/driver/soekris_gpio"

#define DEV_GPIO0      "/dev/gpio0"
#define DEV_GPIO1      "/dev/gpio1"
#define DEV_GPIO254    "/dev/gpio254"

#define BUF_SIZE 4096


void write_to_fd(int fd, char* buf, int len) {
	int r;

	r=write(fd, buf,len);
	fsync(fd);

//	if (r!=len)
	{
	    fprintf(stderr, "(%2i*%2i) ",r,len);
	}
}

void read_from_fd(int fd, char* buf, int len) {
	int readlen=0;
	lseek(fd,0,SEEK_SET);
	//memset(buf,0,BUF_SIZE);
	readlen = read(fd, buf, len);
	buf[readlen]=0;
	//fprintf(stderr, "Read %d bytes \n%s\n", readlen, buf);
}


int main (int argc, char *argv[])
{
	int use4801 =0;
	char buffer[BUF_SIZE+1];
	char dev_buffer[BUF_SIZE+1];
	int fProcErrorLED;
	int fProcDirection;
	int fProcGPIO;
	int fDevGpio0;
	int fDevGpio1;
//	int fDevGpio2;
	int fDevGpio254;
	int passed=0;
	int all_passed=1;
	unsigned int ioctl_int;
	unsigned int ioctl_int_in;

	char* toTest;

	buffer[BUF_SIZE] = 0;

	/* Tests for both 45xx and 4801 */

	/* Open the "file" in the proc system, which provides us with the */
	/* required information about the network devices */
	fProcErrorLED = open(PROC_ERROR_LED, O_RDWR);
	if (fProcErrorLED == -1) {
		fprintf(stderr, "Unable to open %s.", PROC_ERROR_LED);
		return 1;

	}

	fProcDirection = open(PROC_DIRECTION, O_RDWR);
	if (fProcDirection == -1) {
		fprintf(stderr, "Unable to open %s.", PROC_DIRECTION);
		return 1;
	}

	fProcGPIO = open(PROC_GPIO, O_RDWR);
	if (fProcGPIO == -1) {
		fprintf(stderr, "Unable to open %s.", PROC_DIRECTION);
		return 1;
	}

	fDevGpio0 = open(DEV_GPIO0, O_RDWR);
	if (fDevGpio0 == -1) {
		fprintf(stderr, "Unable to open %s.", DEV_GPIO0);
		return 1;
	}


	/* find out it we're dealing with a 4801 */
	read_from_fd(fProcGPIO,buffer, BUF_SIZE);
	if (strlen(buffer)>= 10) {
		use4801=1;
		fprintf(stderr, "Device is a %s (%i bits I/O)\n", use4801?"48xx/55xx":"45xx",strlen(buffer)-1);
	}

	// write 1 to error led
	write_to_fd(fProcErrorLED , "1",1);

	// read 1 from error led
	read_from_fd(fProcErrorLED,buffer, BUF_SIZE);
	passed = (buffer[0] == '1');
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 1 to /proc/driver/soekris_error_led              %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }
	// write 0 to error led
	write_to_fd(fProcErrorLED , "0",1);

	// read 0 from error led
	read_from_fd(fProcErrorLED,buffer, BUF_SIZE);
	passed = (buffer[0] == '0');
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0 to /proc/driver/soekris_error_led              %s\n",passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }


	close(fProcErrorLED);

	fDevGpio0 = open(DEV_GPIO0, O_RDWR);
	if (fDevGpio0 == -1) {
		fprintf(stderr, "Unable to open %s.", DEV_GPIO0);
		return 1;
	}

	fDevGpio254 = open(DEV_GPIO254, O_RDWR);
	if (fDevGpio254 == -1) {
		fprintf(stderr, "Unable to open %s.", DEV_GPIO254);
		return 1;
	}
fprintf(stderr, "----- fDevGpio0=%i, fDevGpio254=%i\n",fDevGpio0,fDevGpio254);
	dev_buffer[0]= 1;
	write_to_fd(fDevGpio254, dev_buffer,1);
	read_from_fd(fDevGpio254,buffer, BUF_SIZE);
	passed = (buffer[0] == dev_buffer[0]);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 1 to /dev/gpio254                                %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	dev_buffer[0]= 0;
	write_to_fd(fDevGpio254, dev_buffer,1);
	read_from_fd(fDevGpio254,buffer, BUF_SIZE);
	passed = (buffer[0] == dev_buffer[0]);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0 to /dev/gpio254                                %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	write_to_fd(fProcDirection, "11111111\n",strlen("11111111\n"));
fprintf(stderr, "\n");
	dev_buffer[0]= 0;
	write_to_fd(fDevGpio0, dev_buffer,1);
	read_from_fd(fDevGpio0,buffer, BUF_SIZE);
	passed = (buffer[0] == dev_buffer[0]);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 00000000 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	dev_buffer[0]= 0xFF;
	write_to_fd(fDevGpio0, dev_buffer,1);
	read_from_fd(fDevGpio0,buffer, BUF_SIZE);
	passed = (buffer[0] == dev_buffer[0]);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 11111111 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	dev_buffer[0]= 0xAA;
	write_to_fd(fDevGpio0, dev_buffer,1);
	read_from_fd(fDevGpio0,buffer, BUF_SIZE);
	passed = (buffer[0] == dev_buffer[0]);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 10101010 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }


	dev_buffer[0]= 0x55;
	write_to_fd(fDevGpio0, dev_buffer,1);
	read_from_fd(fDevGpio0,buffer, BUF_SIZE);
	passed = (buffer[0] == dev_buffer[0]);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 01010101 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }


	write_to_fd(fProcDirection, "11111111\n",strlen("11111111\n"));
fprintf(stderr, "\n");

	buffer[0]= 0;

	// ioctl on /dev/gpio0
	ioctl(fDevGpio0, PPWDATA,buffer);
	ioctl(fDevGpio0, PPRDATA,dev_buffer);

	passed = (buffer[0] == dev_buffer[0]);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0 to /dev/gpio0 with IOCTL PPWDATA               %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	buffer[0]= 0;
	ioctl(fDevGpio0, PPDATADIR,buffer);
	ioctl(fDevGpio0, GPIORDDIRECTION,&ioctl_int);

	passed = (ioctl_int == 0xFF);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0 to /dev/gpio0 with IOCTL PPDATADIR             %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	buffer[0]= 1;
	ioctl(fDevGpio0, PPDATADIR,buffer);
	ioctl(fDevGpio0, GPIORDDIRECTION,&ioctl_int);

	passed = (ioctl_int == 0xF0);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 1 to /dev/gpio0 with IOCTL PPDATADIR             %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }


	/* IOCTL tests */
	ioctl_int_in= 0x0;
	ioctl(fDevGpio0, GPIOWRDIRECTION,&ioctl_int_in);
	ioctl(fDevGpio0, GPIORDDIRECTION,&ioctl_int);

	passed = (ioctl_int == ioctl_int_in);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0 to /dev/gpio0 with IOCTL GPIOWRDIRECTION       %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	ioctl_int_in= 0xff;
	ioctl(fDevGpio0, GPIOWRDIRECTION,&ioctl_int_in);
	ioctl(fDevGpio0, GPIORDDIRECTION,&ioctl_int);

	passed = (ioctl_int == ioctl_int_in);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0xFF to /dev/gpio0 with IOCTL GPIOWRDIRECTION    %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	ioctl_int_in= 0xff;
	ioctl(fDevGpio0, GPIOWRDATA,&ioctl_int_in);
	ioctl(fDevGpio0, GPIORDDATA,&ioctl_int);

	passed = (ioctl_int == ioctl_int_in);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0xff to /dev/gpio0 with IOCTL GPIOWRDATA         %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	ioctl_int_in= 0x00;
	ioctl(fDevGpio0, GPIOWRDATA,&ioctl_int_in);
	ioctl(fDevGpio0, GPIORDDATA,&ioctl_int);

	passed = (ioctl_int == ioctl_int_in);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0x00 to /dev/gpio0 with IOCTL GPIOWRDATA         %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	ioctl_int_in= 0x55;
	ioctl(fDevGpio0, GPIOWRDATA,&ioctl_int_in);
	ioctl(fDevGpio0, GPIORDDATA,&ioctl_int);

	passed = (ioctl_int == ioctl_int_in);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0x55 to /dev/gpio0 with IOCTL GPIOWRDATA         %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

	ioctl_int_in= 0xAA;
	ioctl(fDevGpio0, GPIOWRDATA,&ioctl_int_in);
	ioctl(fDevGpio0, GPIORDDATA,&ioctl_int);

	passed = (ioctl_int == ioctl_int_in);
	all_passed = all_passed && passed;
	fprintf(stderr, " Writing 0xAA to /dev/gpio0 with IOCTL GPIOWRDATA         %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }


// ioctl on /dev/gpio254
	if (!use4801) {
		/* Tests for 45xx only */




		// Set everything to input
		// and test /dev/gpio0
		write_to_fd(fProcDirection, "00000000\n",strlen("00000000\n"));

		dev_buffer[0]= 0;
		write_to_fd(fDevGpio0, dev_buffer,1);
		read_from_fd(fDevGpio0,buffer, BUF_SIZE);
		passed = (buffer[0]==0xF0);
		all_passed = all_passed && passed;
		fprintf(stderr, "  Writing 00000000 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0]= 0xFF;
		write_to_fd(fDevGpio0, dev_buffer,1);
		read_from_fd(fDevGpio0,buffer, BUF_SIZE);
		passed = (buffer[0] == 0xF0);
		all_passed = all_passed && passed;

		fprintf(stderr, " Writing 11111111 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0]= 0xAA;
		write_to_fd(fDevGpio0, dev_buffer,1);
		read_from_fd(fDevGpio0,buffer, BUF_SIZE);
		passed = (buffer[0] == 0xF0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 10101010 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0]= 0x55;
		write_to_fd(fDevGpio0, dev_buffer,1);
		read_from_fd(fDevGpio0,buffer, BUF_SIZE);
		passed = (buffer[0] == 0xF0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 01010101 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// set direction to 00000000
		write_to_fd(fProcDirection, "00000000\n",strlen("00000000\n"));
fprintf(stderr, "\n");

		toTest = "00000000\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
		read_from_fd(fProcDirection,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 00000000 to /proc/driver/soekris_io_settings     %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// set direction to 11111111
		toTest = "11111111\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
		read_from_fd(fProcDirection,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 11111111 to /proc/driver/soekris_io_settings     %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// set direction to 10101010
		toTest = "10101010\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
		read_from_fd(fProcDirection,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 10101010 to /proc/driver/soekris_io_settings     %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// set direction to 01010101
		toTest = "01010101\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
		read_from_fd(fProcDirection,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 01010101 to /proc/driver/soekris_gpio            %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// Set everything to output
		toTest = "11111111\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));


		// write 00000000 to GPIO
		toTest = "00000000\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 00000000 to /proc/driver/soekris_gpio            %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// write 11111111 to GPIO
		toTest = "11111111\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 11111111 to /proc/driver/soekris_gpio            %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// write 10101010 to GPIO
		toTest = "10101010\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 10101010 to /proc/driver/soekris_gpio            %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// write 01010101 to GPIO
		toTest = "01010101\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 01010101 to /proc/driver/soekris_gpio            %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }


		// Set everything to input
		// the way the GPIOs are connected on the 4501, we expect 11110000
		// no matter what we write to the port
		toTest = "00000000\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));

		// write 00000000 to GPIO
		toTest = "00000000\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,"11110000\n")==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 00000000 to /proc/driver/soekris_gpio            %s\n", passed?"ok":"not ok\n -->remember not to have anything connected to the GPIOs");
while (1){  if (' ' != getchar())  break; }

		// write 11111111 to GPIO
		toTest = "11111111\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,"11110000\n")==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 11111111 to /proc/driver/soekris_gpio            %s\n", passed?"ok":"not ok\n -->remember not to have anything connected to the GPIOs");
while (1){  if (' ' != getchar())  break; }


		// write 10101010 to GPIO
		toTest = "10101010\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,"11110000\n")==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 10101010 to /proc/driver/soekris_gpio            %s\n", passed?"ok":"not ok\n -->remember not to have anything connected to the GPIOs");
while (1){  if (' ' != getchar())  break; }

		// write 01010101 to GPIO
		toTest = "01010101\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,"11110000\n")==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 01010101 to /proc/driver/soekris_gpio            %s\n", passed?"ok":"not ok\n -->remember not to have anything connected to the GPIOs");
while (1){  if (' ' != getchar())  break; }

	} else {
		/* Tests for 48xx only */
		/* IOCTL tests */
		// Test bits 0-11
		fDevGpio1 = open(DEV_GPIO1, O_RDWR);
		if (fDevGpio1 == -1) {
			fprintf(stderr, "Unable to open %s.", DEV_GPIO1);
			return 1;
		}


		ioctl_int_in= 0x0;
		ioctl(fDevGpio1, GPIOWRDIRECTION,&ioctl_int_in);
		ioctl(fDevGpio1, GPIORDDIRECTION,&ioctl_int);

		passed = (ioctl_int == ioctl_int_in);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0 to /dev/gpio1 with IOCTL GPIOWRDIRECTION       %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		ioctl_int_in= 0xfff;
		ioctl(fDevGpio1, GPIOWRDIRECTION,&ioctl_int_in);
		ioctl(fDevGpio1, GPIORDDIRECTION,&ioctl_int);

		passed = (ioctl_int == ioctl_int_in);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0xFFF to /dev/gpio1 with IOCTL GPIOWRDIRECTION   %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		ioctl_int_in= 0xfff;
		ioctl(fDevGpio1, GPIOWRDATA,&ioctl_int_in);
		ioctl(fDevGpio1, GPIORDDATA,&ioctl_int);

		passed = (ioctl_int == ioctl_int_in);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0xfff to /dev/gpio1 with IOCTL GPIOWRDATA        %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		ioctl_int_in= 0x00;
		ioctl(fDevGpio1, GPIOWRDATA,&ioctl_int_in);
		ioctl(fDevGpio1, GPIORDDATA,&ioctl_int);

		passed = (ioctl_int == ioctl_int_in);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0x00 to /dev/gpio1 with IOCTL GPIOWRDATA         %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		ioctl_int_in= 0x555;
		ioctl(fDevGpio1, GPIOWRDATA,&ioctl_int_in);
		ioctl(fDevGpio1, GPIORDDATA,&ioctl_int);

		passed = (ioctl_int == ioctl_int_in);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0x555 to /dev/gpio1 with IOCTL GPIOWRDATA        %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		ioctl_int_in= 0xAAA;
		ioctl(fDevGpio1, GPIOWRDATA,&ioctl_int_in);
		ioctl(fDevGpio1, GPIORDDATA,&ioctl_int);

		passed = (ioctl_int == ioctl_int_in);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0xAAA to /dev/gpio1 with IOCTL GPIOWRDATA        %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// Set everything to input
		// and test /dev/gpio0
		write_to_fd(fProcDirection, "000000000000\n", strlen("000000000000\n"));
fprintf(stderr, "\n");

		dev_buffer[0]= 0x00;
		write_to_fd(fDevGpio0, dev_buffer,1);
		read_from_fd(fDevGpio0,buffer, BUF_SIZE);
		passed = (buffer[0]==0xFF);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 00000000 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0]= 0xFF;
		write_to_fd(fDevGpio0, dev_buffer,1);
		read_from_fd(fDevGpio0,buffer, BUF_SIZE);
		passed = (buffer[0]==0xFF);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 11111111 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0]= 0xAA;
		write_to_fd(fDevGpio0, dev_buffer,1);
		read_from_fd(fDevGpio0,buffer, BUF_SIZE);
		passed = (buffer[0]==0xFF);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 10101010 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0] = 0x55;
		write_to_fd(fDevGpio0, dev_buffer,1);
		read_from_fd(fDevGpio0,buffer, BUF_SIZE);
		passed = (buffer[0]==0xFF);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 01010101 to /dev/gpio0                           %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// Set everything to output
		toTest = "111111111111\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
fprintf(stderr, "\n");

		dev_buffer[0] = 0;
		dev_buffer[1] = 0;
		write_to_fd(fDevGpio1, dev_buffer,2);
		read_from_fd(fDevGpio1,buffer, BUF_SIZE);
		passed = (dev_buffer[0]==buffer[0] && dev_buffer[1]==buffer[1]);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0x00 to /dev/gpio1                               %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0] = 0xFF;
		dev_buffer[1] = 0xF;
		buffer[0]= 0x0;
		buffer[1]= 0x0;

		write_to_fd(fDevGpio1, dev_buffer,2);
		read_from_fd(fDevGpio1,buffer, BUF_SIZE);
		passed = (dev_buffer[0]==buffer[0] && dev_buffer[1]==buffer[1]);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0xFFF to /dev/gpio1                              %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0]= 0x55;
		dev_buffer[1]= 0x5;
		buffer[0]= 0x0;
		buffer[1]= 0x0;

		write_to_fd(fDevGpio1, dev_buffer,2);
		read_from_fd(fDevGpio1,buffer, BUF_SIZE);
		passed = (dev_buffer[0]==buffer[0] && dev_buffer[1]==buffer[1]);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0x555 to /dev/gpio1                              %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0]= 0xAA;
		dev_buffer[1]= 0xA;
		buffer[0]= 0x0;
		buffer[1]= 0x0;

		write_to_fd(fDevGpio1, dev_buffer,2);
		read_from_fd(fDevGpio1,buffer, BUF_SIZE);
		passed = (dev_buffer[0]==buffer[0] && dev_buffer[1]==buffer[1]);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0xAAA to /dev/gpio1                              %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0]= 0xBC;
		dev_buffer[1]= 0xA;
		buffer[0]= 0x0;
		buffer[1]= 0x0;

		write_to_fd(fDevGpio1, dev_buffer,2);
		read_from_fd(fDevGpio1,buffer, BUF_SIZE);
		passed = (dev_buffer[0]==buffer[0] && dev_buffer[1]==buffer[1]);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0xABC to /dev/gpio1                              %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		dev_buffer[0]= 0xED;
		dev_buffer[1]= 0xF;
		buffer[0]= 0x0;
		buffer[1]= 0x0;

		write_to_fd(fDevGpio1, dev_buffer,2);
		read_from_fd(fDevGpio1,buffer, BUF_SIZE);
		passed = (dev_buffer[0]==buffer[0] && dev_buffer[1]==buffer[1]);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 0xFED to /dev/gpio1                              %s\n", passed?"ok":"not ok");

		close(fDevGpio1);
while (1){  if (' ' != getchar())  break; }

		// set direction to 000000000000
		toTest = "000000000000\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
		read_from_fd(fProcDirection,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 000000000000 to /proc/driver/soekris_io_settings %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// set direction to 111111111111
		toTest = "111111111111\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
		read_from_fd(fProcDirection,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 111111111111 to /proc/driver/soekris_io_settings %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// set direction to 101010101010
		toTest = "101010101010\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
		read_from_fd(fProcDirection,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 101010101010 to /proc/driver/soekris_io_settings %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// set direction to 010101010101
		toTest = "010101010101\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
		read_from_fd(fProcDirection,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 010101010101 to /proc/driver/soekris_io_settings %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// Set everything to output
		toTest = "111111111111\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
fprintf(stderr, " --> output\n");

		// write 000000000000 to GPIO
		toTest = "000000000000\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 000000000000 to /proc/driver/soekris_gpio        %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// write 111111111111 to GPIO
		toTest = "111111111111\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 111111111111 to /proc/driver/soekris_gpio        %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// write 101010101010 to GPIO
		toTest = "101010101010\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 101010101010 to /proc/driver/soekris_gpio        %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// write 010101010101 to GPIO
		toTest = "010101010101\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,toTest)==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 010101010101 to /proc/driver/soekris_gpio        %s\n", passed?"ok":"not ok");
while (1){  if (' ' != getchar())  break; }

		// Set everything to input
		// since all GPIOS are configures as low-active, we expect 111111111111
		// no matter what we write to the port
		toTest = "000000000000\n";
		write_to_fd(fProcDirection, toTest,strlen(toTest));
fprintf(stderr, " --> input\n");

		// write 000000000000 to GPIO
		toTest = "000000000000\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,"111111111111\n")==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 000000000000 to /proc/driver/soekris_gpio        %s\n", passed?"ok":"not ok\n -->remember not to have anything connected to the GPIOs");
while (1){  if (' ' != getchar())  break; }

		// write 111111111111 to GPIO
		toTest = "111111111111\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,"111111111111\n")==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 111111111111 to /proc/driver/soekris_gpio        %s\n", passed?"ok":"not ok\n -->remember not to have anything connected to the GPIOs");
while (1){  if (' ' != getchar())  break; }

		// write 101010101010 to GPIO
		toTest = "101010101010\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,"111111111111\n")==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 101010101010 to /proc/driver/soekris_gpio        %s\n", passed?"ok":"not ok\n -->remember not to have anything connected to the GPIOs");
while (1){  if (' ' != getchar())  break; }

		// write 010101010101 to GPIO
		toTest = "010101010101\n";
		write_to_fd(fProcGPIO, toTest,strlen(toTest));
		read_from_fd(fProcGPIO,buffer, BUF_SIZE);
		passed = (strcmp(buffer,"111111111111\n")==0);
		all_passed = all_passed && passed;
		fprintf(stderr, " Writing 010101010101 to /proc/driver/soekris_gpio        %s\n", passed?"ok":"not ok\n -->remember not to have anything connected to the GPIOs");
while (1){  if (' ' != getchar())  break; }

	}

	close(fDevGpio0);
	close(fDevGpio254);
	if (all_passed) {
		fprintf(stderr, "\nAll tests passed\n");
	} else {
		fprintf(stderr, "\nAt least one test failed!\n");
	}

	return 0;
}
