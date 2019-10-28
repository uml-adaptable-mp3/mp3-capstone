
              CRC32 v1.02
              -----------
        for VSOS 3.50 or higher
       2017-12-05  VLSI Solution

This programs calculates a CRC32 sum for a file, using the same formula
as is used for Ogg files. 32-bit CRC is enough for checking file integrity
against accidental change, but not against intentional tampering.

Included is a source code file crc32_for_pc.c which implements the
same algorithm in standard C that can be compiled to a PC.
This way file integrity can be checked between platforms.




Usage: Crc32 [-v|+v|-i|+i|-h] file1 [...]
-v|+v   Verbose on|off
-i|+i   Ignore CRC32 calculation on|off
-h      Show this help




Version History:
2017-12-05 HH v1.02 - Ported to VS1005h.
2017-02-23 HH v1.01 - Added command line options.
2016-11-30 HH v1.00 - First release.
