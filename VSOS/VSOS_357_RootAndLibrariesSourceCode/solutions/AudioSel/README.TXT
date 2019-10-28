        AUDIOSEL - Audio selector and loopback program example
        ------------------------------------------------------
                       2018-11-06 VLSI Solution

AudioSel is an audio selector and loopback program example.

To run, first copy AudioSel.DL3 to your SYS/ folder. If you want to use
the I2S input, copy the I2S Master Mode Input Driver AUII2SM.DL3 to your
VS1005 device's SYS/ folder (current version included in this package).

Type "audiosel" on the command line shell, or add "RUN AudioSel"
to CONFIG.TXT.

When running, the following key commands may be used:
'0' - '9': Select a stereo or mono input
'<': Volume down
'>': Volume up
'q': Quit

See the source code in main.c to see the details on how AudioSel works.



EXAMPLE CONFIG.TXT FILE
-----------------------

# Set clock to a higher value so that even the more difficult codecs can play.
RUN SETCLOCK -l100 80
# Start SD card as device D
SDSD D
# Start UART in/out driver
UARTIN
# Audio DAC out driver
AUODAC s
# Set output buffer size and default volume level
RUN AUOUTPUT -s512 -l-12
# New 2015 audio ADC in driver
AUIADC s 48000 line1_1 line1_3
# Set input buffer size
RUN AUINPUT -s512
# S/PDIF automatic out, parameter can be either 48000 or 96000 (default)
AUOSPDA 96000
# Uncomment following line to get AudioSel to always automatically start
#RUN AudioSel
# Start the Shell
S:SHELL.AP3
