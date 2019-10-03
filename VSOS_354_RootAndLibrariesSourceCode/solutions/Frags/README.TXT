
        Frags / MEMTRACK - VS1005 Memory Consumption Monitor
        ----------------------------------------------------
                       for VSOS 3.50 or higher
                      2017-12-05  VLSI Solution



Frags.dl3, potentionally with some help of the MEMTRACK.DL3 memory
tracker, can display VS1005 I, X, and Y memory contents as ASCII
graphics, plus, to some extent, which programs have allocated them.

You can run Frags from the VSOS Shell command line. However, more
detailed output can be shown if you add the following line as the
first line of config.sys:
MEMTRACK

You may also load the memory tracker from the VSOS Shell as follows:
S:>driver +memtrack
However, in this case the memory tracker cannot give as detailed
information as if it was started earlier in the boot process.

The output of Frags looks as follows:

S:>frags
Libs: Memtrack Sdsd Auodac auIi2ss auXsyncs auxPlay Run Uartin sHell Frags
I 0000: ################ ################ ################ ################
I 1000: ################ ################ ################ ################
I 2000: ################ ################ ################ ################
I 3000: MMMMMMMMMSSSSSSS SSSSSSSSSSSAAAAA AAAAAAAIIIIIIIIX XXXXXXXXPPPPPRUU
I 4000: UUUUUUHHHFFFFFFF FFFFFFF......... ................ ................
I 5000: ................ ................ ................ ................
I 6000: ................ ................ ................ ................
I 7000: ................ ................ ................ ...............#

X 0000: ################ ################ ################ ################
X 1000: ################ ################ ################ ##############SS
X 2000: ##IXPP#PPPP#HHHH HHHHHFFFFFF#.... ................ ................
X 3000: ................ ................ ................ ................
X 4000: ................ ................ ................ ................
X 5000: ................ ................ ................ ................
X 6000: ................ ................ ................ ................
X 7000: ................ ................ ................ ................

Y 0000: ################ ################ ################ MMMMMMMFIIIIIIII
Y 1000: UUUUUU.......... ................ ................ .####...........
Y 2000: .......PPPP..... ................ ................ ................
Y 3000: ................ ................ ................ ................
Y 4000: ................ ................ ................ ................
Y 5000: ................ ................ ................ ................
Y 6000: ................ ................ ................ ................
Y 7000: ################ ################ ################ ################

Free: I: 14864 words (45%), X: 22788 words (69%), Y: 23648 words (72%)


The first line shows the libraries that have been loaded. Below that,
allocated and free Instruction, X Data, and Y Data memories are shown.
Allocated unknown areas (usually reserved by VSOS) are marked with '#',
free areas with '.', and areas allocated by libraries with a capital
letter (e.g. 'M' of Memtrack). Without the MEMTRACK driver, only
statically allocated memory areas are shown, but with it also
dynamically allocated areas may be shown. Finally, the total free amount
of each memory type is shown.



License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.


Disclaimer:
No guarantee is given for the usability of this code.


Version History:
2017-12-05 HH v1.01 - Ported to VS1005h.
2016-02-10 HH v1.00 - First release.
