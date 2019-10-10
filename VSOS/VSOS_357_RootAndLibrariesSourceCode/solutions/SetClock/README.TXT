
              SetClock 1.07
              -------------
          for VSOS 3.50 or higher
         2018-04-11  VLSI Solution




SetClock is a VSOS Shell program that lets the user set VS1005's internal
clock frequency to something else than the default 60 MHz.

Note that USB will only work if the clock is set to exactly 60 MHz with
command line parameter "usb".




Running SETCLOCK.DL3 in config.txt, setting the clock to 80 MHz:
RUN SETCLOCK 80




Running SetClock from the VSOS Shell:
Set clock to approximately 80 MHz:
S:>setclock 80

Set clock to exactly 60 MHz (for USB compatibility):
S:>setclock usb

Set core clock to 56.789 MHz using RF clock:
S:>setclock r56.789

Read current clock speed:
S:>setclock

For more options:
S:>setclock -h




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
No guarantee is given for the usability of this code.


Version History:
2018-04-11 HH v1.07 - Added showing fuse parameters.
2018-04-05 HH v1.06 - Speed test made more reliable when multitasking.
2017-12-05 HH v1.05 - Ported to VS1005h.
2017-05-04 HH v1.04 - Now allows setting RF clock to other frequencies than
                      only 60 MHz USB.
2017-01-11 HH v1.03 - UART speed was in some cases left incorrect, fixed.
                      Stability fixes when going to highest frequencies.
2016-12-12 HH v1.02 - All parameters were not set in the right order, fixed.
2016-11-02 HH v1.01 - Can set many parameters, as well as automatically
                      change e.g. core voltage when modifying clock.
2015-08-27 HH v1.00 - First release.
