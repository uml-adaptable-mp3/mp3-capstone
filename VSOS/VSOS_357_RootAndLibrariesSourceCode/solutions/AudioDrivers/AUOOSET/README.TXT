
        AUOOSET - VS1005 DAC Audio Driver v1.05
        ---------------------------------------
               for VSOS 3.50 or higher
              2017-12-05  VLSI Solution



The AUOOSET.DL3 driver offers a 16/32-bit compatible audio driver that
uses the VS1005 secondary "audio offset" path. This audio path offers a
lower quality audio path than the main audio path (used by the AUODAC.DL3
audio driver), and is suitable for e.g. sound effects.



Activating driver and making it stdaudioout in config.sys:
AUOOSET s

Doing the same from the VSOS Shell:
S:>driver +auooset s



Opening the Driver:
  #define LIB_NAME "auooset"

  u_int16 *lib = NULL;
  FILE *fP = NULL;

  lib = LoadLibrary(LIB_NAME);
  if (!lib) {
    printf("Cannot load " LIB_NAME ".DL3 library\n");
    goto finally;
  }
  /* Open */
  fp = (FILE *)RunLoadedFunction(lib, ENTRY_3, 0);
  if (!fp) {
    printf("Cannot open " LIB_NAME ".DL3 audio file\n");
    goto finally;
  }



Fractional sample rate:
  Sample rate is presented as a 32-bit integer. However, particularly
  when streaming, there sometimes is a need to present sample rates
  at greater accuracy. To do this, when setting a sample rate, bits 30:24
  may used presented to present a fractional sample rate in 1/128 Hz
  increments. This is compatible with all VSOS audio drivers: those who
  can't handle fractional sample rates will ignore these bits.

  This driver ignores fractional sample rate information.

  Examples:
  - 44100       Hz is 0x0000ac44.
  - 44100 1/128 Hz is 0x0100ac44.
  - 44100.5     Hz is 0x4000ac44.



Closing the driver:
  finally:
  if (fp) {
    /* Close file */
    RunLoadedFunction(lib, ENTRY_4, (s_int16)fp);
    fp = NULL;
  }
  if (lib) {
    DropLibrary(lib);
    lib = NULL;
  }



Writing Data:
  Character-based reading is NOT supported.

  The only supported write operation is:
  fwrite(buffer, sizeof(buffer[0]), BUFFER_ELEMENTS, fp);
  where the amount to write must be aligned with whole stereo samples,
  so 2 16-bit words for 16-bit audio, and 4 16-bit words for 32-bit audio!



IOCTL Controls:
  Restart driver. Normally this needs never be done.
  Example:
    ioctl(fp, IOCTL_RESTART, NULL);

  Set sample rate and number of bits.
  - labs(rateBits) = sampleRate
  - if rateBits < 0, then use 32-bit I/O
  Example:
    /* Set 32-bit mode at 48 kHz */
    s_int32 rateBits = -48000;
    if (ioctl(fp, IOCTL_AUDIO_SET_RATE_AND_BITS, (char *)(&rateBits))) {
      printf("Couldn't set sample rate and bits\n");
    }

  Get number of bits.
  Example:
    bits = ioctl(fp, IOCTL_AUDIO_GET_BITS, NULL);

  Set number of bits.
  - bits may be 16 or 32
  Example:
    if (ioctl(fp, IOCTL_AUDIO_SET_BITS, (char *)(32))) {
      printf("Couldn't set bits\n");
    }

  Get sample rate.
  Example:
    s_int32 sampleRate;
    if (ioctl(fp, IOCTL_AUDIO_GET_ORATE, (char *)(&sampleRate))) {
      printf("Couldn't get sample rate\n");
    }

  Set sample rate.
  Example:
    s_int32 sampleRate = 48000;
    if (ioctl(fp, IOCTL_AUDIO_SET_ORATE, (char *)(&sampleRate))) {
      printf("Couldn't set sample rate\n");
    }

  Get how many 16-bit words there are free in the output buffer.
  Example:
    iBufFill = ioctl(fp, IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE, NULL);

  Get output buffer size in 16-bit words.
  Example:
    oBufSize = ioctl(fp, IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE, NULL);

  Set output buffer size in 16-bit words.
  Example:
    if (ioctl(fp, IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE, (char *)(1024))) {
      printf("Couldn't set output buffer size\n");
    }

  Get sample counter.
  Example:
    s_int32 sampleCounter;
    if (ioctl(fp, IOCTL_AUDIO_GET_SAMPLE_COUNTER, (char *)(&sampleCounter))) {
      printf("Couldn't get sample counter\n");
    }

  Get underflow sample counter for the output buffer.
  Example:
    s_int32 uFlow;
    if (ioctl(fp, IOCTL_AUDIO_GET_UNDERFLOWS, (char *)(&uFlow))) {
      printf("Couldn't get underflow counter\n");
    }



License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
Absolutely no guarantee is given for the usability of this code.


Version History:
2017-12-05 HH v1.05 - Ported to VS1005h.
2017-03-13 HH v1.04 - Fixed an issue that could cause a noise burst when bit
                      depth was changed between 16 and 32.
2016-09-20 HH v1.03 - Typo corrections to this document, driver is unchanged.
2016-02-02 HH v1.02 - Added IOCTL_AUDIO_GET_UNDERFLOWS function.
2016-01-27 HH v1.01 - Added Identify() function.
2015-06-16 HH v1.00 - First documented release.
