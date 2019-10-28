
        AUXSPDIF - VS1005 S/PDIF Drivers v1.11
        --------------------------------------
                for VSOS 3.50 or higher
               2017-12-05  VLSI Solution



The AUXPDIF drivers offer S/PDIF functionality to VS1005.


There following drivers are available:

Name          Features   Notes and Description

AUOSPDA.DL3        AOut  See note  1
  Driver automatically converts the output of the analog audio output driver
  AUODAC.DL3 to exactly 48 or 96 kHz, then sends that out to S/PDIF.

AUISPD.DL3    In         See notes 2, 3, 5
  S/PDIF input driver, capable of receiving approximately 44.1, 48, 88.2,
  and 96 kHz.

AUXSPD.DL3    In +  Out  See notes 2, 3, 5, 8
  S/PDIF input/output driver, capable of approximately 44.1, 48, 88.2, and
  96 kHz sample rates. Output is automatically synchronized with input.

AUXSPD48.DL3  In +  Out  See notes 2, 4, 6, 8
  S/PDIF input/output driver, capable of approximately 48, and 96 kHz sample
  rates. Output is automatically synchronized with input.
  Uses slightly less memory than AUXSPD.DL3.

AUOSP48S.DL3        Out  See notes 2, 4, 7, 9
  S/PDIF output driver that synchronizes S/PDIF output sample rate with the
  input sample rate, which must be approximately 48 kHz.
  Example use case: In conjunction with I2S input slave driver AUII2SS.DL3
  when it is known that the I2S driver's nominal input sample rate is always
  approximately 48 kHz.

AUOSPD48.DL3        Out  See notes 7
  S/PDIF output driver that supports exactly 48 kHz.

Note 1: This driver supports 48 and 96 kHz output sample rates. An
  automatic hardware sample rate conversion is done from the current
  playback sample rate. For best audio quality, the output sample rate
  should at least as high as the highest playback sample rate.

Note 2: This driver is incompatible with any USB drivers and
  FM Radio receiver software.

Note 3: This driver automatically sets the VSDSP core clock to approximately
  61.440 or 56.446 MHz.

Note 4: This driver automatically sets the VSDSP core clock to approximately
  61.440 MHz.

Note 5: This driver supports 44.1, 48, 88.2, and 96 kHz sample rates.

Note 6: This driver supports 48 and 96 kHz sample rates.

Note 7: This driver supports 48 kHz sample rate.

Note 8: The input and output always operate at the same sample rate.
This driver automatically synchronizes its input and output.
The synchronization driver AUXSYNCS.DL3 is not needed.

Note 9: Output is automatically synchronized with stdaudioinput, which
must be approximately 48 kHz. The synchronization driver AUXSYNCS.DL3 is not
needed.


Features Legend:
In     - Digital DSP Input
Out    - Digital DSP Output
AOut   - Automatic DAC SRC Duplication Output

The Automatic DAC SRC Duplication Output feature duplicates the data
that is sent through the DAC_SRC sample rate upconverter for analog
sound output. For details, see the audio path going through the SRC_CF
component block in Figure "VS1005g playback (DA) audio paths" in the
VS1005g datasheet. Note that for acceptable audio quality, the S/PDIF
sample rate must be at least as high as the highest sample rate that is
played back.


Example config.txt files:

-------- example config.txt #1 ----------
# Start DAC driver and make it stdaudioout
AUODAC s
# Start S/PDIF auto driver:
# - auto DAC copy mode
# - default samplerate (96000)
# - no auto volume control
AUOSPDA
-------- example config.txt #2 ----------
# Start DAC driver and make it stdaudioout
AUODAC s
# Start S/PDIF auto driver:
# - auto DAC copy mode
# - 48000 Hz (alternative: 96000)
# - auto volume control (copies volume control of stdaudioout)
AUOSPDA 48000 v
-------- example config.txt #3 ----------
# Start DAC driver and make it stdaudioout
AUODAC s
# Start S/PDIF input driver and make it staudioin
AUISPD s
# Start Input/output synchronization driver:
# it will keep DAC in sync with S/PDIF input
AUXSYNCS
# Start stdaudioin to stdaudioout audio copy driver
AUXPLAY
-------- example config.txt #4 ----------
# Start S/PDIF In/Out driver and make it staudioin/stdaudioout
AUXSPD s
# Start stdaudioin to stdaudioout audio copy driver, so now S/PDIF input
# is copied to S/PDIF output
AUXPLAY
-------- examples end -------------------

