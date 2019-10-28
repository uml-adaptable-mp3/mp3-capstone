/// \file DevSdDpi.c VsOS Device driver for SD card in SPI mode
/// \author Panu-Kristian Poiksalo and Pasi Ojala, VLSI Solution Oy

#include <stdlib.h>
#include <string.h>
#include "vsos.h"
#include "devSdSpi.h"
#include "vo_fat.h"	
#include "vs1005g.h"
#include "mmcCommands.h"
#include "devboard.h"
//#include "forbid_stdout.h"
#include "vo_stdio.h"
//#include "uart1005.h"


const DEVICE devSdSpiDefaults = {	
	0, //u_int16   flags; //< present, block/char
	DevSdSpiIdentify, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);	 
	DevSdSpiCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
	(ioresult(*)(DEVICE*))CommonOkResultFunction, //ioresult (*Delete)(DEVICE *dev);
	DevSdSpiIoctl, //ioresult (*Ioctl)(DEVICE *dev, s_int16 request, char *argp); //Start, Stop, Flush, Check Media
	// Stream operations
	0,
	0,
	// Block device operations
	DevSdSpiBlockRead, //ioresult (*BlockRead)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
	DevSdSpiBlockWrite, //ioresult (*BlockWrite)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
	// Stream operations
	//DevSdSpiInput, //ioresult (*Input)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
	//DevSdSpiOutput, //ioresult (*Output)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
	//FILESYSTEM *fs;
	//DIRECTORY *root; //Zero if the device cannot hold directories
	//u_int16  deviceInstance;
	//u_int16  deviceInfo[10]; // For filesystem use, size TBD	
};

void PrintBuffer(u_int16 *buf); //debug function in vo_fat.c


char* DevSdSpiIdentify(register __i0 void *obj, char *buf, u_int16 bufsize){
	return "SD Card in SPI mode";
}

static ioresult CardError (register __i0 DEVICE *ph, const char *msg) {
//	UartPutString("STATUS=");
//	UartPutString(msg);
//	UartPutString("=status");
	ph->Ioctl(ph, IOCTL_END_FRAME, 0); //Deselect device on bus
	return SysError(msg);
}
	
#if 0
void PrintR1(u_int16 r1) { //Print SD response 1 message
	if (r1==0xff) {
		printf("No response.");
		return;
	}
	if (r1==0x7f) {
		printf("<7F>");
		return;	
	}
	if (r1&1) {
		printf ("Idle");
	} else {
		printf ("Busy");
	}
	if (r1 & 0x2) printf (", Erase Reset");
	if (r1 & 0x4) printf (", Illegal command");
	if (r1 & 0x8) printf (", Wrong CRC");
	if (r1 & 0x10) printf (", Erase sequence error");
	if (r1 & 0x20) printf (", Address error");
	if (r1 & 0x40) printf (", Parameter error");
	printf(".");		
}


void PrintMmcCommand(u_int16 cmd){
	if (cmd == (MMC_GO_IDLE_STATE | 0x40)) printf("GO_IDLE");
	if (cmd == (MMC_SEND_OP_COND | 0x40)) printf("SEND_OP");
	if (cmd == (MMC_SEND_IF_COND | 0x40)) printf("SEND_IF");
	if (cmd == (MMC_SEND_CSD | 0x40)) printf("SEND_CSD");
	if (cmd == (MMC_SEND_CID | 0x40)) printf("SEND_CID");
	if (cmd == (MMC_STOP_TRANSMISSION | 0x40)) printf("STOP");
	if (cmd == (MMC_READ_SINGLE_BLOCK | 0x40)) printf("RD_SINGLE");
	if (cmd == (MMC_READ_MULTIPLE_BLOCK | 0x40)) printf("RD_MULTI");
	if (cmd == (MMC_CLR_WRITE_PROT | 0x40)) printf("CLR_WPROT");
	if (cmd == (55 | 0x40)) printf("ACMD");
	if (cmd == (41 | 0x40)) printf("ACMD41");
	if (cmd == (MMC_READ_OCR | 0x40)) printf("READ_OCR");
	
}
#endif




// Convert device driver calls from old style to new style
#define Output(p,v,b,n) Write(p,b,v,n)
#define Input(p,v,b,n) Read(p,b,v,n)

//	static u_int16 mmcn=0;


