
        Slave Audio Input Synchronization Driver v1.03
        ----------------------------------------------
                   for VSOS 3.50 or higher
                  2017-12-05  VLSI Solution



When inputting audio data in slave mode (using for example the I2S
audio input slave driver AUII2SS.DL3), the exact sample rate of the
audio is usually not known. Even if the nominal sample rate is known,
mismatches between master transmitter and the VS1005 receiver clock
crystals causes there to always be a mismatch between them (example:
transmitter nominally sends 48000 Hz, but because of a clock mismatch
the receiver sees the data at 48002.3 Hz). Given time, this will sooner
or later cause an underflow or overflow situation in the receiver
audio buffers, which may cause audible clicks or other kinds of audio
distortion.

The Slave Audio Input Synchronization Driver AUXSYNCS.DL3 synchronizes a
slave audio input driver with the analog Earphone/Line Out driver.

Before starting the Sync Driver, the user must first load and connect a
slave audio input driver to stdaudioin, and the analog output driver
to stdaudioout. When the driver is loaded, it will automatically adjust
the analog output sample rate according to the input. The adjustment
range is upto 97500 Hz, so standard sample rates upto 96 kHz can be
received. The Sync Driver can dynamically change its sample rate if
the input sample rate changes.

Example config.txt file clip:
# Load I2S Slave Input driver and make it stdaudioin
AUII2SS s
# Load Line Out / Earphone output driver and make it stdaudioout
AUODAC s
# Connect and synchronize stdaudiouit with stdaudioin slave
AUXSYNCS

The Sync Driver has been tested with the I2S Slave Input drivers, but
it is designed to be usable with any generic slave input driver that
offers a near-constant data rate. It may not work properly with input
drivers where there are large data bursts.



Loading and Activating the Sync Driver from the VSOS Shell:
S:>driver +auxsync



Deactivating and unloading the Sync Driver from the VSOS Shell:
S:>driver -auxsync



Loading and Activating the Sync Driver from C:
  #define LIB_NAME "auxsyncs"

  u_int16 *syncLib = NULL;

  syncLib = LoadLibrary(LIB_NAME);
  if (!syncLib) {
    printf("Cannot load " LIB_NAME ".DL3 library\n");
    goto finally;
  }



Deactivating and unloading the Sync Driver from C:
  finally:
  if (syncLib) {
    DropLibrary(syncLib);
    syncLib = NULL;
  }



IOCTL Controls:
  In addition to these controls, all the underlying controls of
  stdaudioinput and stdaudiooutput remain.

  Get sample rate. This functionality is added to the stdaudioinput.
  Example:
    s_int32 sampleRate;
    if (ioctl(stdaudioin, IOCTL_AUDIO_GET_IRATE, (char *)(&sampleRate))) {
      printf("Couldn't get sample rate\n");
    }



Example C program to Play Sound After Sync Driver Is Loaded:
  #include <vo_stdio.h>
  #include <apploader.h> // Contains LoadLibrary() and DropLibrary()
  #include <consolestate.h>

  #define BUFSIZE 128

  ioresult main(char *parameters) {
    static s_int16 myBuf[BUFSIZE];

    if (!stdaudioin || !stdaudioout) {
      printf("E: NO AUDIO IN OR OUT!\n");
      return S_ERROR;
    }

    while (!(appFlags & APP_FLAG_QUIT)) { /* Until Ctrl-C is pushed */
      fread(myBuf, sizeof(s_int16), BUFSIZE, stdaudioin);
      fwrite(myBuf, sizeof(s_int16), BUFSIZE, stdaudioout);
    }

    return S_OK;
  }



License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
Absolutely no guarantee is given for the usability of this code.


Version History:
2017-12-05 HH v1.03 - Ported to VS1005h.
2017-05-05 HH v1.02 - Now doesn't hang on IOCTL_AUDIO_GET_IRATE request if
                      input is not running.
2016-02-04 HH v1.01 - Fractional sample rate support added.
2016-01-27 HH v1.00 - Initial release.
