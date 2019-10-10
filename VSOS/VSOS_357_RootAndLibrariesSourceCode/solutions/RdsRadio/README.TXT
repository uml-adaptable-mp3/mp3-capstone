
	VS1005 VSOS RDS FM RADIO v1.05
	------------------------------
	   2018-06-12 Test Release

This is a test release of the VS1005 VSOS RDS FM Radio. To use it, copy
the file RdsRadio.dl3 to you SYS/ folder.



STARTING THE PROGRAM
--------------------

Start the program by writing the following command on the command line:
S:>RdsRadio

or by using the following line in CONFIG.TXT:
RUN RDSRADIO

If you want to start the radio at a certain channel, write the frequency
in kHz as a parameter. E.g. to start at 98.1 MHz:
S:>RdsRadio 98100



THE USER INTERFACE
------------------

?	Display signal strength in relative dB. RDS typically works
	if the signal strength is above 60 dB. However, as RDS
	signal strengths vary a lot, this is not guaranteed.

.	Fine tune channel frequency by +10 kHz.
,	Fine tune channel frequency by -10 kHz.

n / S2	Find next channel
p / S1	Find previous channel

d	Toggle debug mode. In debug mode more details are shown when
	doing channel tuning. Also, each second an RDS quality percentage
	is shown. Any percentage below 10% means that no real RDS signal
	has been detected. For a very good channel the percentage can be
	upto 98-100%.

t	Fine tune radio channel.

T	Show time stamp.

< / S3	Volume down by 1 dB.
> / S4	Volume up by 1 dB.

D	Activate VLSI internal test dump mode.

s  Set stereo mode.
m  Set mono mode.

0  Disable noise killer (if available)
5  Set noise killer to low (if available)
4  Set noise killer to mid (if available)
3  Set noise killer to high (if available)

Ctrl-C	Exit RDS FM Radio application.



MODIFYING THE DEFAULT STARTING FREQUENCY
----------------------------------------

To modify the default start frequency, change the following line in main.c:
  u_int32 initFreq = 93700;

To perform a test frequency scan:
S:>RdsRadio s



MODIFYING THE FREQUENCY LIMTIS
------------------------------

To modify the FM radio band frequency limits, change defines for FM_LOW
and FM_HIGH in fmModel.h.



MODIFYING THE USER INTERFACES
-----------------------------

At the beginning of main.c, you leave undefined preprocessor symbols
USE_DEVBOARD_BUTTONS and/or USE_STDIN to remove the iser interfaces
by VS1005 Developer Board Buttons S1 through S4 and/or UART keys,
respectively. As a default, both interfaces are active.



CHANNEL SEARCH TUNING
---------------------

You can modify channel tuning with the following two constants in
fm_function.c.

#define FM_NEXT_SIGNAL_STRENGTH_ABSOLUTE_LIMIT 400 /* In 1/10th of a decibel */

This constant is dependent on the absolute signal level you are getting
from your antenna components. Run a frequency scan, and decide an
appropriate level.


#define FM_NEXT_THRESHOLD 30 /* How high a signal level peak there must be, in 1/10th of a decibel */

Defines how much stronger a channel has to be from the background noise to
be recognized. The higher the number, the less spurious channels are shown,
but make it too high, and some real channels may not be recognized properly,
especially if the FM radio space is heavily congested.



Version History:
2018-06-12 HH v1.05 - Added mono/stereo control and noise killer control.
2017-12-05 HH v1.04 - Ported to VS1005h.
2017-01-20 HH v1.03 - RDS can be turned off. In this case IQ strength
                      calculation is now faster than before right after a
                      channel change.
2016-12-15 HH v1.02 - Cleaned the code slightly, added VS1005 DevBoard S1-S4
                      buttons.
2016-11-21 HH v1.01 - Fixed interrupt release bug that revealed by VSOS 3.40.
2016-05-30 HH v1.00 - First test release.
