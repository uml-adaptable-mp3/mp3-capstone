
	   Interrupt Demo 1.01
	   -------------------
	 for VSOS 3.50 or higher
	2017-12-05  VLSI Solution


This is an interrupt demo which demonstrates how to take an VS1005
interrupt under your control in C language.

The example activates three interrupts:
- GPIO0: interrupt-driven capture for GPIO0, display changes to
  GPIO0_0 through GPIO0_3 (buttons S1 through S4 on the VS1005 Development
  Board).
- REGU: interrupt-driven capture for Power Button.
- TIMER1: timer interrupt that goes off once a second.

Stubs are provided for all VS1005g's interrupts. However, do note that some
interrupts are used by VSOS and its drivers, so all of them may not be
used freely.

See the .s files for each interrupt to see how to get control of it in C.


NOTE!
You should write as short interrupt routines as possible. In the
VSDSP architecture interrupts can _not_ be interrupted, so a long
interrupt will cause problems with system performance.

NOTE!
Interrupts should run at lowest interrupt priority unless for critical
performance reasons.


Version History:
2017-12-05 HH v1.01 - Ported to VS1005h.
