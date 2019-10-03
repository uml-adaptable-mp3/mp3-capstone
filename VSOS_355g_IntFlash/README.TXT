VSOS KERNEL 3.54 FOR VS1005G INTERNAL FLASH (1 megabyte) IN:
- VS1005G DEVELOPER BOARD
- VS1005G BREAK-OUT BOARD
- VS1005G AMP BOARD

This kernel can also be used with VS1205g and VS8005g.

This kernel uses the internal or external flash for booting the OS, 
e.g. S: disk is flash. Boot the board with button S1 pressed to show
the flash to the PC as a removable drive. 

Since VSOS 3.40 there it is NOT longer necessary to select internal
or external flash from vsos_vs1005g.c. If you are using external
flash, its default size is 2 megabytes, as used in the VS1005G
Developer Board. If you have an external SPI flash of different size,
change it in the file vsos_vs1005g.c.
* WARNING: it's very dangerous to switch power off during flash write.

To use SD card, you must load SDSD driver in runtime, e.g. write 
"SDSD D" in S:CONFIG.TXT and copy SDSD.DL3 and SDSDX.DL3 to S:SYS/
To use the on-board Nand Flash, use K9F4G.DL3 driver.
All drivers can be downloaded from www.vsdsp-forum.com section
'VS1005 Software'.

See video tutorials at:
http://www.vsdsp-forum.com/phpbb/viewtopic.php?f=13&t=1273#p5655


Version History:

Updates in version 3.55 - 2018-03-16
* Added checksum checking into apploader.c from experimental kernel.
  Corrupted binaries now fail loading with a "Corrupt lib" error.

Updates in version 3.54 - 2018-02-08
* Seeking in files that are being written to supported.
* Updating and appending to files supported: in addition to modes "r" and
  "w", fopen() can now also handle "w+", "r+", "a", and "a+".
  Note that writing to the centre of the file may be slow if either:
  - Writes are not aligned with 512 bytes.
  - Write size is not a multiple of 512 bytes.
* Symbols added for more efficient Ogg Vorbis encoder.

Updates in version 3.53 - 2018-01-31
* Speed optimization makes writing large cluster records significantly (20x)
  faster.

Updates in version 3.52 - 2018-01-22
* Corrects situation where MakeFileDescriptor() would lose file descriptors
  if memory units with unsupported file systems are inserted.

Updates in version 3.51 - 2018-01-17
* Supports loading applications that are partially chip-specific. This
  makes it possible to create universal libraries and programs even
  if there are chip-specific portions in them.

Updates in version 3.50 - 2017-12-12
* Supports application-level (.DL3/.AP3), ROM independent binary compatibility
  with the forthcoming VS1005h.
  * To make your .DL3 or .AP3 application or driver compatible with VS1005h,
    do the following:
    * If your project contains rom1005g.txt, remove it from the project.
    * See to it that one of your source files includes <kernel.h>. A good
      practice is to include it from the file containing main().
    * If your project contains any hardcoded symbol information using
      LINK_ABS() or LINKINFO3(), remove these lines.
    * Select Build -> Rebuild Solution
    * Done!
  * To compile solutions that are compatible with VSOS 3.44 and older for
    VS1005g (but not necessarily compatible with VS1005h), do the following:
    * Add the following to Preprocessor definitions:
      KEEP_VSOS_344_AND_OLDER_COMPATIBILITY
    * Add -p B to your project's linker options

Updates in version 3.44 - 2017-08-16
* Corrected a bug in Cyclic which would cause scheduling errors if the
  cyclic task couldn't be serviced in time.

Updates in version 3.42 - 2017-05-22
* Update for file attribute handling and number of clusters in vo_fat.c
* Configuration [S4] no longer increases kernel debug verbosity
* Various updates to VSOS header files

Updates in version 3.41 - 2016-11-29
* Corrects a regression which would prevent one application from
  loading another with the setting-appFile method.

Updates in version 3.40 - 2016-11-03
* VSOS contains now two projects, VSOS_340_Ext creating eeprom_e.img for
  external flash, and VSOS_340_Int creating eeprom_i.img for internal
  flash. The correct one is automatically selected when you program the
  flash with either the internal or extrnal flash prommer. There is no
  need to select USE_INTERNAL_FLASH from source code any more.
* Better stability while booting: first boot record sets internal memory
  delay to an optimal value.
