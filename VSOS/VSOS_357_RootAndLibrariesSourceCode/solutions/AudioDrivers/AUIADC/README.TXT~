
        AUIADC Stereo Analog Input Driver 1.05
        --------------------------------------
               for VSOS 3.50 or higher
              2017-12-05  VLSI Solution



The analog input drivers lets the user read a stereo signal from a
selected input.



Activating driver at 48 kHz, setting it to default inputs of the
VS1005 DevBoard, and making it stdaudioin in config.sys:
AUIADC s 48000 line1_1 line1_3

To do the same from the VSOS Shell:
S:>driver +auiadc s 48000 line1_1 line1_3


For microphone input at 8 kHz (dec6 activates hardware down-by-6
decimator):
AUIADC s 8000 mic1 mic2 dec6

To do the same from the VSOS Shell:
S:>driver +auiadc s 8000 mic1 mic2 dec6



Unloading the driver using the VSOS Shell:
S:>driver -auiadc




Opening the Driver:
  // Change name as necessary
  #define LIB_NAME "AUIADC"

  u_int16 *adcLib = NULL;
  FILE *adcFP = NULL;

  adcLib = LoadLibrary(LIB_NAME);
  if (!adcLib) {
    printf("Cannot load " LIB_NAME ".DL3 library\n");
    goto finally;
  }
  /* Open */
  adcFP = (FILE *)RunLoadedFunction(adcLib, ENTRY_3, 0);
  if (!adcFP) {
    printf("Cannot open " LIB_NAME ".DL3 audio file\n");
    goto finally;
  }



Closing the driver:
  finally:
  if (adcFP) {
    /* Close file */
    RunLoadedFunction(adcLib, ENTRY_4, (s_int16)adcFP);
    adcFP = NULL;
  }
  if (adcLib) {
    DropLibrary(adcLib);
    adcLib = NULL;
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



Reading Data:
  Character-based reading is NOT supported.

  The only supported read operation is:
  fread(buffer, sizeof(buffer[0]), BUFFER_ELEMENTS, fp);
  where the amount to read must be aligned with whole stereo samples,
  so 2 16-bit words for 16-bit audio, and 4 16-bit words for 32-bit audio!



IOCTL Controls:
  Restart driver. Normally this needs never be done.
  Example:
    ioctl(fp, IOCTL_RESTART, NULL);

  Set sample rate and number of bits.
  - labs(rateBits) = sampleRate
  - if rateBits < 0, then use 32-bit I/O
  - Available only with Master Mode drivers
  Example:
    s_int32 rateBits = -48000;
    if (ioctl(fp, IOCTL_AUDIO_SET_RATE_AND_BITS, (char *)(&sampleRate))) {
      printf("Couldn't set sample rate and bits\n");
    }

  Get number of bits.
  Example:
    bits = ioctl(fp, IOCTL_AUDIO_GET_BITS, NULL);

  Set number of bits.
  - bits may be 16 or 32
  - For Master Mode it is recommended to use IOCTL_AUDIO_SET_RATE_AND_BITS
  Example:
    if (ioctl(fp, IOCTL_AUDIO_SET_BITS, (char *)(32))) {
      printf("Couldn't set bits\n");
    }

  Get sample rate.
  Example:
    s_int32 sampleRate;
    if (ioctl(fp, IOCTL_AUDIO_GET_IRATE, (char *)(&sampleRate))) {
      printf("Couldn't get sample rate\n");
    }

  Set sample rate.
  - It is recommended to use IOCTL_AUDIO_SET_RATE_AND_BITS instead
  Example:
    s_int32 sampleRate = 48000;
    if (ioctl(fp, IOCTL_AUDIO_SET_IRATE, (char *)(&sampleRate))) {
      printf("Couldn't set sample rate\n");
    }

  Get input buffer fill state in 16-bit words.
  - Only for drivers with input capability
  Example:
    iBufFill = ioctl(fp, IOCTL_AUDIO_GET_INPUT_BUFFER_FILL, NULL);

  Get input buffer size in 16-bit words.
  - Only for drivers with input capability
  Example:
    iBufSize = ioctl(fp, IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE, NULL);

  Set input buffer size in 16-bit words.
  - Only for drivers with input capability
  Example:
    if (ioctl(fp, IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE, (char *)(1024))) {
      printf("Couldn't set input buffer size\n");
    }

  Get sample counter.
  Example:
    s_int32 sampleCounter;
    if (ioctl(fp, IOCTL_AUDIO_GET_SAMPLE_COUNTER, (char *)(&sampleCounter))) {
      printf("Couldn't get sample counter\n");
    }

  Select analog input. Parameter bitmask must have one stereo element,
  or one left and one right element. Definitions can be found
  in <aucommon.h>
  Stereo elements:
  - AID_FM
  Left elements:
  - AID_LINE1_1, AID_LINE3_1, AID_LINE2_1, AID_MIC1, AID_DIA1
  Right elements:
  - AID_LINE1_2, AID_LINE3_2, AID_LINE2_2, AID_MIC2, AID_DIA2, AID_DIA3,
    AID_LINE1_3
  Optionally, AID_DEC6 may also be defined. It activates the high-quality
  down-by-6 decimator.

  Example:
    s_int32 sampleCounter;
    if (ioctl(fp, IOCTL_AUDIO_SELECT_INPUT,
        (char *)(AID_LINE1_1|AID_LINE1_3))) {
      printf("Couldn't select input\n");
    }

  Get overflow sample counter for the input buffer.
  - Only for drivers with input
  Example:
    s_int32 oFlow;
    if (ioctl(adcFP, IOCTL_AUDIO_GET_OVERFLOWS, (char *)(&oFlow))) {
      printf("Couldn't get overflow counter\n");
    }



License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
No guarantee is given for the usability of this code.


Version History:
2017-12-05 HH v1.05 - Ported to VS1005h.
2016-12-12 HH v1.04 - More thorough initialization when FM input is selected.
2016-09-20 HH v1.03 - Typo corrections to this document, driver is unchanged.
2016-02-02 HH v1.02 - Added IOCTL_AUDIO_GET_OVERFLOWS function.
2016-01-27 HH v1.01 - Minor .h file correction; no functional change.
2015-07-17 HH v1.00 - First release.