u_int16 MmcCommand(s_int16 cmd, u_int32 arg, DEVICE *ph) {
	//Output(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
	u_int16 i,j;
	u_int16 buf[] = {0xffffU, 0xffffU, 0xffffU};
//   UartPutString("MC=");
//   mmcn++;
//   UartWrite(&mmcn,2);
//   UartWrite(&cmd,2);
//   UartWrite(&arg,4);
//   UartPutString("=mc");                                            	
	
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
	//printf("  MmcCommand %02x%08lx:",cmd,arg);	
	//PrintMmcCommand(cmd);printf(":");
	//printf("do mmccommand %d\n",cmd);
	ph->Ioctl(ph, IOCTL_END_FRAME, 0); //Deselect just in case
	ph->Output(ph, 0, buf, 6); //Send some clocks to the SD card
	ph->Ioctl(ph, IOCTL_START_FRAME, 0); //Select card
	
	if (cmd != (MMC_STOP_TRANSMISSION|0x40)) {
	
		ph->Output(ph, 0, buf, 6); //Send some clocks to the SD card
		buf[0] = 0x00;
		i = 3000;
		while ((--i) && (ph->Input(ph, 0, 0, 0) != 0xff))
		;
		if (i==0) {
			return CardError(ph,"SD Busy");
		}
		//printf("(%d waitcycles)",300-i);
	}
	
	buf[0] = (cmd << 8) | (u_int16)(arg >> 24);
	buf[1] = (u_int16)(arg >> 8);
	buf[2] = (u_int16)(arg << 8) | 0x95;
	ph->Output(ph, 0, buf, 6); //Send out the MMC command
	for (i=0; i<100; i++) {
		j = ph->Input(ph, 0, 0, 0);
		if (j != 0xff) break;
	}
	//printf("%02x:",j);
	//PrintR1(j);
	//printf("\n");
//   UartPutString("MR=");
//   UartWrite(&mmcn,2);
//   UartWrite(&j,2);
//   UartPutString("=mr\r");                                            	

	return j;
}


ioresult DevSdSpiCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo) {
	devSdSpiHwInfo* hw=(devSdSpiHwInfo*)dev->hardwareInfo;	
	memcpy(dev, &devSdSpiDefaults, sizeof(*dev));	
	dev->deviceInstance = __nextDeviceInstance++;	
	hw->ph = (DEVICE*)name; //Get pointer to SPI physical from the second parameter.
	//debug_printf("DevSdSpi: Trying to create SD card device using %s port.\n",ph->Identify(ph,0,0));
	return dev->Ioctl(dev, IOCTL_RESTART, 0);
}



ioresult DevSdSpiBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	devSdSpiHwInfo* hw=(devSdSpiHwInfo*)dev->hardwareInfo;
	DEVICE *ph=hw->ph;
	if (data == 0) {
		return CardError(ph, "Null Pointer");
	}
	if (!__F_CAN_SEEK_AND_READ(dev)) {
		return CardError(ph, "Seek");
	}
	//debug_printf("DevSdSpi: BlockRead sector %ld\n",firstBlock);
	
	while (blocks) {
#if 1
		if (hw->multipleReadNextBlock != firstBlock) {
			if (hw->multipleReadNextBlock != -1) {
				/*end read*/
				//printf("Multiple end: 0x%lx\n", firstBlock);
				MmcCommand(MMC_STOP_TRANSMISSION|0x40, 0, ph);
				do {
					/*TODO: timeout?*/
				} while (ph->Input(ph, 0,0,0) != 0xff);
				//printf("stopped\n");
			}
			//printf("Multiple first: 0x%lx\n", firstBlock);
			if (MmcCommand(MMC_READ_MULTIPLE_BLOCK|0x40, (firstBlock<<hw->hcShift), ph) == 0xff) {
				//printf("First failed, restart device");
				dev->Ioctl(dev, IOCTL_RESTART, 0);
				MmcCommand(MMC_STOP_TRANSMISSION|0x40, 0, ph);
				do {
					/*TODO: timeout?*/
				} while (ph->Input(ph, 0,0,0) != 0xff);
				if (MmcCommand(MMC_READ_MULTIPLE_BLOCK|0x40, (firstBlock<<hw->hcShift), ph) == 0xff) {
					//printf("First failed, really.");				
					dev->flags &= ~__MASK_OPEN; //Set the device closed
					return CardError(ph, "Card Removed1");
				}
			}
		} else {
			//printf("Multiple next: 0x%lx\n", firstBlock);
			ph->Ioctl(ph, IOCTL_START_FRAME, 0); //select
		}
		hw->multipleReadNextBlock = firstBlock + 1;
		{
			u_int16 c;
			u_int16 t = 65535;
			do {
				c = ph->Input(ph, 0, 0, 0);
			} while (c == 0xff && --t != 0);
			if (c != 0xfe) {
//printf("No data: 0x%02x (or timeout)", c);
				//memset(data, 0, 256);
				dev->flags &= ~__MASK_OPEN; //Set the device closed
				return CardError(ph, "Card Removed2");
			}
			ph->Input(ph, 0, data, 512);
			ph->Output(ph, 0xff, 0, 2);	/* Discard CRC */
		}
#else
		if (MmcCommand(MMC_READ_SINGLE_BLOCK | 0x40, firstBlock << hw->hcShift, ph) == 0xff) {
			dev->flags &= ~__MASK_OPEN; //Set the device closed
			return CardError(ph, "Card Removed3");
		}
		i=0;
		while ((r = ph->Input(ph,0,0,0)) == 0xff) {
			if (++i>32000) {
				return CardError(ph, "SD Timeout");
			}
		}
		if (r != 0xfe){
			memset(data, 0, 256*blocks);			
			return CardError(ph, "SD Data");
		}		
		ph->Input(ph, 0, data, 512);		
		//PrintBuffer(data);
		ph->Output(ph, 0xff, 0, 2); //Send 2 bytes of 0xff to read out the CRC
#endif		
		blocks--;					
		data += (512/2);
		firstBlock++;
	}
	ph->Ioctl(ph, IOCTL_END_FRAME, 0); //Deselect just in case			
	return S_OK;
}


