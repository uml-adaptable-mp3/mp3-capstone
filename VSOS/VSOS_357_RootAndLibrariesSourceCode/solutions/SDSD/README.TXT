
  SD mode SD driver SDSD.DL3, SDSDR.DL3, SDSDX.DL3, SDSDMONO.DL3
  --------------------------------------------------------------
                    for VSOS 3.52 or higher
                   2018-01-31  VLSI Solution




These SD drivers are divided into four files.

SDSD.DL3 is the full-fledged SD driver that uses SDSDX.DL3 as a helper.

SDSDR.DL3 is a read-only version of the SD driver that uses SDSDX.DL3 as
a helper. The benefit of the driver is that it is smaller than the
full-fledged SD driver.

SDSDX.DL3 contains startup routines, and is called by both SDSD.DL3 and
SDSDR.DL3 when they start up, or there is a problem with the SD card, or if
the SD card is replaced.

SDSDMONO.DL3 is a monolithic version of the driver that doesn't use helper
drivers.

To start the full SD driver, put the following line to your config.txt file:
SDSD D
where D is the drive letter for your SD drive. You can use drive letters
from A to Z, except S (which is reserved for the system drive). If you
intend to replace the S driver, you need to use the monolithic driver:
SDSDMONO S

To start the read-only SD driver, put the following line to your config.txt
file:
SDSDR D
where D is the drive letter for your SD drive. You can use drive letters
from A to Z, except S (which is reserved for the system drive).

Normally the SD driver tries to activate 4-bit data mode. If you have a
board without DAT1, DAT2, and DAT3, or where they may be broken, you
can force the driver to start in the slower 1-bit data mode by putting
'1' on the startup line in config.txt, e.g.:
SDSD D 1

When the SDSD.DL3 or SDSDR.DL3 driver starts, it will initialize the SD card
using the SDSDX.DL3 help library. After that, SDSDX.DL3 is removed from
memory, and will only be temporarily loaded when there are issues with the
card, or if the card is removed / replaced.

The benefit of using the new drivers is that they require less memory then
the old, monolithic SD driver (except when starting up). The required
instruction space is as follows:

32-bit
words  Description
-----+-------------------------------------------------------------------
1165 | SDSD.DL3 (2016-12-14 version).
 876 | SDSDR.DL3 (2016-12-14 version).
1177 | SDSDX.DL3 (2016-12-14 version) initialization library.
2342 | SDSD.DL3 + SDSDX.DL3 (2016-12-14 version) when doing initialization.
2053 | SDSDR.DL3 + SDSDX.DL3 (2016-12-14 version) when doing initialization.
1954 | SDSDMONO.DL3 (2016-12-14 version).
-----+-------------------------------------------------------------------




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2018-01-31 HH v1.16 - Compatibility with MMC cards enhanced.
                      Added autorevert to 1-bit bus if 4-bit bus isn't working.
2017-12-05 HH v1.15 - Ported to VS1005h.
2017-01-30 HH v1.14 - Regression: Init wait time was too fast for some SD cards,
                      fixed.
2016-12-14 HH v1.13 - Better eMMC support: can read Extended CSD + use 4-bit
                      mode.
2016-12-07 HH v1.12 - Now aware of clock speed; keeps bus speed always <= 25MHz.
2016-09-01 HH v1.11 - default drive letter 'D' would not be set, fixed.
2015-10-13 HH v1.10 - New '1' option forces 1-bit mode instead of 4-bit mode.
2015-09-04 HH v1.09 - Removes gracefully from memory.
2015-04-17 HH v1.08 - Modified mainBase.c to allow unloading / reloading driver.
2015-04-08 HH v1.07 - Drivers now compile from same sources using #ifdef
                      directives.
2015-03-10 HH v1.06 - Added SDSDMONO.DL3 so that system drive may be changed.
2015-01-12 HH v1.05 - Fixed recovery after removing and reinserting an SD card.
2014-12-08 HH v1.04 - Created a smaller read-only version, SDSDR.DL3.
2014-12-05 HH v1.03 - Divided into two parts, SDSD.DL3 and SDSDX.DL3.
2014-11-24 HH v1.02 - Added optional write CRC checking code, released new
                      driver.
2014-11-21 HH v1.01 - Corrected MMC card size reporting + closing Multiple Block
                      Write.
2014-11-20 HH v1.00 - Corrected bug that caused write errors on some SD cards.
