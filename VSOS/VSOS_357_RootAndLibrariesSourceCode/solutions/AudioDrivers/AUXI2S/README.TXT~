
         I2S Audio Drivers v1.08
         -----------------------
         for VSOS 3.50 or higher
        2017-12-31  VLSI Solution



I2S audio drivers offer flexible input/output functionality using the I2S
audio bus.

There are seven drivers:
Name         V1.02   Features  
             Size/w
AUXI2SM.DL3   789    Master, In +  Out, Sys
AUII2SM.DL3   572    Master, In + AOut, Sys
AUOI2SM.DL3   576    Master,       Out, Sys
AUOI2SMA.DL3  290    Master,      AOut,      Cfg
AUXI2SS.DL3   709    Slave,  In +  Out, Sys
AUII2SS.DL3   507    Slave,  In       , Sys
AUOI2SS.DL3   511    Slave,        Out, Sys

Features Legend:
In     - Digital DSP Input
Out    - Digital DSP Output
AOut   - Automatic DAC SRC Duplication Output
Master - Master Mode
Slave  - Slave Mode
Sys    - May take over stdaudioin and/or stdaudioout
Cfg    - Sample rate and number of bits may be configured when loading

The Automatic DAC SRC Duplication feature duplicates the data that
has been sent through the DAC_SRC sample rate upconverter for analog
sound output. For details, see the audio path going through the SRC_CF
component block in Figure "VS1005g playback (DA) audio paths" in the
VS1005g datasheet. Note that for acceptable audio quality, the I2S
sample rate must be at least as high as the highest sample rate that is
played back. The Automatic DAC SRC Duplication feature doesn't use the
processor, so the AUOI2SMA.DL3 driver does zero processing when running.

Master mode drivers  are limited to one of the following sample rate
and bit rate configurations:
   Sample Rate   Bits   Value in configuration file for Cfg drivers:
-  24 kHz,          32        /-24000
-  48 kHz,       16/32   48000/-48000
-  96 kHz,       16/32   96000/-96000
- 192 kHz,       16     192000

Sample rate and number of bits can also be set with utilities
AUINPUT.DL3 and AUOUTPUT.DL3.

The sample rate when using the slave drivers is whatever the transmitter
is sending. The driver offers no direct access to the sample rate. To
determine sample rate the user can use the sample counter (see
IOCTL Controls) and e.g. the function ReadTimeCount() (see <audio.h>).
To synchronize an I2S slave input and VS1005g earphone/line output, use
synchronization library AUXSYNC.DL3.

When in slave mode, VS1005g can be set to 16-bit mode even if the master
is sending 32 bits and vice versa. 32-bit resolution is achieved only if
both the master and VS1005g are in 32-bit mode.



Activating example driver with Cfg capability and make it use
32-bit 96000 Hz output:
# Load I2S automatic master driver (audio files can be at any sample rate)
AUOI2SMA -96000

Doing the same from the VSOS Shell:
S:>driver +auxi2sma -96000



Activating example driver with Sys capability, make it standard audio
in config.sys, then set it to 96000 Hz 16-bit mode:
# Load I2S master driver
AUXI2SM s
# Set driver to 96000 Hz 16-bits mode
run auinput 96000 16

Doing the same from the VSOS Shell:
S:>driver +auxi2sm s
S:>auinput 96000 16



Unloading the beforementioned driver using the VSOS Shell:
S:>driver -auxi2sm



Opening a Driver:
  // Change name as necessary
  #define LIB_NAME "auxi2sm"

  u_int16 *i2sLib = NULL;
  FILE *i2sFP = NULL;

  i2sLib = LoadLibrary(LIB_NAME);
  if (!i2sLib) {
    printf("Cannot load " LIB_NAME ".DL3 library\n");
    goto finally;
  }
  /* Open */
  i2sFP = (FILE *)RunLoadedFunction(i2sLib, ENTRY_3, 0);
  if (!i2sFP) {
    printf("Cannot open " LIB_NAME ".DL3 audio file\n");
    goto finally;
  }



