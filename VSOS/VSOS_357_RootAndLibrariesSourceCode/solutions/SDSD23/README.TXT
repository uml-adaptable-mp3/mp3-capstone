
            SD driver set using VS23S0x0 as buffer
            SDSD23.DL3, SDSDR23.DL3, SDSDX2323.DL3
            --------------------------------------
                    for VSOS 3.56 or higher
                   2018-08-28  VLSI Solution




These SD drivers are divided into three files. They use VS23S0x0 for
write buffering. Upon startup they detect whether there is 1, 2, 3, or 4
1 Mbit SRAM VS23S010 ICs, or alternatively a 4 MBit SRAM VS23S040 IC
connected.

SDSD23.DL3 is the full-fledged SD driver that uses SDSDX23.DL3 as a helper.

SDSDR.DL3 is a read-only version of the SD driver that uses SDSDX23.DL3 as
a helper. The benefit of the driver is that it is smaller than the
full-fledged SD driver.

SDSDX23.DL3 contains startup routines, and is called by both SDSD23.DL3 and
SDSDR.DL3 when they start up, or there is a problem with the SD card, or if
the SD card is replaced.

SDSDMN23.DL3 is a monolithic version of the driver that doesn't use helper
drivers.

To start the full SD driver, put the following line to your config.txt file:
SDSD23 D
where D is the drive letter for your SD drive. You can use drive letters
from A to Z, except S (which is reserved for the system drive). If you
intend to replace the S driver, you need to use the monolithic driver:
SDSDMN23 S

When the SDSD23.DL3 or SDSDR.DL3 driver starts, it will initialize the SD card
using the SDSDX23.DL3 help library. After that, SDSDX23.DL3 is removed from
memory, and will only be temporarily loaded when there are issues with the
card, or if the card is removed / replaced.




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2018-08-28 HH v1.18 - VS23S0x0 test pattern slightly changed so that it checks
                      against bits stuck to 0 or 1.
2018-03-15 HH v1.17 - Corrected documentation, driver is unchanged.
2018-01-31 HH v1.16 - Compatibility with MMC cards enhanced.
                      Added autorevert to 1-bit bus if 4-bit bus isn't working.
                      Made version numbering compatible with parent SDSD driver.
2017-12-05 HH v1.02 - Ported to VS1005h.
2017-10-04 HH v1.01 - Changed output to show which VS23S0x0 chip detected.
2017-03-30 HH v1.00 - First release, branched from driver SDSD.DL3 & company.
