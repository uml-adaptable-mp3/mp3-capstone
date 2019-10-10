
    PROGRAM VS1005G FLASH METASOLUTION
    ----------------------------------
         VLSI Solution 2017-12-05


This README.TXT fill will instruct how to program an image file from a PC
to VS1005g's Internal 1 MiB FLASH or External 2 MiB SPI FLASH through UART.
To get an image file, you can use the DumpFlash solution.


To program an image file to the Internal 1 MiB FLASH:

1) Connect a the USB/UART converter cable between the PC and the VS1005g
   board.
2) Close any terminal emulation program that may use the UART.
3) Boot the VS1005g board in some mode that is NOT the VSOS Shell, e.g.
   USB drive mode (push S1 when booting).
4) Open this project (ProgramFlash) in VSIDE.
5) Copy the image file to the project folder with name eeprom.img.
6) In VSIDE, Select menu Project -> Prommer/Flasher Utility.
7) Select "VS1005g Internal Flash Prommer".
8) Let "Select image file to write" be "eeprom.img".
9) eeprom.img is programmed to the internal FLASH memory.
10) After programming, you may reboot the board and close the project.


To program an image file to the External 2 MiB FLASH:

1) Connect a the USB/UART converter cable between the PC and the VS1005g
   board.
2) Close any terminal emulation program that may use the UART.
3) Boot the VS1005g board in some mode that is NOT the VSOS Shell, e.g.
   USB drive mode (push S1 when booting).
4) Open this project (ProgramFlash) in VSIDE.
5) Copy the image file to the project folder with name eeprom.img.
6) In VSIDE, Select menu Project -> Prommer/Flasher Utility.
7) Select "VS1005g External SPI Flash Prommer".
8) Let "Select image file to write" be "eeprom.img".
9) eeprom.img is programmed to the external FLASH memory.
10) After programming, you may reboot the board and close the project.
