
	    CYCLIC DRIVER DEMO
	    ------------------
	2017-12-05 VLSI Solution Oy


The cyclic driver demo consists of two parts:
1) The cyclic driver RunCyc.dl3, which contains a counter that steps
   forwards ten times a second.
2) A client program ReadCyc.dl3 that displays the counter.

To see how the system works, try this:
S:>ReadCyc

Fatal error: symbol _tenthsCounter not found (missing library or too old kernel). Stop.


Here the cyclic driver hadn't started yet, so the symbol tenthsCounter
isn't available yet.

Reboot and try again:

S:>driver +RunSyc
S:>ReadCyc
Tenths counter 34

First the RunCyc driver is loaded, then the counter can be read with ReadCyc.


Alternatively you can load the driver in config.txt:
RUNCYC


See the source code for details.

Version History:
2017-12-05 HH v1.01 - Ported to VS1005h.
2016-09-01 HH v1.00 - First release.
