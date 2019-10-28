
             DISKFREE v1.07
             --------------
        for VSOS 3.52 or higher
       2018-01-31  VLSI Solution




DiskFree displays (or, when called from C, returns) the amount of free
space on a FAT device, or on all available devices.

To use, either compile the Solution, or simply copy DiskFree.dl3
to your VS1005 Evaluation Board's SYS/ folder.




The VSOS Shell command line options are:

Usage: DiskFree [-p x|-m x|-v|-h] [D:]
D:      Print results for drive D (:: means all drives)
        There may be multiple drive parameters.
        Drive parameters must be after -p/-m options.
-p x    Store last result in KiB to X-memory 32-bit pointer pointed to by x
        (Usable for calling with RunLibraryFunction(); makes operation silent)
-m x    Look only until x MiB (faster operation)
-v      Verbose (giving twice makes even more verbose)
-h      Show this help

Without a parameter, DiskFree displays the status of the current directory
drive.



From C code, the following calling convention may be used:

#include <apploader.h>

#define DRIVE 'S'
{
  char command[14];
  u_int32 spaceInKiB = 0;
  sprintf(command, "-p 0x%04x %c:", &spaceInKiB, DRIVE);
  if (RunProgram("DiskFree", command) == S_OK) {
    printf("Drive %c: has %ld KiB free space\n", DRIVE, spaceInKiB);
  } else {
    printf("DiskFree failed for drive %c:\n", DRIVE);
  }
}




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2018-01-31 HH v1.07 - New extra verbose "-v -v" option shows cluster map.
2018-01-24 HH v1.06 - Corrected results with -m option.
2017-12-05 HH v1.05 - Ported to VS1005h.
2017-04-24 HH v1.04 - totalSize corrected, compatible with VSOS 3.42 and newer.
                      Will give slightly incorrect info with older VSOS.
2017-03-22 HH v1.03 - Corrected documentation for C code call.
2015-10-01 HH v1.02 - Added -v option for additional information.
2015-09-22 HH v1.01 - Speed increased a lot for FAT32 (3-8x faster).
2015-09-22 HH v1.00 - First release.
