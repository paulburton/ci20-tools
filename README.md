ci20-tools
==========

ci20-tools is a collection of tools for controlling MIPS Creator CI20 boards using USB. The tools are built atop a library named libci20 which enables easy manipulation of the boards state - most fundamentally including memory access & processor state, but with higher level functionality built atop those.

#### Using the tools

First of all you will need to connect your CI20 to the host computer (the one which will run ci20-tools), by connecting the mini-USB port on the board to your host. Of note is that USB power cables intended for the Sony PSP console often include a mini-USB connector in addition to a power connector compatible with the CI20. It is therefore possible to use one of those cables to both power & communicate with a CI20 using a single USB port of the host.

The tools depend upon the board being booted in "USB boot" mode. This can be achieved by placing the boot select jumper between pins 1 & 2 (as though booting from NAND) and holding down the button whilst powering on the board. Alternatively, to avoid the requirement of holding the button, you can place the boot select jumper between pins 2 & 3 (as though booting from SD) and power on the board with no SD card inserted.

Once the board is connected in USB boot mode your host machine should see a USB device with ID a108:4780. Ensure that you have permissions to read & write that device (eg. by setting up an appropriate udev rule). You should now be able to run any of the tools.

#### The tools

* **ci20-usb-boot**: allows you to boot an ELF file on a CI20 board, performing the setup typically required of a bootloader including clock, DDR & cache initialization.

* **ci20-usb-test**: currently a simple test of the USB communication code. It will display the contents of the OTP of a board, and toggle the LED between its red & blue states 10 times.
