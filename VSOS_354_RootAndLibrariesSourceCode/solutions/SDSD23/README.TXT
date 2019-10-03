
            SD driver set using VS23S0x0 as buffer
     SDSD23.DL3, SDSDR23.DL3, SDSDX23.DL3, SDSDMONO23.DL3
     ----------------------------------------------------
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
                      Made version numbering compatible with parent SDSD driver.
2017-12-05 HH v1.02 - Ported to VS1005h.
2017-10-04 HH v1.01 - Changed output to show which VS23S0x0 chip detected.
2017-03-30 HH v1.00 - First release, branched from driver SDSD.DL3 & company.
