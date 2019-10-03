#!/bin/sh

vs3emu -chip vs1005 -monaddr 0x8000 -m mem_desc_debug_all.mem \
       -s 115200 -ts 115200 -x 12288 -p 1000 -e 0x80 -c kernel.cmd
