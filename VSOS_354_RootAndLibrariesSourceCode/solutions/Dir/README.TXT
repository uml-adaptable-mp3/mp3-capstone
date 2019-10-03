
               DIR 1.17
               --------
        for VSOS 3.52 or higher
       2018-01-31  VLSI Solution




Dir is a VSOS Shell program that lists the contents of the current
directory, or a full path. For options, run:
S:>dir -h
Usage: dir [-s|-sn|-st|-ss|+s|-r|+r|-d|+d|-a|+a|-f|+f|-h] [path]
-s|-sn  Sort files by name
-st     Sort files by date
-ss     Sort files by size
+s      Don't sort files (faster with large directories)
-r      Reverse sort (if sort selected)
+r      Forwards sort (if sort selected)
-d      Show file date
+d      Don't show file date
-a      Only audio files
+a      All files
-f      Fast listing (don't show play time for MP3 files with -a)
+f      Slower listing (show play time also for MP3 files with -a)
-v      Verbose on
+v      Verbose off
-h      Show this help page

To use, either compile the VSIDE Solution, or simply copy Dir.dl3 to
your VS1005 Evaluation Board's SYS/ folder.




License:
This code may be used freely in any product containing one or more ICs
by VLSI Solution.




Disclaimer:
No guarantee is given for the usability of this code.




Version History:
2018-01-31 HH v1.17 - Has new -v option that shows and verifies cluster records
                      for each file listed.
2017-12-05 HH v1.16 - Ported to VS1005h.
2017-05-03 HH v1.15 - Skips disk label in a way compatible with VSOS 3.42.
                      Fixed bug that would return incorrect number of audio
                      files when MakePlayList() / DIR ENTRY_1 is called.
2017-04-20 HH v1.14 - Better FLAC output. Now prints -0:00.0 for audio files
                      with unknown length. Reacts to CTRL-C.
2017-03-13 HH v1.13 - Audio file recognition works for ADTS AAC files.
2017-01-19 HH v1.12 - Size-optimized code and minor bug fixes.
2016-10-20 HH v1.11 - Fixed pointer bug with -a option.
2016-04-26 HH v1.10 - Many new audio formats supported for -a option.
                      Relative paths also supported.
2016-03-11 HH v1.04 - Now also handles MP2 when in audio mode.
2016-02-10 HH v1.03 - Doesn't output file names if called as library function.
2015-09-23 HH v1.02 - Shows file dates. Added sorting by size, date, reverse.
2015-05-29 HH v1.01 - Sorted listing, plus other options.
2015-05-20 HH v1.00 - First release.
