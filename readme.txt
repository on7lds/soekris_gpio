Soekris Net45xx/Net4801 GPIO driver
Provides basic control (read and write) over the 8/12 general purpose
IO-pins of the Soekris Net45xx/Net4801 and the error-led (which happens 
to be connected to another general purpose IO pin)

On the Net4801 the driver also allows access to the termperature sensors
and voltage monitors.


The latest version of this driver can be found at http://soekris.hejl.de

  (c) Copyright 2003-2004   Martin Hejl <martin@hejl.de>
                            G&H Softwareentwicklung GmbH


The driver automatically detects wether it is run on a 4501 or a 4801 

Credits/References
        Linux Device Drivers, 2nd Edition, O'Reilly, ISBN 0-59600-008-1
        http://www.blurbco.com/~gork/net4501/net4501_modules-1.2.tar.gz
        linux-2.4.20/drivers/char/sc520_wdt.c
        http://www.tldp.org/LDP/lkmpg/
        Soekris Engineering net4501 User's Manual
        Elan SC520 Microcontroller User's Manual
        Elan SC520 Microcontroller Register Set Manual
        soekris-tech mailing list
                (http://lists.soekris.com/mailman/listinfo/soekris-tech)
        Linux scx200 device driver
        Geode SC1100 Specification
        PC87366  Specification
        FreeBSD Soekris Net4801 Environmental Monitor
                (http://phk.freebsd.dk/soekris/env4801/)

Instructions for running the driver on a Soekris Net4501:
---------------------------------------------------------		
Accessing the GPIO pins (assuming all are output):
        echo 11111111 > /proc/driver/soekris_gpio --> turns all ports on
        echo 00000000 > /proc/driver/soekris_gpio --> turns all ports off
        echo 00000100 > /proc/driver/soekris_gpio --> turns GPIO2 on,
                                                      everything else off

        cat /proc/driver/soekris_gpio   --> prints the current
                                            status of the gpio pins

        Note when GPIO are used as inputs:
        when "dangling" (not connected to anything)
        GPIO0 to GPIO3 are seen as "0"
        GPIO4 to GPIO7 are seen as "1"

        accessing the error LED:
        echo 1 > /proc/driver/soekris_error_led --> turns the led on

        cat /proc/driver/soekris_error_led
                 --> prints the current status of the error led

Setting input or output for a pin
        echo 10000001 /proc/driver/soekris_io_settings ->
                      turns GPIO7 and GPIO0 to output, all the others to input

        cat /proc/driver/soekris_io_settings -->prints the current io-settings

Devices:
        major ? (dynamic) minor 0 :   set/read PIO0-7 (Net4501 mapping)
        major ? (dynamic) minor 254 : set/read error_led (Net4501)
        Which major number has been assigned can be determined by looking at
        /proc/devices


Known IOCTL commands:
        PPCLAIM
        PPRELEASE
        PPFCONTROL
                    These don't do anything, but don't cause an error either

        PPDATADIR
                    Data line direction for GPIO0-7: non-zero for input mode.
                    This is _not_ "generic" at all, but very much hard-wired 
                    towards being able to use an HD44780 LCD on the GPIO pins
                    (in 4-bit mode) and being able to do so using generic 
                    ppdev instructions

                    So, GPIO4-GPIO7 will _always_ be set to output after this
                    call, only GPIO0-GPIO3 are changed

        PPWDATA,PPRDATA
                    Reads/writes to the gpio0-7 (minor 0)
                    or the error led (minor 254)
                    (param is interpreted as a char)

        GPIORDDIRECTION,GPIOWRDIRECTION
                    "new" command (not from parport) - same effect
                    as using /proc/driver/soekris_io_settings

        GPIORDDATA,GPIOWRDATA
                    "new" command (not from parport)
                    read/write GPIO0-7
                    (param is interpreted as an int)

                
Instructions for running the driver on a Soekris Net4801:
--------------------------------------------------------- 
Accessing the termperature sensors
        cat /proc/driver/soekris_temp

        Should show something like this:
        Temp 0 0xcd 127 C
        Temp 1 0xcd 127 C
        Temp 2 0x81 46 C

		It seems that Temp 0 and 1 are not connected (they always show 127 C)
		The first value in hex is the contents of the "Temperature Channel 
		Configuration and Status Register". From the PC87366 Spec:

		Bit	Description
		7   End of Conversion. This bit reflects the data in the RDCHT register. 
		    0: No new data (default)
		    1: New data not yet read
		
		6   Open.
		    0: No fault (default)
		    1: Remote diode continuity (open circuit) fault
		
		5   OTS Output Enable.
		    0: Disabled (default)
		    1: Enabled
		
		4   ALERT Output Enable.
		    0: Disabled (default)
		    1: Enabled
		
		3   Channel Overtemperature Limit Exceeded.
		    0: Overtemperature lower than limit (default)
		    1: Overtemperature higher than limit
		
		2   Channel Temperature High Limit Exceeded.
		    0: Temperature lower than limit (default)
		    1: Temperature higher than limit
		
		1   Channel Temperature Low Limit Exceeded. 
		    0: Temperature higher than limit (default)
		    1: Temperature lower than limit
		
		0   Channel Enable.
		    0: Disabled (default)
		    1: Enabled		


		The second value is the temperature in Celsius

Accessing the voltage monitors
        cat /proc/driver/soekris_voltage

		Should show something like this:
		avi0 3.01 V
		avi1 2.04 V
		VCC 5.08 V
		VPWR 12.29 V
		avi4 2.54 V
		avi5 2.54 V
		avi6 0.00 V
		Vsb 3.33 V
		Vdd 3.35 V
		Vbat 3.01 V
		AVdd 3.33 V
		ts0 1.56 V
		ts1 1.58 V
		ts2 1.50 V

		The mapping of the different values to avi0 and so on is taken from 
		http://phk.freebsd.dk/soekris/env4801/ - until Soekris Engineering 
		releases some official specifications.

Accessing the GPIO pins (assuming all are output):
        echo 111111111111 > /proc/driver/soekris_gpio --> turns all ports on
        echo 000000000000 > /proc/driver/soekris_gpio --> turns all ports off
        echo 000000000100 > /proc/driver/soekris_gpio --> turns GPIO2 on,
                                                          everything else off

        cat /proc/driver/soekris_gpio   --> prints the current
                                            status of the gpio pins

        Note when GPIO are used as inputs:
        when "dangling" (not connected to anything) all GPIOS are seen as "1"

        accessing the error LED:
        echo 1 > /proc/driver/soekris_error_led --> turns the led on

        cat /proc/driver/soekris_error_led
                                --> prints the current status of the error led

Setting input or output for a pin
        echo 000010000001 /proc/driver/soekris_io_settings ->
                      turns GPIO7 and GPIO0 to output, all the others to input

        cat /proc/driver/soekris_io_settings -->prints the current io-settings

Devices:
        major ? (dynamic) minor 0 : set/read PIO0-7 (Net4801 mapping)
        major ? (dynamic) minor 1 : set/read PIO0-11 (Net4801 mapping) 
                                    (write low byte first)
        major ? (dynamic) minor 254 : set/read error_led (Net4801)
        Which major number has been assigned can be determined by looking at
        /proc/devices


Known IOCTL commands:
        PPCLAIM
        PPRELEASE
        PPFCONTROL
                    These don't do anything, but don't cause an error either

        PPDATADIR
                    Data line direction for GPIO0-7: non-zero for input mode.
                    This is _not_ "generic" at all, but very much hard-wired 
                    owards being able to use an HD44780 LCD on the GPIO pins 
                    (in 4-bit mode) and being able to do so using generic 
                    ppdev instructions.
                    So, GPIO4-GPIO7 will _always_ be set to output after this
                    call, only GPIO0-GPIO3 are changed

                    This call does not change the input/output settings of 
                    GPIO8-11

        PPWDATA,PPRDATA
                    Reads/writes to the gpio0-7 (minor 0)
                    or the error led (minor 254)
                    (param is interpreted as a char)

        GPIORDDIRECTION,GPIOWRDIRECTION
                    "new" command (not from parport) - same effect
                    as using /proc/driver/soekris_io_settings
                    (param is interpreted as an int)

        GPIORDDATA,GPIOWRDATA
                    "new" command (not from parport)
                    read/write GPIO0-11 (param is interpreted as an int)

