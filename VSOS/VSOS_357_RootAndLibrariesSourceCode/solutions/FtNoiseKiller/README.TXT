
          FM Stereo Radio Noise Killer v1.00
          ----------------------------------
               for VSOS 3.55 or higher
              2018-06-12  VLSI Solution



Because of the way stereo information is transmitted on FM radio,
stereo reception is always more suscept to white noise and other
artifacts than mono reception. A way to reduce or remove the noise is
to either dampen the stereo effect at the receiver, or to just turn
FM stereo reception off. The FtNoiseKiller package offers an adaptive
FM stereo radio noise killer algorithm that doesn't destroy the stereo
image.



Name      Description
FTINOISE  Noise killer that connects to stdaudioin
SETNOISE  User program to set parameters for the noise killer

Features and limitations:
- Optimized for 32 kHz operation.
- Requires about 25 MIPS at 32 kHz 16 bits.
- Can only handle 16-bit audio (if audio is set to 32 bits, the
  noise killer is disabled).

For additional information, see the VS1005 VSOS Audio Subsystem document,
Chapter Noise Killer Audio Drivers.



Starting FTINOISE.DL3 in config.txt and setting room size:
# Starts noise killer and connects to stdaudioin
FTINOISE
# Set room size to default 50 dB threshold.
# When reception is poor, a 40 dB threshold may be more effective.
# The noise killer is disabled by setting it to 0.
RUN SETNOISE -n50

Note: FTINOISE is loaded and stays in memory. Because SETNOISE doesn't
need to stay in memory, it is started with the RUN command.


Starting FTINOISE.DL3 from the VSOS Shell command line and setting
similar parameters:
S:>driver +ftinoise
S:>setnoise -n50 -v
S:>rdsradio


Unloading the FTINOISE.DL3 driver using the VSOS Shell:
S:>driver -ftinoise


Starting FTINOISE.DL3 using C code:
  // Change name as necessary
  #define LIB_NAME "ftinoise"

  u_int16 *noiseLib = NULL;
  FILE *noiseFP = NULL;

  noiseLib = LoadLibrary(LIB_NAME);
  if (!noiseLib) {
    printf("Cannot load " LIB_NAME ".DL3 library\n");
    goto finally;
  }
  noiseFP = stdaudioout;

Note that if you have started FTINOISE from config.txt, you don't need
to open it from your software, but instead you can directly send ioctl()
calls to stdaudioin.



Closing the driver:
  finally:
  if (noiseFP) {
    /* Close file */
    RunLoadedFunction(i2sLib, ENTRY_4, (void *)noiseFP);
    noiseFP = NULL;
  }
  if (noiseLib) {
    DropLibrary(noiseLib);
    noiseLib = NULL;
  }




Running SETNOISE from the VSOS Shell:
S:>setnoise -n50

To see the current noise killer parameters:
S:>setnoise
Noise killer at 50 dB

To see the help page:
S:>setnoise -h
Usage: SetNoise [-i|-o] [-v|+v] [-nx] [-h]
-i      Set stdaudioin (default)
-o      Set stdaudioout
-nx     Set noise killer level (default: 50 dB, 0 = off)
-v|+v   Verbose on/off
-h      Show this help


Data structures:

If using an earlier version than VSOS 3.60, the following data structures
are not yet in aucommon.h, so you need to add this to your source code:

#ifndef IOCTL_AUDIO_GET_NOISE_KILLER
#define IOCTL_AUDIO_GET_NOISE_KILLER	290
#define IOCTL_AUDIO_SET_NOISE_KILLER	291
#endif /* !IOCTL_AUDIO_GET_NOISE_KILLER */




IOCTL Controls:
  All ioctl controls except those listed here are forwarded to the underlying
  audio driver.

  Read noise killer level parameters:
    s_int16 dB;
    struct Room *room;
    if ((dB = ioctl(fp, IOCTL_AUDIO_GET_NOISE_KILLER, NULL)) >= 0) {
      printf("Noise killer set to %d dB\n", dB);
    } else {
      printf("E: Cannot read noise killer setting\n");
    }

  Set noise killer level to 50 dB:
    struct Room *room;
    if (ioctl(fp, IOCTL_AUDIO_SET_NOISE_KILLER, (void *)(50)) < 0) {
      printf("E: Cannot set noise killer\n");
    }




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
No guarantee is given for the usability of this code.


Version History:
2018-06-12 HH v1.00 - First release for stdaudioin.
