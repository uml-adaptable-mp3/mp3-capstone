
        AuControl - stdaudioin/stdaudioout control programs v1.04
        ---------------------------------------------------------
                        for VSOS 3.50 or higher
                       2017-12-05  VLSI Solution




AuInput and AuOutput let the user select the audio source, sample rate,
and number of bits for stdaudioin and stdaudioout, respectively.




The VSOS Shell command line options are as follows. The order of the
command line options don't matter. Without parameters the programs print
the current audio parameters.

Usage: AuInput [rate|bits|bufsize|chconf|-h]
rate    sample rate (Hz, must be >= 8000)
bits    number of bits (16 or 32)
bufsize buffer size in 16-bit words (100..7999)
chconf  Audio channel config (only with AUIADC driver, see definitions below)
-h      Show this help

chconf needs either one stereo element, or one left and one right element.
Stereo elements:
- fm (doesn't start FM, but prepares data paths and interrupts.)
Left elements:
- line1_1, line2_1, line3_1, mic1, dia1
Right elements:
- line1_2, line2_2, line3_2, line1_3, mic2, dia2, dia3


Usage: AuOutput [rate|bits|bufsize|-h]
rate    sample rate (Hz, must be >= 8000)
bits    number of bits (16 or 32)
bufsize buffer size in 16-bit words (100..7999)
-h      Show this help



Examples:

In VSOS Shell, to set input sample rate to 48 kHz and number of bits
to 16:
S:>auinput 48000 16

In VSOS Shell, to set output sample rate to 16 kHz:
S:>auoutput 16000

In VSOS Shell, to set inputs to default DevBoard line inputs.
This control only works with the AUIADC.DL3 audio driver:
S:>auinput line1_1 line1_3

To set microphone input at 8 kHz (dec6 activates hardware down-by-6
decimator). This control only works with the AUIADC.DL3 audio driver:
S:>auinput 8000 mic1 mic2 dec6



Running from config.txt:

Add the command word "run" before control programs; otherwise the
programs will be loaded as drivers and remain in memory. E.g.:
RUN AUINPUT 16000 16




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2017-12-05 HH v1.04 - Ported to VS1005h.
2016-09-22 HH v1.03 - Slight TRACE optimization.
2016-01-26 HH v1.02 - Added buffer size controls.
2016-01-20 HH v1.01 - Added AuOutput.dl3, renamed solution AuControl.
2015-08-20 HH v1.00 - First release (solution was named AuInput).