Example #2 from the VSOS Shell command line:
S:>driver +auodac s
S:>driver auospda 48000 v

Example #3 from the VSOS Shell command line:
S:>driver +auodac s
S:>driver +auispd s
S:>driver +auxsyncs
S:>driver +auxplay




For applications that want to specifically access the S/PDIF driver,
the following functions are available.

Opening the Driver:
  // Change name as necessary
  #define LIB_NAME "auospda"

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

  This driver ignores fractional sample rate information.

  Examples:
  - 44100       Hz is 0x0000ac44.
  - 44100 1/128 Hz is 0x0100ac44.
  - 44100.5     Hz is 0x4000ac44.




IOCTL Controls:
  Restart driver. Normally this needs never be done.
  Example:
    ioctl(fp, IOCTL_RESTART, NULL);

  Set sample rate and number of bits (bits are ignored).
  - labs(rateBits) = sampleRate
  Example:
    /* Set 48 kHz with 24 bits */
    s_int32 rateBits = -48000; /* Or, rateBits = 48000; */
    if (ioctl(fp, IOCTL_AUDIO_SET_RATE_AND_BITS, (char *)(&rateBits))) {
      printf("Couldn't set sample rate and bits\n");
    }

  Get sample rate.
  Example:
    s_int32 sampleRate;
    if (ioctl(fp, IOCTL_AUDIO_GET_ORATE, (char *)(&sampleRate))) {
      printf("Couldn't get sample rate\n");
    }

  Set sample rate.
  Example:
    s_int32 sampleRate = 96000;
    if (ioctl(fp, IOCTL_AUDIO_SET_ORATE, (char *)(&sampleRate))) {
      printf("Couldn't set sample rate\n");
    }

  Get volume.
  Volume is a number between 0 - 511 where 256 is full-scale, and each
  successive number represents a volume gain step of -0.5 dB.
  See table below:
  -   0 = +128.0 dB: Insane amplification (driver saturates to +24.0 dB)
  - 208 =  +24.0 dB: Highest amplification supported by this driver
  - 255 =   +0.5 dB: Slight gain
  - 256 =    0.0 dB: Full scale, neutral volume
  - 257 =   -0.5 dB: 
  - 510 = -127.0 dB: Total silence (with this driver, since about -74 dB)
  - 511 = -127.5 dB: Total silence, may shut down output (not implemented)
  Example:
    volume = ioctl(fp, IOCTL_AUDIO_GET_VOLUME, NULL);

  Set volume.
    Scale for volume is the same as for IOCTL_AUDIO_GET_VOLUME.
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
2017-12-05 HH v1.11 - Ported to VS1005h.
2017-05-16 HH v1.10 - Added AUSP48S.DL3 and AUOSPD48.DL3.
2017-05-11 HH v1.08 - Added AUXSPD48.DL3.
2017-05-05 HH v1.07 - Added AUXSPD.DL3.
2017-05-05 HH v1.06 - Added AUISPD.DL3.
2016-09-20 HH v1.05 - Typo corrections to this document, driver is unchanged.
2016-02-02 HH v1.04 - Reformatted this document.
2016-01-27 HH v1.03 - Corrected bug that caused occasional crash at startup.
2015-07-17 HH v1.02 - Option 'S' moved to option 'V'.
2015-07-14 HH v1.01 - Added manual + automatic volume control for AUOSPDA.DL3.
2015-07-07 HH v1.00 - First release.
