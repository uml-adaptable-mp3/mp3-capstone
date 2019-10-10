
        Reverb Generator FtReverb v1.12
        -------------------------------
            for VSOS 3.55 or higher
           2019-04-10  VLSI Solution




The FtReverb package offers a reverb echo generator that connects to
stdout.

To use the package, copy the .DL3 files to your VS1005 system disk SYS/
folder, then follow the instructions in this manual. The reverb
generator can be accessed both from C programs using ioctl() calls, or
from the VSOS Shell command line.

The package consists of the following programs:

Name      Description
FTOREV    Reverb generator driver that connects to stdaudioout
FTOREV23  Reverb generator driver that connects to stdaudioout and uses
          VS23S010 or VS23S040 memory IC for extended storage
SETREV    User program to set parameters for the reverb generator

Features and limitations:
- Optimized for 48 kHz operation
- Requires a maximum of 32 / 48 MIPS at 48 kHz 16 bits (FTOREV / FTOREV23)

For additional information, see the VS1005 VSOS Audio Subsystem document,
Chapter Reverb Generator Audio Drivers.



Starting FTOREV.DL3 in config.txt and setting room size:
# Starts reverb generator and connects to stdaudioout
FTOREV
# Set room size to 6 meters
RUN SETREV -s600
# Start automatic audio loopback driver
AUXPLAY

Note: FTOREV is loaded and stays in memory. Because SETREV doesn't
need to stay in memory, it is started with the RUN command.


Starting FTOREV.DL3 from the VSOS Shell command line and setting
similar parameters:
S:>driver +ftorev
S:>setrev -s600 -v
ROOM:
  (-s) Room size       :    600 cm (1-32767)
  (-t) Reverb time     :   1000 ms (1-65535)
  (-f) Room softness   :  32767    (0=hard-65535=soft)
  (-d) Dry gain        :    768    (0-32767, 1024=1)
  (-w) Wet gain        :    256    (0-32767, 1024=1)
       Sample rate     :  48000 Hz
       Delay pairs     :      8
       Ext. mem. size  :      0 bytes
       Ext. mem. read  : 0x0000
       Ext. mem. write : 0x0000
S:>driver +auxplay

To monitor any changes you do with SetRev, you might want to start
the automatic background playback friver AUXPLAY:
S:>driver +auxplay


Unloading the FTOREV.DL3 driver using the VSOS Shell:
S:>driver -ftorev


Starting FTOREV.DL3 using C code:
  // Change name as necessary
  #define LIB_NAME "ftorev"

  u_int16 *revLib = NULL;
  FILE *revFP = NULL;

  revLib = LoadLibrary(LIB_NAME);
  if (!revLib) {
    printf("Cannot load " LIB_NAME ".DL3 library\n");
    goto finally;
  }
  revFP = stdaudioout;

Note that if you have started FTOREV from config.txt, you don't need
to open it from your software, but instead you can directly send ioctl()
calls to stdaudioout.



Closing the driver:
  finally:
  if (revFP) {
    /* Close file */
    RunLoadedFunction(i2sLib, ENTRY_4, (void *)revFP);
    revFP = NULL;
  }
  if (revLib) {
    DropLibrary(revLib);
    revLib = NULL;
  }




Running SETREV from the VSOS Shell:
To set room reverberation time to 2 seconds:
S:>setrev -t2000

To see the current reverb parameters:
S:>setrev
ROOM:
  (-r) First reflection:     20 ms
  (-s) Room size       :    600 cm (1-32767)
  (-t) Reverb time     :   1000 ms (1-65535)
  (-f) Room softness   :  32767    (0=hard-65535=soft)
  (-d) Dry gain        :    768    (0-32767, 1024=1)
  (-w) Wet gain        :    256    (0-32767, 1024=1)
       Sample rate     :  48000 Hz
       Delay pairs     :      7
       Ext. mem. size  : 524288 bytes
       Ext. mem. read  : 0x456b FTOREV23::ReadVS23S0x0
       Ext. mem. write : 0x4575 FTOREV23::WriteVS23S0x0

