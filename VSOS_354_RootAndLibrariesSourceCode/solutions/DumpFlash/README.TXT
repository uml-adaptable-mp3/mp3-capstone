
    DUMP VS1005G FLASH METASOLUTION
    -------------------------------
       VLSI Solution 2017-12-05


This README.TXT fill will instruct how to dump the contents of a VS1005g
Internal 1 MiB FLASH or External 2 MiB SPI FLASH through UART to a PC.
This image file can then be programmed to new VS1005g devices using the
ProgramFlash solution.


To dump contents of the Internal 1 MiB FLASH:

1) Connect a the USB/UART converter cable between the PC and the VS1005g
   board.
2) Close any terminal emulation program that may use the UART.
3) Boot the VS1005g board in some mode that is NOT the VSOS Shell, e.g.
   USB drive mode (push S1 when booting).
4) Open this project (DumpFlash) in VSIDE.
5) In VSIDE, Select menu Project -> Prommer/Flasher Utility.
6) Select "VS1005g Internal Flash Prommer".
7) Let "Select image file to write" be "eeprom.img".
8) The result is written to flashdump.img, size 1048576 bytes. If you
   check with a hex editor, the first four characters in the image
   should be either VLS5 or WLS5.
9) You may now reboot the board and close the project.


To dump contents of the External 2 MiB FLASH:

1) Connect a the USB/UART converter cable between the PC and the VS1005g
   board.
2) Close any terminal emulation program that may use the UART.
3) Boot the VS1005g board in some mode that is NOT the VSOS Shell, e.g.
   USB drive mode (push S1 when booting).
4) Open this project (DumpFlash) in VSIDE.
5) In VSIDE, Select menu Project -> Prommer/Flasher Utility.
6) Select "VS1005g Externap SPI Flash Prommer".
7) Let "Select image file to write" be "eeprom.img".
8) The result is written to flashdump.img, size 2097152 bytes. If you
   check with a hex editor, the first four characters in the image
   should be either VLS5 or WLS5.
9) You may now reboot the board and close the project.
