/// \file devSdi.c SPI Data Input
/// \author Pasi Ojala, VLSI Solution Oy

/// Based on devHello example.

/// This device creates a disk, from which, when any file is opened, SPI data received to SPI1 port will be returned.
/// Try for example to write "DEVSDI D" and "PLAYFILE D:" in your CONFIG.TXT, and then send an MP3 file
/// to SPI1 with a microcontroller.

/// This device initializes and uses SPI1 interrupt. Unloading this device driver would not be a good idea (no fini()).

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include "simpledevice.h"
#include <string.h>
#include "spireceivedreq.h"
#include <timers.h>
#include <iMem.h>
#include <kernel.h>

//See spireceivedreq.h for definitions. Currently DREQ = TMS, CANCEL = TCK.

extern const SIMPLE_DEVICE sdiDevice;

ioresult main(char *parameters) {
	vo_pdevices[(*parameters|0x20)-'a'] = &sdiDevice;
}

const char*  DevSdiIdentify (register __i0 void *self, char *buf, u_int16 bufsize) {
	return "DevSdi";
}

static int odd = 0; //non-zero means we have an extra byte in old..
static u_int16 old = 0;
ioresult DevSdiOpen(register __i0 SIMPLE_FILE *self, const char *name, const char *mode){ ///< Start device, populate descriptor, find and start filesystem.
	//TODO:BUG: should allocate resources!
	odd = old = 0;
	SpiFIFOYVars.wr = SpiFifoY;
	SpiFIFOYVars.rd = SpiFifoY;
	PERIP(GPIO1_MODE) |= (15<<4); //SPI1 pins to PERIP
	WriteIMem(32+INTV_SPI1, ReadIMem((u_int16)SpiFifoYInt-1));
	PERIP(GPIO2_MODE) &= ~((1<<GPIO_DREQ_BIT)|(1<<GPIO_CANCEL_BIT));
 	PERIP(GPIO2_DDR)  |= (1<<GPIO_DREQ_BIT);
 	PERIP(GPIO2_DDR)  &= ~(1<<GPIO_CANCEL_BIT);
	//spi receiver config
	PERIP(SPI1_CF) = SPI_CF_INTXCS | SPI_CF_SLAVE | SPI_CF_DLEN16;
	PERIP(INT_ORIGIN0) = INTF_SPI1;
	PERIP(INT_ENABLE0_HP) |= INTF_SPI1;
	PERIP(GPIO2_SET_MASK) = (1<<GPIO_DREQ_BIT);

	self->pos = 0;
	return S_OK;
}

ioresult DevSdiClose(register __i0 SIMPLE_FILE *self){ ///< Flush, Stop, Clean and Free the descriptor of the device
	//TODO:BUG: should deallocate resources!
	PERIP(INT_ENABLE0_HP) &= ~INTF_SPI1;
	PERIP(GPIO2_CLEAR_MASK) = (1<<GPIO_DREQ_BIT);
	return S_OK;
}

IOCTL_RESULT DevSdiIoctl(register __i0 SIMPLE_FILE *self, s_int16 request, IOCTL_ARGUMENT arg){ ///< Reset, Start, Stop, Restart, Flush, Check Media etc
	return S_ERROR;
}

u_int16  DevSdiRead(register __i0 SIMPLE_FILE *self, void *buf, u_int16 destinationIndex, u_int16 bytes){ ///< Read bytes to *buf, at byte index
	u_int16 read = 0;
	if (!bytes) {
		return 0;
	}
	if (PERIP(GPIO2_DDR) & (1<<GPIO_CANCEL_BIT)) {
		goto eof;
	}
//printf("R%d %d\n", bytes, SpiFifoYFill());
	if (odd) {
		/* a byte is waiting, copy it first */
		odd = 0;
		MemCopyPackedBigEndian(buf, destinationIndex, &old, 1, 1);
		if (bytes <= 1) {
			return 1;
		}
		destinationIndex++;
		bytes--;
		read++;
	}
	if (destinationIndex & 1) {
		u_int16 tmpBuf[16];
		/* odd alignment, have to use a temporary buffer */
		while (bytes > 1) {
			s_int16 toRead = bytes & ~1;
			if (toRead > 2*sizeof(tmpBuf)) {
				toRead = 2*sizeof(tmpBuf);
			}
			while (SpiFifoYFill() < toRead/2) {
				Delay(1);
				if (PERIP(GPIO2_DDR) & (1<<GPIO_CANCEL_BIT)) {
					goto eof;
				}
			}
			if (PERIP(GPIO2_DDR) & (1<<GPIO_CANCEL_BIT)) {
				goto eof;
			}
//puts("0FifoYGet\n");
			/* not aligned, read to temporary buffer first */
			SpiFifoYGet(tmpBuf, toRead/2);
//puts("0MemCpy\n");
			MemCopyPackedBigEndian(buf, destinationIndex, tmpBuf, 0, toRead);
			destinationIndex += toRead;
			bytes -= toRead;
			read += toRead;
		}
		if (bytes & 1) {
			while (SpiFifoYFill() < 1) {
				Delay(1);
				if (PERIP(GPIO2_DDR) & (1<<GPIO_CANCEL_BIT)) {
					goto eof;
				}
			}
			//puts("1 FifoYGet\n");
			if (PERIP(GPIO2_DDR) & (1<<GPIO_CANCEL_BIT)) {
				goto eof;
			}
			SpiFifoYGet(&old, 1);
			odd = 1;
			((u_int16 *)buf)[destinationIndex/2] = (((u_int16 *)buf)[destinationIndex/2] & 0xff00) |  ((old >> 8) & 0xff);
			destinationIndex++;
			read++;
		}
	} else {
		s_int16 toRead = (bytes+1)/2;
		while (SpiFifoYFill() < toRead) {
			Delay(1);
			if (PERIP(GPIO2_DDR) & (1<<GPIO_CANCEL_BIT)) {
				goto eof;
			}
		}
		if (PERIP(GPIO2_DDR) & (1<<GPIO_CANCEL_BIT)) {
			goto eof;
		}
//printf("FifoYGet %d bytes\n", bytes);
		SpiFifoYGet(buf+(destinationIndex/2), bytes/2);
		destinationIndex += bytes;
		if (bytes & 1) {
			while (SpiFifoYFill() < 1) {
				Delay(1);
				if (PERIP(GPIO2_DDR) & (1<<GPIO_CANCEL_BIT)) {
					goto eof;
				}
			}
//puts("2 FifoYGet\n");
			SpiFifoYGet(&old, 1);
			odd = 1;
			((u_int16 *)buf)[destinationIndex/2] = (((u_int16 *)buf)[destinationIndex/2] & 0xff) |  (old & 0xff00);
		}
		read = bytes;
	}
	self->pos += read;
	return read;
eof:
	self->flags |= __MASK_EOF;
	return 0;
}

u_int16  DevSdiWrite(register __i0 SIMPLE_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes){ ///< Write bytes from *buf at byte index
	return 0;
}

const SIMPLE_DEVICE sdiDevice = {
	__MASK_PRESENT | __MASK_CHARACTER_DEVICE | __MASK_READABLE | __MASK_OPEN,
	DevSdiIdentify,
	DevSdiOpen,
	DevSdiClose,
	DevSdiIoctl,
	DevSdiRead,
	DevSdiWrite
};

