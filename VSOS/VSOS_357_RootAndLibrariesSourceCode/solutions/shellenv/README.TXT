
        SHELLENV - Shell Environment emulation driver 1.02
        --------------------------------------------------
                   for VSOS 3.50 or higher
                  2017-12-05  VLSI Solution




SHELLENV.DL3 makes it possible to run programs that expect the shell
environment current directory concept to work, e.g. CD.DL3 or DEL.DL3.

Note that this driver contains the functionality of the earlier
SETFOPEN.DL3 driver. You should not load both.


To activate the driver, add the following line to config.sys, before
any drivers or programs that might need the environment:
SHELLENV




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2017-12-05 HH v1.02 - Ported to VS1005h.
2017-05-18 HH v1.01 - Uses Mutex to avoid two recursive relative paths which
                      it can't handle.
2017-01-17 HH v1.00 - Developed from the SETFOPEN.DL3 driver.
