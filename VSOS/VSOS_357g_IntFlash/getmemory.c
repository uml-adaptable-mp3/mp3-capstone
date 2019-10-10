#include <vo_stdio.h>
#include "getmemory.h"
#include "vsos.h"
#include <timers.h>

u_int16 highestISoFar = 0;

u_int16 GetMemory(u_int16 page, u_int16 address, u_int16 sizeWords, u_int16 align) {
	u_int16 res = 0;

	//printf("GM(%d, 0x%04x, 0x%04x, 0x%03x)", page, address, sizeWords, align);

	if (address) {
		switch(page) {
			case 1:
				res = (u_int16)AllocMemAbsX(address, sizeWords);
				break;
			case 2:
				res = (u_int16)AllocMemAbsY(address, sizeWords);
				break;
			default:
				if ((address >= 256) && (address < 0x8000u)) {
					SysError("ld");
				} else {
					res = address; //allow single address I absolutes (for int vectors)
				}
				break;
		}
	} else {
		switch(page) {
			case 0:
				res = (u_int16)__AllocMemI(sizeWords);
				if ((res+sizeWords) > highestISoFar) {
					highestISoFar = res+sizeWords;
					if (highestISoFar > 0x7000) {
						fprintf(vo_stderr,"IMEM: only %dw free!\n",0x7fc0-highestISoFar);
					}
				}
				break;
			case 1:
				res = (u_int16)AllocMemX(sizeWords, align);
				break;
			case 2:
				res = (u_int16)AllocMemY(sizeWords, align);
				break;
			default:
				SysError("ld");
				break;
		}
	
	}

	if (!res) {
		Delay(1000);
		SysError("Out of mem %d (@%d,%dw,%d).\n",page, address, sizeWords, align);
		while(1) {
			Delay(1000);
		}
		res = -1;
	}
	//printf(" = %04x\n", res);
	return res;
}
