
        Equalization Filter FtEqu 1.06
        ------------------------------
           for VSOS 3.50 or higher
          2017-12-05  VLSI Solution




The FtEqu package offers upto 16 channels of high-quality equalization
filtering. To use the package, copy the .DL3 files to your VS1005
system disk SYS/ folder, then follow the instructions in this manual.
The equalizer can be accessed both from C programs using ioctl() calls,
or from the VSOS Shell command line.

The package consists of the following programs:

Name    Description
FTOEQU  Equalization driver that connects to stdaudioout
SETEQU  User program to set parameters for the equalizer

Each stereo filter channel requires approximately 2.5 MIPS at 48 kHz,
or 5 MIPS at 96 kHz. If more than one filter channels are used, it
may be required to use a higher clock speed than VS1005's default
60 MHz. Clock speed can be set e.g. with VSOS Shell command SETCLOCK.




Starting FTOEQU.DL3 in config.txt and setting a bass gain filter as
the first filter:
# Starts equalizer to stdaudioout
FTOEQU
# Set L+R @ 300 Hz to +12 dB, Q factor 0.7
RUN SETEQU 1 3 130 12 0.7

Note: FTOEQU is loaded and stays in memory. Because SETEQU doesn't need to
stay in memory, it is started with the RUN command.


Starting FTOEQU.DL3 from the VSOS Shell command line and setting a bass
gain filter as the first filter:
S:>driver +ftoequ
S:>setequ 1 3 130 12 0.7


Unloading the FTOEQU.DL3 driver using the VSOS Shell:
S:>driver -ftoequ


Starting FTOEQU.DL3 using C code:
  // Change name as necessary
  #define LIB_NAME "ftoequ"

  u_int16 *equLib = NULL;
  FILE *equFP = NULL;

  equLib = LoadLibrary(LIB_NAME);
  if (!equLib) {
    printf("Cannot load " LIB_NAME ".DL3 library\n");
    goto finally;
  }
  equFP = stdaudioout;

Note that if you have started FTOEQU from config.txt, you don't need to
open it from your software, but instead you can directly send ioctl()
calls to stdaudioout.



Closing the driver:
  finally:
  if (equFP) {
    /* Close file */
    RunLoadedFunction(i2sLib, ENTRY_4, (void *)equFP);
    equFP = NULL;
  }
  if (equLib) {
    DropLibrary(equLib);
    equLib = NULL;
  }




Running SETEQU from the VSOS Shell:
From the VSOS Shell, you can e.g. Set L+R @ 300 Hz to +12 dB, Q factor 0.7:
S:>setequ 1 3 130 12 0.7

To read the status of the first filter:
S:>setequ 1

To clear the first filter:
S:>setequ 1 0

To read all filters:
S:>setequ

For more options:
S:>setequ -h




Data structures:

If using an earlier version than VSOS 3.23, the following data structures
are not yet in aucommon.h, so you need to add this to your source code:

#ifndef IOCTL_AUDIO_GET_EQU_FILTER
#define IOCTL_AUDIO_GET_EQU_FILTER	233
#define IOCTL_AUDIO_SET_EQU_FILTER	234
#define IOCTL_AUDIO_GET_EQU_MAX_FILTERS	235

#define FLT_EQUF_LEFT	1
#define FLT_EQUF_RIGHT	2

struct FilterEqualizer {
  s_int16 filterNumber;	/* 0 .. MAX_FILTERS-1 */
  u_int16 flags;	/* FLT_EQUF_ flags */
  double centerFrequencyHz;
  double gainDB;        /* Recommended -12.0 .. +12.0 dB */
  double qFactor;       /* Recommended 0.1 .. 4.0 (higher is steeper) */
};

#endif /* !IOCTL_AUDIO_GET_EQU_FILTER */




IOCTL Controls:
  All ioctl controls except those listed here are forwarded to the underlying
  audio driver.

  Read how many filters are available. If no equalizer in path, S_ERROR will
  be returned.
  Example:
    s_int16 n = ioctl(equFP, IOCTL_GET_EQU_MAX_FILTERS, NULL);

  Read status of one equalizer filter. Example shows how to read filter 0.
    struct FilterEqualizer fltEqu;
    fltEqu.filterNumber = 0; /* 0 .. MAX_FILTERS-1 */
    if (ioctl(fp, IOCTL_AUDIO_GET_EQU_FILTER, (void *)(&fltEqu)) == S_OK) {
      printf("%2d: flags 0x%04x, %10.2f Hz, %6.2f dB, Q %5.2f\n",
             fltEqu.filterNumber+1, fltEqu.flags, fltEqu.centerFrequencyHz,
             fltEqu.gainDB, fltEqu.qFactor);
    } else {
      printf("%2d: ERROR\n", fltEqu.filterNumber+1);
    }

  Set first equalizer filter; both to left and right audio channel:
    struct FilterEqualizer fltEqu;
    fltEqu.filterNumber = 0;
    fltEqu.flags = FLT_EQUF_LEFT | FLT_EQUF_RIGHT;
    fltEqu.centerFrequencyHz = 130.0;
    fltEqu.gainDB = 12.0;
    fltEqu.qFactor = 0.7;
    ioctl(fp, IOCTL_AUDIO_SET_EQU_FILTER, (void *)(&fltEqu));
  (Note: to disable a filter, set fltEqu.flags to 0.)



Example values for basic bass/treble control:
  We set the bass control to 130 Hz, XXX dB, Q value 0.7.
  We set the treble control to 10 kHz, YYY dB, Q value 0.7.
  Note: Replace XXX / YYY with actual values!

  Using config.txt:
    run setequ 1 3   130 XXX 0.7
    run setequ 2 3 10000 YYY 0.7

  Using VSOS Shell:
    setequ 1 3   130 XXX 0.7
    setequ 2 3 10000 YYY 0.7

  Using C code:
    struct FilterEqualizer fltEqu;

    fltEqu.filterNumber = 0;
    fltEqu.flags = FLT_EQUF_LEFT | FLT_EQUF_RIGHT;
    fltEqu.centerFrequencyHz = 130.0;
    fltEqu.gainDB = XXX;
    fltEqu.qFactor = 0.7;
    ioctl(fp, IOCTL_AUDIO_SET_EQU_FILTER, (void *)(&fltEqu));

    fltEqu.filterNumber = 1;
    fltEqu.flags = FLT_EQUF_LEFT | FLT_EQUF_RIGHT;
    fltEqu.centerFrequencyHz = 10000.0;
    fltEqu.gainDB = YYY;
    fltEqu.qFactor = 0.7;
    ioctl(fp, IOCTL_AUDIO_SET_EQU_FILTER, (void *)(&fltEqu));




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
No guarantee is given for the usability of this code.


Version History:
2017-12-05 HH v1.06 - Ported to VS1005h.
2017-03-14 HH v1.05 - Corrected memory corruption bug when used with AUODAC.
2016-02-01 HH v1.04 - Mainly documentation update.
2015-09-09 HH v1.03 - Accidentally made AUODAC.DL3 audio buffer small, fixed.
2015-09-04 HH v1.02 - Could only set first channel properly, fixed.
2015-08-27 HH v1.00 - First release for stdaudioout.
