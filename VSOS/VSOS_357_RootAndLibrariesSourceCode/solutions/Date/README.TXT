
            Date, SetDate, RtcRead 1.04
            ---------------------------
              for VSOS 3.50 or higher
             2018-05-02  VLSI Solution




This package VSOS programs Date, SetDate, and RtcRead. They may also
be called with e.g. RunProgram("SETDATE", "2015-10-02 09:37:50").
Run the Date and SetDate programs with "-h" option for more
information.

To use, either compile the Solution, or simply copy Date.dl3,
SetDate.dl3 and RtcRead.dl3 to your VS1005 Evaluation Board's
SYS/ folder.

For these programs to work, you MUST install an LR44 battery (or
similar power source) to the VS1005 Evaluation Board battery
connector BATT1 (top right corner). Plus is on the top side.

Time can be set with either SetDate, or C function SetRtc().
E.g. SetRtc(mktime(tm)) where tm is a tm structure (see time.h
for details, and rtc.h for prototype for SetRtc()).

The programs are:
- Date - Displays current date and time (see -h for options)
- SetDate - Sets current date and/or time (see -h for options)
- RtcRead - Reads current RTC value to VSOS's currentTime struct.
	Values from this struct are used to set the file date
	whenever a write file is closed. VSOS 3.24 and newer call
	this program automatically before closing a write file.

Date.dl3, SetDate.dl3 and RtcRead.dl3 binaries will also work with
older VSOS versions, starting from VSOS 3.20, but only VSOS 3.24
or newer will automatically run RtcRead.dl3 to get the current date
every time before closing a file.




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2018-05-02 HH v1.04 - Weekday printed correctly, %B correctly explained with -h.
2017-12-05 HH v1.03 - Ported to VS1005h.
2015-10-02 HH v1.02 - Release for VSOS 3.25.
2015-09-21 HH v1.01 - First release.