ioresult DevSdSpiBlockWrite(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	devSdSpiHwInfo* hw=(devSdSpiHwInfo*)dev->hardwareInfo;
	DEVICE *ph=hw->ph;
	u_int16 errorCount=0;
	
	if (!__F_CAN_SEEK_AND_WRITE(dev)) {
		return S_ERROR;
	}
	if (data == 0) {
		return CardError(ph, "Null Pointer");
	}
	//debug_printf("<b>DevSdSpi: BlockWrite sector %ld</b>\n",firstBlock);
	//PrintBuffer(data);
#if 1
	if (hw->multipleReadNextBlock != -1) {
		/*end read*/
		//printf("Write: STOP_TRANSMISSION\n");
		MmcCommand(MMC_STOP_TRANSMISSION|0x40, 0, ph);
		do {
			/*TODO: timeout?*/
		} while (ph->Input(ph, 0,0,0) != 0xff);
		hw->multipleReadNextBlock = -1;
	}
#endif
	while (blocks) {
		u_int16 c;
		retryWrite:
		c = MmcCommand(MMC_WRITE_BLOCK | 0x40, firstBlock << hw->hcShift, ph);
		if (c==0xff) {
			dev->flags &= ~__MASK_OPEN; //Set the device closed
			return CardError(ph, "Card Removed4");
		}	
		{
			s_int16 t = 0;
			while (c != 0x00) {				
				if (++t < 0 || c == 0x01) { // timeout
					return CardError(ph, "Pre-Write Fail");
				}
				c = ph->Input(ph,0,0,0); //SpiSendReceiveMmc(0xff00, 8);
			}
		}		
		ph->Output(ph, 0xfe, 0, 1); //send 1 byte of 0xfe   
		ph->Output(ph, 0, data, 512); //send the data block
		ph->Output(ph, 0xff, 0, 2); //send 2 bytes of 0xff for CRC		
		//wait until MMC write is finished				
		{
			u_int16 i = 0;
			while (((ph->Input(ph,0,0,0)) != 0xff) && (++i < 32000))
			;
			ph->Ioctl(ph, IOCTL_END_FRAME, 0); //Deselect
			ph->Output(ph, 0xff, 0, 4); //send 4 bytes of 0xff
			if (i >= 32000) return SysError("Post-Write Fail");
		}
		
		#if 0 //Don't verify
		{
			int i;
			static u_int16 verifyBuffer[256];
			DevSdSpiBlockRead(dev, firstBlock, 1, verifyBuffer);
			for (i=0; i<256; i++) {
				if (verifyBuffer[i] != data[i]) {
					//printf("VERIFY ERROR - WRITE %ld:\n",firstBlock);
					//PrintBuffer(data);
					//printf("IS NOT SAME AS READBACK:\n");
					//PrintBuffer(verifyBuffer);
					if (++errorCount > 2) {
						return SysError("Verify");
					}
					//printf("Retry write\n");
					goto retryWrite;
				}
			}
			//printf(" Verify ok.\n");
		}
		#endif
		data += 256;
		firstBlock++;
		blocks--;
	}
	ph->Ioctl(ph, IOCTL_END_FRAME, 0); //Deselect just in case			
	return S_OK;
}


