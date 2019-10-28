
        Pitch and Speed Shifter FtPitch v1.01
        -------------------------------------
               for VSOS 3.50 or higher
              2017-12-05  VLSI Solution




The FtPitch package offers a pitch and speed shifter that connects to
stdout. The speed shifter can nominally be controlled to speeds between
0.68x and 1.64x of realtime, and the pitch shifter can nominally be
controlled between 0.61x and 1.47x of normal pitch. To use the package,
copy the .DL3 files to your VS1005 system disk SYS/ folder, then follow
the instructions in this manual. The pitch shifter can be accessed both
from C programs using ioctl() calls, or from the VSOS Shell command
line.

The package consists of the following programs:

Name      Description
FTOPITCH  Pitch/Speed shifter driver that connects to stdaudioout
SETPITCH  User program to set parameters for the shifter

Features and limitations:
- Speed divided by pitch (speed/pitch) must be between 0.68 and 1.64.
- Pitch shifting alters sample rate. If the resulting sample exceeds
  96 kHz, playback will be at incorrect speed.
- The shifter has been optimized to work best for audio where sample
  rate is between 32 and 48 kHz.




Starting FTOPITCH.DL3 in config.txt and setting playback speed to
0.8x of normal and pitch to 1.1x of normal:
# Starts pitch/speed shifter to stdaudioout
FTOPITCH
# Set speed to 0.8x and pitch to 1.1x
RUN SETPITCH -s0.8 -p1.1

Note: FTOPITCH is loaded and stays in memory. Because SETPITCH doesn't
need to stay in memory, it is started with the RUN command.


Starting FTOPITCH.DL3 from the VSOS Shell command line and setting
similar parameters:
S:>driver +ftopitch
S:>setpitch -s0.8 -p1.1


Unloading the FTOPITCH.DL3 driver using the VSOS Shell:
S:>driver -ftopitch


Starting FTOPITCH.DL3 using C code:
  // Change name as necessary
  #define LIB_NAME "ftopitch"

  u_int16 *pitchLib = NULL;
  FILE *pitchFP = NULL;

  pitchLib = LoadLibrary(LIB_NAME);
  if (!pitchLib) {
    printf("Cannot load " LIB_NAME ".DL3 library\n");
    goto finally;
  }
  pitchFP = stdaudioout;

Note that if you have started FTOPITCH from config.txt, you don't need
to open it from your software, but instead you can directly send ioctl()
calls to stdaudioout.



Closing the driver:
  finally:
  if (pitchFP) {
    /* Close file */
    RunLoadedFunction(i2sLib, ENTRY_4, (void *)pitchFP);
    pitchFP = NULL;
  }
  if (pitchLib) {
    DropLibrary(pitchLib);
    adcLib = NULL;
  }




Running SETPITCH from the VSOS Shell:
To set playback speed to 0.8x of normal and pitch to 1.1x of normal:
S:>setpitch -s0.8 -p1.1

To see the current pitch and speed:
D:>setpitch
Pitch 1.100
Speed 0.800

S:>setpitch -h




Data structures:

If using an earlier version than VSOS 3.42, the following data structures
are not yet in aucommon.h, so you need to add this to your source code:

#ifndef IOCTL_AUDIO_GET_PITCH
#define IOCTL_AUDIO_GET_PITCH		270
#define IOCTL_AUDIO_SET_PITCH		271
#define IOCTL_AUDIO_GET_SPEED		272
#define IOCTL_AUDIO_SET_SPEED		273
#endif /* !IOCTL_AUDIO_GET_SPEED */




IOCTL Controls:
  All ioctl controls except those listed here are forwarded to the underlying
  audio driver.

  Read playback pitch:
    u_int16 t;
    if ((t = ioctl(fp, IOCTL_AUDIO_GET_PITCH, NULL)) != 0xFFFFU) {
      nothingToDo = 0;
      printf("Pitch %5.3f\n", t*(1.0/16384.0));
    } else {
      printf("Cannot read pitch\n");
    }

  Set playback pitch:
    u_int16 t = pitch*16384.0 + 0.5;
    if (ioctl(fp, IOCTL_AUDIO_SET_PITCH, (void *)t) == S_ERROR) {
      printf("E: Couldn't set pitch (no pitch shifter driver?)\n");
    }

  Read playback speed:
    u_int16 t;
    if ((t = ioctl(fp, IOCTL_AUDIO_GET_SPEED, NULL)) != 0xFFFFU) {
      nothingToDo = 0;
      printf("Speed %5.3f\n", t*(1.0/16384.0));
    } else {
      printf("Cannot read speed\n");
    }

  Set playback speed:
    u_int16 t = pitch*16384.0 + 0.5;
    if (ioctl(fp, IOCTL_AUDIO_SET_PITCH, (void *)t) == S_ERROR) {
      printf("E: Couldn't set pitch (no pitch shifter driver?)\n");
    }



License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
No guarantee is given for the usability of this code.


Version History:
2017-12-05 HH v1.01 - Ported to VS1005h.
2017-03-14 HH v1.00 - First release for stdaudioout.