Closing a driver:
  finally:
  if (i2sFP) {
    /* Close file */
    RunLoadedFunction(i2sLib, ENTRY_4, (s_int16)i2sFP);
    i2sFP = NULL;
  }
  if (i2sLib) {
    DropLibrary(i2sLib);
    i2sLib = NULL;
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
  fread(buffer, sizeof(buffer[0]), BUFFER_ELEMENTS, i2sFP);
  where the amount to read must be aligned with whole stereo samples,
  so 2 16-bit words for 16-bit audio, and 4 16-bit words for 32-bit audio!

  For best performance, it is recommended to read at least 16 words at a time.



Writing Data:
  Character-based reading is NOT supported.

  The only supported write operation is:
  fwrite(buffer, sizeof(buffer[0]), BUFFER_ELEMENTS, i2sFP);
  where the amount to write must be aligned with whole stereo samples,
  so 2 16-bit words for 16-bit audio, and 4 16-bit words for 32-bit audio!

  For best performance, it is recommended to write at least 16 words at a time.



IOCTL Controls:
  Restart driver. Normally this needs never be done.
  Example:
    ioctl(i2sFP, IOCTL_RESTART, NULL);

  Set sample rate and number of bits.
  - labs(rateBits) = sampleRate
  - if rateBits < 0, then use 32-bit I/O
  - Available only with Master Mode drivers
  Example:
    s_int32 rateBits = -48000;
    if (ioctl(i2sFP, IOCTL_AUDIO_SET_RATE_AND_BITS, (char *)(&sampleRate))) {
      printf("Couldn't set sample rate and bits\n");
    }

  Get number of bits.
  Example:
    bits = ioctl(i2sFP, IOCTL_AUDIO_GET_BITS, NULL);

  Set number of bits.
  - bits may be 16 or 32
  - For Master Mode it is recommended to use IOCTL_AUDIO_SET_RATE_AND_BITS
  Example:
    if (ioctl(i2sFP, IOCTL_AUDIO_SET_BITS, (char *)(32))) {
      printf("Couldn't set bits\n");
    }

  Get sample rate.
  - Only for Master Mode drivers
  Example for driver with input:
    s_int32 sampleRate;
    if (ioctl(i2sFP, IOCTL_AUDIO_GET_IRATE, (char *)(&sampleRate))) {
      printf("Couldn't get sample rate\n");
    }
  Example for driver with output:
    s_int32 sampleRate;
    if (ioctl(i2sFP, IOCTL_AUDIO_GET_ORATE, (char *)(&sampleRate))) {
      printf("Couldn't get sample rate\n");
    }

  Set sample rate.
  - Only for Master Mode drivers
  - It is recommended to use IOCTL_AUDIO_SET_RATE_AND_BITS instead
  Example for driver with input:
    s_int32 sampleRate = 48000;
    if (ioctl(i2sFP, IOCTL_AUDIO_SET_IRATE, (char *)(&sampleRate))) {
      printf("Couldn't set sample rate\n");
    }
  Example for driver with output:
    s_int32 sampleRate = 48000;
    if (ioctl(i2sFP, IOCTL_AUDIO_SET_ORATE, (char *)(&sampleRate))) {
      printf("Couldn't set sample rate\n");
    }

  Get input buffer fill state in 16-bit words.
  - Only for drivers with input capability
  Example:
    iBufFill = ioctl(i2sFP, IOCTL_AUDIO_GET_INPUT_BUFFER_FILL, NULL);

  Get input buffer size in 16-bit words.
  - Only for drivers with input capability
  Example:
    iBufSize = ioctl(i2sFP, IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE, NULL);

  Set input buffer size in 16-bit words.
  - Only for drivers with input capability
  Example:
    if (ioctl(i2sFP, IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE, (char *)(1024))) {
      printf("Couldn't set input buffer size\n");
    }

  Get how many 16-bit words there are free in the output buffer.
  - Only for drivers with DSP output capability
  Example:
    iBufFill = ioctl(i2sFP, IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE, NULL);

  Get output buffer size in 16-bit words.
  - Only for drivers with DSP output capability
  Example:
    oBufSize = ioctl(i2sFP, IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE, NULL);

  Set output buffer size in 16-bit words.
  - Only for drivers with DSP output capability
  Example:
    if (ioctl(i2sFP, IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE, (char *)(1024))) {
      printf("Couldn't set output buffer size\n");
    }

  Get sample counter.
  - Not available in AUOI2SMA.DL3
  Example:
    s_int32 sampleCounter;
    if (ioctl(i2sFP, IOCTL_AUDIO_GET_SAMPLE_COUNTER, (char *)(&sampleCounter))){
      printf("Couldn't get sample counter\n");
    }

  Get overflow sample counter for the input buffer.
  - Only for drivers with input
  Example:
    s_int32 oFlow;
    if (ioctl(i2sFP, IOCTL_AUDIO_GET_OVERFLOWS, (char *)(&oFlow))) {
      printf("Couldn't get overflow counter\n");
    }

  Get underflow sample counter for the output buffer.
  - Only for drivers with output
  Example:
    s_int32 uFlow;
    if (ioctl(i2sFP, IOCTL_AUDIO_GET_UNDERFLOWS, (char *)(&uFlow))) {
      printf("Couldn't get underflow counter\n");
    }



License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
Absolutely no guarantee is given for the usability of this code.


Version History:
2017-12-31 HH v1.08 - Partially orted to VS1005h. Not working yet because
                      hardware changes have not been updated.
2017-03-13 HH v1.07 - Fixed an issue that could cause a noise burst when bit
                      depth was changed between 16 and 32.
2016-09-20 HH v1.06 - Typo corrections to this document, driver is unchanged.
2016-02-02 HH v1.05 - Added IOCTL_AUDIO_GET_OVERFLOWS / _UNDERFLOWS functions.
2016-01-25 HH v1.04 - All drivers start as default at 16 bits, not 32 bits.
2015-12-21 HH v1.03 - Corrected IOCTL_AUDIO_GET_IRATE and IOCTL_AUDIO_GET_ORATE
                      for all master drivers.
2015-12-21 HH v1.02 - Added Cfg capability to AUOI2SMA.DL3.
2015-03-27 HH v1.01 - Bug fixes, now expects <aucommon.h>.
2015-03-26 HH v1.00 - Added Master and Slave Mode drivers.
2015-03-20 HH v0.01 - Master mode I/O driver test release.
