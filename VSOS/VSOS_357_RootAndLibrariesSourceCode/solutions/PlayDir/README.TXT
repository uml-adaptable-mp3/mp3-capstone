
        PlayDir 1.03 - UART-Controlled Audio Player
        -------------------------------------------
                 for VSOS 3.50 or higher
                2017-12-05  VLSI Solution



PlayDir plays all the files of the current directory, in alphabetical order.
It can be controlled through stdin, which, if the UARTIN driver is used,
is the UART.

For options, run:
S:>playdir -h
Usage: PlayDir [-v|+v|-p|+p|-s|+s|-h]
-v      Verbose
+v      Not verbose
-p      Start in pause mode
+p      Start in play mode
-s      Shuffle mode on
+s      Shuffle mode off
-h      Show this help

To use, either compile the VSIDE Solution, or simply copy PlayDir.dl3 to
your VS1005 Evaluation Board's SYS/ folder.

For more information on the command line options, as well as the input
and output of PlayDir, refer to the PDF document VS1005 VSOS Shell,
Chapter Using the UART Controlled Player.




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2017-12-05 HH v1.03 - Ported to VS1005h.
2017-03-14 HH v1.02 - Can now also control pitch / speed shifter with UART
                      characters '+', '-', '=', 'f', 's', 'b'.
2016-02-10 HH v1.01 - Tells buffer size to Dir to avoid buffer overflow.
2015-08-28 HH v1.00 - First release.