ioresult DevSdSpiIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp) {
	devSdSpiHwInfo* hw=(devSdSpiHwInfo*)dev->hardwareInfo;
	DEVICE *ph = hw->ph;
	u_int16 i, n, cmd;

	
	//printf(" SwSpiIoctl:%d ",request);
	switch (request) {
		case IOCTL_START_FRAME:			
			break;
		
		case IOCTL_END_FRAME:
			break;

		case IOCTL_RESTART: {	
				
			#if 1
			hw->multipleReadNextBlock = -1;
			#endif
			

			tryagain:
			
			MmcCommand(MMC_STOP_TRANSMISSION|0x40, 0, ph);
			
			for (n=10; n; n--) {
				//printf("SD Go Idle\n");
				printf(".");
				DelayL(10000);
				ph->Ioctl(ph, IOCTL_START_FRAME, 0); //Select device on bus
				ph->Output(ph,0xff,0,514); //send 514 bytes of 0xff
				ph->Ioctl(ph, IOCTL_END_FRAME, 0); //Deselect device on bus
				ph->Output(ph,0xff,0,512); //send 512 bytes of 0xff

				/* MMC Init, command 0x40 should return 0x01 if all is ok. */
				i = MmcCommand(MMC_GO_IDLE_STATE/*CMD0*/|0x40,0,ph);
				//printf("MMC=%d ",i);
				ph->Output(ph,0xff,0,512); //send 512 bytes of 0xff
				//printf("i=%d.",i);
				
				if (i == 1)
				break;
			}
			if (!n) {
				dev->flags = 0;
				ph->Ioctl(ph, IOCTL_END_FRAME, 0); //Deselect device on bus
				return SysReport("No Card");
			}
			
			

			
			cmd = MMC_SEND_OP_COND|0x40;
			//printf("Send IF Cond: ");
			{
				u_int16 mmc_if;
				if (mmc_if = (MmcCommand(MMC_SEND_IF_COND|0x40, 0x00000122/*2.7-3.6V*/, ph) /* & ~0x04 */) == 1) {
					/* MMC answers: 0x05 SD answers: 0x01 */
					/* Should we read the whole R7 response? */
					cmd = 0x40|41; /* ACMD41 - SD_SEND_OP_COND */
				}
				//if (mmc_if == 0xfe) {
				//	printf("StopTransmission ");
				//	
				//}
			}
			/* MMC Wake-up call, set to Not Idle (mmc returns 0x00)*/
			//i = 0; /* i is 1 when entered .. but does not make the code shorter*/
			while (1) {
				register int c;
				//printf("\n");
				//printf("Here ");
				if (cmd == (0x40|41)) {
					MmcCommand(0x40|55/*CMD55*/,0,ph);
					c = MmcCommand(cmd/*MMC_SEND_OP_COND|0x40*/, 0x40000000UL, ph);
				} else {
					c = MmcCommand(cmd/*MMC_SEND_OP_COND|0x40*/, 0, ph);
				}
				//printf(" c=%d. ",c);
				if (c == 0)
				break;
					
				//printf("i=%d ",i);
				if (++i >= 10/*< 0*/ || c != 1) {
					//printf("toTryAgain ");
					goto tryagain; /* Not able to power up mmc */
				}
			}
			//printf("out");

			hw->hcShift = 9;
			/* PRETEC's 1G MMC does not seemd to handle CMD58 here! */
			/* Support HC for SD, but not for MMC! */
			if (cmd == (0x40|41) &&
			MmcCommand(MMC_READ_OCR/*CMD58*/|0x40, 0, ph) == 0) {
				u_int16 buf;
				ph->Input(ph, 0, &buf, 2); //read 2 bytes (1 word) to buf
				if (buf & (1<<(30-16))) { /* OCR[30]:CCS - card capacity select */
					/* HighCapacity! */
					hw->hcShift = 0;
				}
			}
	   	    		 	
			/* Set Block Size of 512 bytes -- default for at least HC */
			/* Needed by MaxNova S043618ATA 2J310700 MV016Q-MMC */
			/* Must check return value! (MicroSD restart!) */
			i = MmcCommand(MMC_SET_BLOCKLEN|0x40, 512, ph);
			if (i != 0) {
				goto tryagain;
			}
			/* All OK return */
			//printf("MMC Init Ok.\n");
		
			hw->ph = ph; //set the physical device (SPI port) to hwinfo
		
			dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;
			dev->fs = StartFileSystem (dev, "0"); // Find a filesystem, which understands this device
			ph->Ioctl(ph, IOCTL_END_FRAME, 0); //Deselect device on bus
			
			if (dev->fs) {
				//printf("DevSdSpi: Created.\n");
				return S_OK;
			} else {
				//Set device to be present, but not open, because no filesystem can handle it.
				//printf("DevSdSpi: Cannot create because no filesystem can handle it.\n");
				dev->flags = __MASK_PRESENT | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;
				return S_ERROR; //Filesystem not initialized
			}
		}
		
	
					
	default:
		return S_ERROR;
		break;
	}			
	return S_OK;
}
		
	
