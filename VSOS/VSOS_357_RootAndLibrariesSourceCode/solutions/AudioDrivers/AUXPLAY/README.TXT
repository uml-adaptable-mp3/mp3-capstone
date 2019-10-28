
        Simple Audio Playback Driver AUXPLAY.DL3 v1.02
        ----------------------------------------------
                   for VSOS 3.50 or higher
                  2017-12-05  VLSI Solution



AUXPLAY.DL3 reads audio data from stdaudioin and copies it to
stdaudioout. Optionally, it can print out if there are input buffer
overflows and/or output buffer underflows.

When the driver is running, it is FORBIDDEN to run ioctl()'s that
alter the audio buffer or number of sample bits! It is also FORBIDDEN
to load new audio drivers that modify and/or replace stdaudioin and/or
stdaudioout.


Example config.txt file clip:
# Load AUXPLAY. If loaded with 'q' parameter, start in quiet mode
# where audio buffer underflows and overflows are NOT reported to stdout.
AUXPLAY



Loading and Activating the Playback Driver from the VSOS Shell:
S:>driver +auxplay



Deactivating and unloading the Playback Driver from the VSOS Shell:
S:>driver -auxplay



Loading and Activating the Playback Driver from C:
  #define LIB_NAME "auxplay"

  u_int16 *playLib = NULL;

  playLib = LoadLibrary(LIB_NAME);
  if (!playLib) {
    printf("Cannot load " LIB_NAME ".DL3 library\n");
    goto finally;
  }



Deactivating and unloading the driver from C:
  finally:
  if (playLib) {
    DropLibrary(playLib);
    playLib = NULL;
  }




IOCTL Controls:
  None.




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
Absolutely no guarantee is given for the usability of this code.




Version History:
2017-12-05 HH v1.02 - Ported to VS1005h.
2017-05-05 HH v1.01 - Doesn't crash if stdaudioin or stdaudioout doesn't exist.
2016-02-03 HH v1.00 - Initial release.