To see the help page:
S:>setrev -h
Usage: SetRev [-i|-o] [-v|+v] [-sx] [-h]
-i      Set stdaudioin
-o      Set stdaudioout (default)
-sx     Set room size to x cm (200-1200 recommended)
-tx     Set reverb Time to x ms (100-5000 recommended)
-fx     Set room wall soFtness (0 = hard, 65535 = soft)
-dx     Set Dry gain (0-65535, 1024 = 1)
-wx     Set Wet gain (0-65535, 1024 = 1)
-v|+v   Verbose on/off
-h      Show this help

Note:
It is recommended that Dry gain + Wet gain would not be much
over 1024 to avoid distortion.




Data structures:

If using an earlier version than VSOS 3.60, the following data structures
are not yet in aucommon.h, so you need to add this to your source code:

#ifndef IOCTL_AUDIO_GET_REV_ROOM
#define IOCTL_AUDIO_GET_REV_ROOM	280
#define IOCTL_AUDIO_SET_REV_ROOM	281

struct Room {
  /* These fields filled in by the caller of DesignRoom() */
  u_int16 firstReflectionMS; /* How many milliseconds before first reflection */
  u_int16 roomSizeCM;	/* Room size in cm */
  u_int16 reverbTimeMS;	/* Room reverb time in ms = -60 dB attenuation time */
  u_int16 softness;	/* Room softness, 0 = hard, 65535 = soft */
  u_int32 sampleRate;	/* Sample rate used for making room design */
  s_int16 dryGain;	/* 1024 = 1 */
  s_int16 wetGain;	/* 1024 = 1 */
  s_int16 (*Read )(register       u_int16 *d, register u_int32 addr, register u_int16 words); /* Function to read from external memory */
  s_int16 (*Write)(register const u_int16 *d, register u_int32 addr, register u_int16 words); /* Function to write to external memory */
  u_int32 memSizeWords; /* External memory size */
  /* Rest of the fields filled in by the room designer */
  s_int16 delays;	/* How many delays per channel used */
};

#endif /* !IOCTL_AUDIO_GET_REV_ROOM */




IOCTL Controls:
  All ioctl controls except those listed here are forwarded to the underlying
  audio driver.

  Read room parameters:
    s_int16 t;
    struct Room *room;
    if ((t = ioctl(fp, IOCTL_AUDIO_GET_REV_ROOM, (void *)(room))) >= 0) {
      printf("Room size       : %5u cm (1-32767)\n", myRoom.roomSizeCM);
      printf("Reverb time     : %5u ms (1-65535)\n", myRoom.reverbTimeMS);
      printf("Softness        : %5u    (0=hard-65535=soft)\n", myRoom.softness);
      printf("Dry gain        : %5d    (0-32767, 1024=1)\n", myRoom.dryGain);
      printf("Wet gain        : %5d    (0-32767, 1024=1)\n", myRoom.wetGain);
      printf("Sample rate     : %5ld Hz\n", myRoom.sampleRate);
      printf("Delay pairs     : %5d\n", myRoom.delays);
    } else {
      printf("E: Cannot read room parameters\n");
    }

  Set room parameters:
    struct Room *room;
    if (ioctl(fp, IOCTL_AUDIO_SET_REV_ROOM, (void *)(room)) < 0) {
      printf("E: Cannot set room parameters\n");
    }




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
No guarantee is given for the usability of this code.


Version History:
2019-04-10 HH v1.12 - Corrected output with -h option.
2018-04-25 HH v1.11 - Bug fix: Playback and setting up can now safely be done
                      by different processes (e.g. playback with AUXPLAY while
                      using SETREV on the command line to set parameters.)
2018-04-24 HH v1.10 - FTOREV23 added, -r option added to SETREV.
2018-04-05 HH v1.00 - First release for stdaudioout.
