
                Sine v1.01
                ----------
          for VSOS 3.50 or higher
         2017-12-05  VLSI Solution




Sine is a program that makes it possible to generate one or several
sine waveforms with better than 24-bit acccuracy.




Usage: Sine freq1 dbl1 dbr1 [freq2 dbl2 dbr2 [...]] [-rrate|-h]
freq dbl dbr    Set frequency to freq Hz, left volume to dbl dB,
                right volume to dbr dB. If dbl or dbr is greater than zero,
                volume is muted for that channel.
-rrate  Set sample rate to rate Hz
-h      Show this help

Examples:
  Sine 1000 0 0
  Sine -r24000 1000 -6 1 2000 1 -6 3000 -6.1 -6.1




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2017-12-05 HH v1.01 - Ported to VS1005h.
2016-09-26 HH v1.00 - First release.
