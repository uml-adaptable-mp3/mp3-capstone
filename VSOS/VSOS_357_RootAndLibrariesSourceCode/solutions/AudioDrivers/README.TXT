
	NEW AUDIO DRIVERS SET v1.05
	---------------------------
	 for VSOS 3.50 or higher
	 2017-12-05 VLSI Solution




To install, copy files from SYS/ directory folder in this package, to
the SYS/ directory of your VSOS3 installation.

Select to your config.txt the lines you need from the following example
template:
------------ example template starts --------------
# New 2015 audio DAC out driver
AUODAC s
# New 2015 audio ADC in driver
AUIADC s 48000 line1_1 line1_3
#AUIADC s 8000 mic1 mic2 dec6
# I2S slave input
#AUII2SS s
# If using I2S slave input as stdaudioin, synchronize it with stdaudioout
#AUXSYNCS
# I2S automatic out
#AUOI2SMA 48000
# S/PDIF automatic out, parameter can be either 48000 or 96000 (default)
#AUOSPDA 96000 s
------------ example template end------------------

Read the README.TXT files for each driver source code folder for details
on how to use them.




Version History:
2017-12-05 HH v1.05 - Ported to VS1005h. I2S drivers are not working yet
                      because hardware has changed and the driver has not
                      been updated yet.
2017-05-17 HH v1.04 - Many new S/PDIF drivers added. Almost all drivers have
                      have had updates. See README.TXT files for the specific
                      drivers for details.
2016-02-04 HH v1.03 - Now all drivers include IOCTL_AUDIO_GET_OVERFLOWS
                      and/or IOCTL_AUDIO_GET_UNDERFLOWS where applicable.
                      Added support for fractional sample rates to
                      IOCTL_AUDIO_SET_IRATE/_ORATE/_RATE_AND_BITS.
                      Added metadriver AUXPLAY.
2016-01-26 HH v1.02 - Bug fix to drivers I2S, AUODAC, and AUOSPDA.
                      Added driver AUXSYNCS.
                      Added fp->Indentify() method to all drivers.
2015-12-21 HH v1.01 - Update to I2S drivers.
2015-07-17 HH v1.00 - First release.
