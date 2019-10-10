*** Chinese-Japanese-Korean support (UNIFONT)

This folder contains different versions of UNIFONT.DAT.
To run UNIFONT-capable programs with Chinese/Japanese/Korean language support,
copy one of the supplied files to your SYS folder as name UNIFONT.DAT.

Contact VLSI to obtain different versions of UNIFONT with support for different
languages.


*** root/ , small_root/ , and SYS_everything/

These folders contain a collection of files and folders which you should copy
to your 'S:' System Disk (flash) after promming the kernel with VSIDE and
UART cable.

root/ is the "normal" root, and small_root has a limited set of drivers and
utilites.

small_root/ is suitable for the internal flash or VS1005 breakout board.

Since the size of the flash in the boards is limited, we've had to decide, 
which files we put into the root/ and small_root/. SYS_everything/ is a
folder that contains practically all the libraries and programs offered
by VLSI. If you need a library, device driver or utility which is missing
from your root image, copy it from this folder to the SYS/ folder in your
flash.


*** config_audio_decoders.txt

You may use this configuration file instead of the default config.txt for
applications that need the maximum performance out of high-resolution audio
decoders. Read the comments in the file, and copy on top of default
config.txt if applicable.
