
        AUODAC - VS1005 DAC Audio Driver v1.07
        --------------------------------------
               for VSOS 3.50 or higher
              2017-12-05  VLSI Solution



The AUODAC.DL3 driver offers a 16/32-bit compatible audio DAC driver for
the Earphone / Line Outputs using VS1005's main audio path.



Activating driver and making it stdaudioout in config.sys:
AUODAC s

Doing the same from the VSOS Shell:
S:>driver +auodac s



Unloading the audio driver using the VSOS Shell:
S:>driver -auodac



Opening the Driver:
  #define LIB_NAME "auodac"

  u_int16 *lib = NULL;
  FILE *fp = NULL;

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



Fractional sample rate:
  Sample rate is presented as a 32-bit integer. However, particularly
  when streaming, there sometimes is a need to present sample rates
  at greater accuracy. To do this, when setting a sample rate, bits 30:24
  may used presented to present a fractional sample rate in 1/128 Hz
  increments. This is compatible with all VSOS audio drivers: those who
  can't handle fractional sample rates will ignore these bits.

  This driver supports fractional sample rates upto approximately
  0.09 Hz accuracy.

  Examples:
  - 44100       Hz is 0x0000ac44.
  - 44100 1/128 Hz is 0x0100ac44.
  - 44100.5     Hz is 0x4000ac44.



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

  Set sample rate and number of bits. Sample rate may include fractional part.
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

  Get sample rate. Returns only the integer part.
  Example:
    s_int32 sampleRate;
    if (ioctl(fp, IOCTL_AUDIO_GET_ORATE, (char *)(&sampleRate))) {
      printf("Couldn't get sample rate\n");
    }

  Set sample rate. May include fractional part.
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
  - Only for drivers with output
  Example:
    s_int32 uFlow;
    if (ioctl(fp, IOCTL_AUDIO_GET_UNDERFLOWS, (char *)(&uFlow))) {
      printf("Couldn't get underflow counter\n");
    }

  Get volume.
  Volume is a number between 0 - 511 where 256 is full-scale, and each
  successive number represents a volume gain step of -0.5 dB.
  See table below:
  -   0 = +128.0 dB: Insane amplification (driver saturates to 0.0 dB)
  - 255 =   +0.5 dB:                      (driver saturates to 0.0 dB)
  - 256 =    0.0 dB: Full scale volume    (highest for this driver)
  - 257 =   -0.5 dB: 
  - 510 = total silence (doesn't shut analog drivers off)
  - 511 = total silence (shuts analog drivers off)
  Example:
    volume = ioctl(fp, IOCTL_AUDIO_GET_VOLUME, NULL);

  Set volume. Scale for volume is the same as for IOCTL_AUDIO_GET_VOLUME.
  Example:
    /* Set full scale volume */
    if (ioctl(fp, IOCTL_AUDIO_SET_VOLUME, (char *)(256))) {
      printf("Couldn't set volume\n");
    }



License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
No guarantee is given for the usability of this code.


Version History:
2017-12-05 HH v1.07 - Ported to VS1005h.
2017-03-13 HH v1.06 - Fixed an issue that could cause a noise burst when bit
                      depth was changed between 16 and 32.
2016-09-20 HH v1.05 - Typo corrections to this document, driver is unchanged
2016-02-04 HH v1.04 - Added IOCTL_AUDIO_GET_UNDERFLOWS function.
                      Support for fractional sample rates added.
2016-01-26 HH v1.03 - Bug fix.
2015-07-17 HH v1.02 - Enhanced output buffer allocation.
2015-07-14 HH v1.01 - Added volume control to documentation.
2015-06-16 HH v1.00 - First documented release.
