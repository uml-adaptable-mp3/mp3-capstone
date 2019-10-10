
              FORMAT v1.03
              ------------
        for VSOS 3.52 or higher
       2018-01-22  VLSI Solution




FORMAT formats a disk drive to FAT32 format. It particularly makes it
possible to use 64 GB and larger devices like SD cards and USB memory
sticks.

When formatting 64 GB or larger SD cards or USB Sticks using current
Microsoft Windows OS versions, they are always formatted using the
exFAT file system which VSOS doesn't support. With FORMAT, it is
possible to also use these memory devices under VSOS. They will also
work when used with Microsoft Windows, as long as you don't reformat
them to exFAT.

Note: Formatting a device will remove all files from it.




The VSOS Shell command line options are:

Usage: Format x: [-v|+v|-llabel|-sx|-cx|-fx|-ix|-p|-px|-F|-y|+y|-h]
x:      Drive name
-v|+v   Verbose on/off
-llabel Set disk label
-sx     Force size to x MiB
-cx     Force cluster size to x 512-byte sectors
-fx     Set number of FATs (1 or 2)
-ix     Set 32-bit serial number volume ID to x
-p|+p   Make/don't make partition table
-px     Reserve x MiB for partition table
-F|+F   Force / don't force making file system even if illegal
-y|+y   Don't ask / Ask for confirmation
-n|+n   Dry run (don't actually write) on/off
-a|+a   Erase (very slow) / Don't erase all data
-h      Show this help




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2018-01-22 HH v1.03 - Made it possible to format disks that are originally in
                      unknown format, like exFAT.
2017-12-05 HH v1.02 - Ported to VS1005h.
2017-05-02 HH v1.01 - Minor fixes + size optimization.
2017-04-27 HH v1.00 - First release.