* Hardware SPI now adapts speed according to system clock.
* It is now possible to set Internal / External SPI Flash bitrate to
  speed up the boot process. Right-click project VSOS_340_Ext / VSOS_340_Int
  (or whatever the version number in the future is) -> Properties
   -> Pre/Post Build. In Post-Build Command(s) search for the line
  coff2allboot and the value written to y:0xFC41, i.e. SPI0_CLKCF.
  Bits 9:2 contain a value that tells speed with the formula:
  speed = 6.144 MHz / (value + 1).
  - Example: y:0xFC41=0x4  -> value=1, speed = 6.144MHz / (1+1) = 3.072MHz.
  - Example: y:0xFC41=0x24 -> value=9, speed = 6.144MHz / (1+9) = 0.614MHz.
  The default is now made 3.072 MHz instead of the old 0.512 MHz, speeding
  up the boot process.

Updates in version 3.35 - 2016-09-30
* Clears automatically any interrupt vectors that would have left pointing
  to a dropped library.

Updates in version 3.34 - 2016-09-29
* Added interrupt level and vector printout to CallToDroppedLibrary()

Updates in version 3.33 - 2016-09-28
* DropLibrary now fills cleared IRAM regions with calls to
  CallToDroppedLibrary() so you can detect more easily calls to any library
  which has been dropped. Any call to a dropped library will give an immediate
  stop trace to console.

Updates in version 3.32 - 2016-09-28
* Fixed a bug in library loader that could overwrite an interrupt vector

Updates in version 3.31 - 2016-08-30
* Fixed unlink: deleting non-closed files caused a hangup of FAT filesystem
  routine(s)
* Moved PrintInt and PrintTasks to transient_i to free 221 words of instruction
  memory. These functions print the interrupt and task status at startup and
  are not called after that.

Updates in version 3.30 - 2016-06-22
* Added better debug outputs for libtrace.dl3
* Added post-crash shell
* Modified uartin.dl3, console.dl3 in such a way that is incompatible
  with VSOS 3.27 and older, but which will allow UNICODE support using the
  shell in the future.

Updates in version 3.27 - 2016-03-18
* Added support for VS1005 Amp Board power functions
* Fixed FAT crash bug when a FAT device is not available
* File write date regression from VSOS 3.26 fixed

Updates in version 3.26 - 2016-02-15
* Improved support for listing tasks and memory dumps

Updates in version 3.25 - 2015-10-02
* Allow disk 'U' to be used for configuration 5 USB update mode
* UART Ctrl-C reset in USB update mode

Updates in version 3.24 - 2015-09-21
* RTC file date support (needs RTCREAD.DL3 and SETDATE.DL3)
* Improved write speed to SPI flash

Updates in version 3.23 - 2015-09-07
* UART Shell support integrated to main kernel
* Cyclic task handler
* Improved error messages
* Application loader fixes and improvements
* Exported LoadLibraryP for dynamic loading of drivers
* SystemUIMessageHandler support for equalizer and audio drivers

Updates in version 3.22 - 2015-02-20
* Fixed mutual exclusion problem in fopen which caused system lock-ups
  if call to fopen caused another call to fopen
* Fixed a bug in FAT which sometimes caused corrupted disks when the disk
  became full

Updates in version 3.21
* Exports some kernel functions to make the VLSI Classic Player need less memory

Updates in version 3.20 - 2014-12-15
* Fat filesystem: support for seeking in writable files (todo:append).
* Fat filesystem: fixed writing zero length files.
* Resource allocation fix in devHwSpi
* libvsos03.a: support for unlink (deleting files)
* libvsos03.a: support for creating and deleting directories

Updates in version 3.19.1
* Fixed re-entrancy of application loader
* Support for new SD card driver
* Changed CONFIG.TXT loading slightly: pressing buttons [S1] and [S2] during
  boot will load configuration 5 and then run VoMassStorage on drive S, allowing
  CONFIG.TXT to load another disk driver such as SD or NAND tho show in USB.

Updates in version 3.19 - 2014-10-17
* This is a release to test new features before releasing 3.20.
  Most of the updates are in separate dl3 libs instead of the kernel sources.
* Updated the linker to support new directives ABS and CHIPSPECIFIC
* Added k9f4g.dl3 Nand Flash driver. The new linker is needed to build it.
* Added list of loaded libraries and free memory to main menu.
* Added libtrace.dl3 to trace which library a physical address
  belongs to. Uses find_err.dl3 to dig up the name of the function
  to which the address belongs to
* ZeroPtrCall now uses libtrace to find which program and function
  caused the error. The output is to vo_stderr (UART)
* Added libraries to control MP3 playing externally
* Support for MP3 and OGG encoding via external libraries and test ap3
* Added file output buffering for smooth mp3 recording to SD card
* Added dynamic creation of new tasks, using allocXY to get new stack areas.

Updates in version 3.12 - 2014-08-25
* Exported the interface for AddSymbol, GetSymbolAddress thus making
  the global namespace expandable. Allows sharing symbols such as
  functions or variables between different applications (DLLIMPORT).

Updates in version 3.11 - 2014-06-27
* Added modelCallbacks array for finding the (decoder) model(s) so that
  different processes can control the music playback

