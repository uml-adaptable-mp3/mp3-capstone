# mp3-capstone

[![Build Status](https://travis-ci.org/uml-adaptable-mp3/mp3-capstone.svg?branch=master)](https://travis-ci.org/uml-adaptable-mp3/mp3-capstone)

Adaptable MP3 Player Capstone Project for the Seven Hills Foundation.

The GitHub Repository is located [here](https://github.com/uml-adaptable-mp3/mp3-capstone)

## Contents
- [Contents](#contents)
- [Building](#building)
- [Loading New Software](#loading-new-software)
- [Info](#info)


## Building

The software must be built from a Windows machine, and currently requires a bash
terminal like [Git Bash](https://gitforwindows.org/).

To build the software:

1. In a bash terminal, navigate to the `build` folder.
2. Run `./build.bat`
3. All of the required files will be built and added to the `loadable/` directory.

## Loading New Software

To load the software, first configure the VS1005g for software loading:

1. Connect a jumper cable between Pin 9 (GPIO-0) and Pin 7 (IOVDD) of the VS1005.
2. Ensure no flash drive is connected to the USB port.
3. Connect a USB cable (from the VS1005's mini-USB port) to a PC.
   1. A device named `VSOSSYS` should appear as a removable drive
4. Copy all files in `loadable` to the VS1005
5. When complete, eject the drive and remove the jumper cable to return to normal operation.

## Info

VSOS is a free and open source operating system provided by [VLSI Solution](http://www.vlsi.fi/en/home.html) for their VS1005 SoC.