Updates in version 3.10 - 2014-05-27
* Ability to import and export named symbols in libraries
* Support for external relocations
* Fixed error in VSOS restart after app exit
* Fixed a FAT fragment allocation issue
* Standardized method for parameter passing from one app to another

Updates in version 3.09 - 2014-05-07
* Added long file name support for opening files and directories
* Updated the interface for file searching
* Fixed interrupt vectors when reloading a running kernel
* Added SystemUiMessage interface for sending global UI messages
* Added master volume control handler to SystemUiMessage

Updates in version 3.08 - 2014-05-02
* Updated the SD-card version of kernel
* Some improvements in kernel debug messages

Updates in version 3.07 - 2014-04-25
* Added support for internal flash.
* Combined the drivers for internal and external flash
* Updated device driver calling, should be final now: 
  - init() is called when the driver is loaded for the first time
  - main(char *parameters) is called for each line of S:CONFIG.TXT
  - fini() is called if the device driver is dropped from memory
* Added proper support for stderr, vo_stderr
  - At boot stdout and stderr point to UART serial port (115200 bps).
  - if lcd console driver is loaded, stdout will go to lcd screen,
    but writes to stderr will continue to go to the UART.
* Added support for soft serial port console (if UART is needed for other
  purpose)
* Increased stack size for main task (now 512 doublewords)
* Fixed an interrupt bug in scheduler
* Fixed flash boot issue
* Fixed power button reset issue
* Fixed generic interrupt support
* Added support for SIMPLE_FILE and SIMPLE_DEVICE drivers
* Fixed a re-entrancy issue in fopen, fclose. Some problems may still exist.
* Started to add some long file name support, now mainly for f->Identify() 
* Experimenting with support for one "*" wildcard inside file name for FAT
  fopen, it will match the first file or skip n files if mode "rb#n" is used. 
* Increased the number of available file handles
* File buffer allocation now uses malloc instead of fixed buffers
* Improved USB support for the internal and external flash
* Added a safety trap for null device writes
* Changed kernel version numbering


IMPORTANT NOTE:
VSOS3 is a major update from the VSOS2 kernel. When the kernel boots,
it has only a minimal set of device drivers. The console will use
UART and nothing will be printed to the LCD screen.

To see the VSOS3 booting, use a terminal (HyperTerminal, Termite,...)
or from VSIDE see Project->Start Serial Port Monitoring.

For "normal" operation, you need to have VSOS3 device drivers (*.DL3)
in S:SYS directory and CONFIG.TXT in the S: root. If you load the
device drivers LCD288, LCDCON, TOUCH288, STDBTCH and AUDIO, then this
kernel will work much like the old VSOS2. 

VSOS3 does not run VSOS2 *.APP Applications. Old applications must be
recompiled to VSOS3's *.AP3 applications. The benefit from using AP3
applications is that applications can call and run other applications
and use shared dynamically loaded *.DL3 libraries.
 



	* Installation:
	- Option 1) Build the kernel and use VSIDE menu "Project" -> 
	"Prommer/Flasher Utility" -> "VS1005G External SPI Flash Prommer"
	to burn the eeprom.img to the developer board. Remember to keep
	the "BOOT SELECT" pressed when you press "RESET" to reset the board
	so that you can write to the external SPI Flash. After promming
	press "Cancel" to exit from the prommer.
	- Option 2) Download the "vs1005g-firmware-preparations.zip" from
	thread "Flashing a new or bricked VS1005G board" in the Forum,
	/* http://www.vsdsp-forum.com/phpbb/viewtopic.php?f=13&t=681 */
	replace the eeprom.img with this one and follow the .txt instructions.
	- Check that you see the new version number  on screen 
        when you boot the card.

	** IMPORTANT **
	- To compile applications that use the updated API, solution will
	copy the vsos02.o file to your vside\libvs1005g folder.
	It requires to run VSIDE as administrator or the copying will fail.
	
	
	REMEMBER TO UPDATE ALSO THE APP FILES IN YOUR SD CARD


Updates for version 0.241 (August 2013)
	- Fixed touch.c because 0.24 modifications made the touchscreen
	  unreliable
	- Combined all LCD drivers into the source tree, you can
	  select one LCD driver from the options in lcd.c
	- Added the UART LCD emulator VSOSLcdEmulator.exe Windows binary to the
	  source files
	- Reverted some of the StdButton visual complexity to improve speed
	- Added a return value to LcdInit so the lcd driver can tell if it's 
	  a normal display or a simple/small display (ask to limit drawing
	  complexity)


Updates for version 0.24 (July 2013):
	- SD driver using native SD hardware (seems to be fast and reliable)
	- Resource protection (not many drivers use it yet)
	- Audio device driver now supports digital out
	- Updated FAT
	- Updated visual style of StdButtons
	- Moved some Lcd functions such as LcdTextOutXY to lcdfunctions.c

Updates for version 0.23 (December 2012):

	- Improved the real-time scheduler, fixed AllocMemY
	- Improved kernel module calling (added SysCall)
	- Fixed power-button delayed reset issue
	- Added colorscheme support also for console applications
	- Added the capability to prom the OS kernel into VS1005G internal flash
	
Updates for version 0.22 (October 2012):

	- Added Standard Audio interface (stdaudioin, stdaudioout)
	- Removed old MP3 decoder interface, obsoleted by Standard Audio
	  interface
	- Updated the MP3 decoder example template to use the new interface
	- Added loadable kernel modules
	- Added generic user interface module API to allow changing the display
	  and input devices without recompiling applications
	- Added Serial port speed adjustment interface to the kernel
	- Added Clock speed adjustment interface to the kernel
	- Checked and fixed standard input/output macros (vo_stdio.h)
	- Added timing services to the kernel
	- Added power control services to the kernel
	- Fixed some template build issues when running VSIDE under Linux
	- Fixed system state issues when restarting the OS after app exit
	- Updated fopen to manage unbuffered files better
	- Fixed INIT.APP main menu template 5th button hang issue
	- Added several example templates
	- Fixed several small issues

Updates for version 0.21 (August 2012): 

	- Fixed FAT reads: due to a small mistake, fread from a file in 0.20
	  incorrectly read past end of file to the end of the last cluster,
	  which sometimes caused incorrect loading of an APP file, resulting
	  in corrupted functions and random failures of APP. This is now fixed.	
	- Fixed FAT writes for multi-cluster writes. Writing large configuration
	  files etc to the SD card should work better now.
	/// \todo Seeking past the end of file doesn't work. If you want to
	  make a file of e.g. 1 megabyte, you must first write a megabyte of
	  empty data into the file. Then you can seek within the megabyte.
	  This will be fixed in the future.
	- Fixed the "SD card in SPI mode" driver a little. It is more
	  robust now. Still writes to some SD cards MAY fail occasionally,
	  so I'd advise to write files only when really needed for the time
	  being. An upcoming "SD card in SD mode" driver will fix this.
	  It will also improve the write speeds a lot, the current driver's
	  write speed is something like 100 kilobytes per second.
	- Included vo_filesystems in the API to allow custom filesystems.



Info for version 0.20:

	This is VSOS Kernel 0.20 for VS1005G. It is preinstalled in VS1005G 
	developer boards manufactured in July 2012.
	
	The kernel's display driver comes from the file lcd-ili9341.c. To
	target another display, you need to modify that C file or provide
	another C file instead of it. The touch screen is handled by touch.c. 
	To target another touch screen, please modify touch.c.
	
	It is no longer necessary to recompile applications after changing
	the kernel. Any version 0.2 application should be able to run with any
	version 0.2 kernel, provided that all the required functions 
	are present. The interface between the application and kernel is
	defined in file vsos02.abs which is the source code for vsos02.o.
	Any modifications to vsos02.abs should be done only with great care.	

	To install this kernel into a new board, you must compile it
	and burn the resulting eeprom.img to SPI flash.
	
	Additionally, early VS1005G devices require the file STARTEXTFLASH.NAN
	to be burned into the NAND flash of the developer board to allow 
	booting from the SPI flash. Please see the forum for details.

	The VSOS Kernel and VS1005 software is a work in progress. There is
	not much documentation yet. We will continue writing it and publish
	at www.vlsi.fi and www.vsdsp-forum.com. We are also looking forward
	to hearing about your successes and problems in the forum.
	
	Please refer all questions about this software to the forum,
	and not to VLSI support until we get some more experience and more
	software running on the VS1005
	
	Have fun coding and designing your products!
	Panu-Kristian Poiksalo,
	VLSI Solution Oy, July 2012


	The VSOS Operating System is original work of VLSI Solution Oy.
	Panu-Kristian Poiksalo is the main author of the kernel, 
	device drivers, kernel data structures, calling conventions, 
	model-view-controller interface, relocating loader and linker. 
	Henrik Herranen has created the Standard Audio Interface and
	the MP3 decoder model for VSOS.
	Pasi Ojala has provided the linker, RTOS and task switching.
	Lasse Makkonen has written the C compiler and demo code.

	Copyright (C) 2012 VLSI Solution Oy, Tampere, Finland.	
	This source code is provided free of charge for use in products
	that contain VLSI Solution's integrated circuits. The source
	code is provided without any warranty whatsoever. This software
	is work in progress and may contain bugs. Please report bugs to the
	VSOS Software Section of VSDSP Forum or to VLSI Solution Oy directly.
