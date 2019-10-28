/// \file devSpiFlash.c VsOS Device driver for SPI Flash block devices
/// which have 24-bit address, 4K erase sector size and support page programming
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

/// CHANGE in v3.07 - chip size is passed as extrainfo parameter to DevSPIFlashCreate

/// \todo Use malloc to get the write cache (4K)
/// \todo Allow writing to more than one SPI flash at the same time - may corrupt data now

#include <stdlib.h>
#include <string.h>
#include <vo_stdio.h>
#include <timers.h>
#include <clockspeed.h>
#include "devSpiFlash.h"
#include <iochannel.h>
#include <hwLocks.h>


//#define CHIP_SIZE_SECTORS (1024*2) //NOT used - size is passed as extrainfo parameter to constructor
#define RESERVED_SECTORS (2*65536/512) // Reserve 128KB from the beginning of flash for boot code
#define CACHE_BLOCKS 8 ///< MUST be 8! \todo use malloc to get the memory as needed

#define SPI_FLASH_COMMAND_WRITE_ENABLE  0x06
#define SPI_FLASH_COMMAND_WRITE_DISABLE  0x04
#define SPI_FLASH_COMMAND_READ_STATUS_REGISTER  0x05
#define SPI_FLASH_COMMAND_WRITE_STATUS_REGISTER  0x01
#define SPI_FLASH_COMMAND_READ  0x03
#define SPI_FLASH_COMMAND_WRITE 0x02
#define SPI_FLASH_COMMAND_CLEAR_ERROR_FLAGS 0x30
#define SPI_FLASH_COMMAND_ERASE_BLOCK 0xD8
#define SPI_FLASH_COMMAND_ERASE_SECTOR 0x20
#define SPI_FLASH_COMMAND_ERASE_CHIP 0xC7


struct {
	CacheBlock ca[CACHE_BLOCKS];
} c;
u_int16 currentSectorInCache = 0;
u_int16 cacheIsDirty = 0;

#include "devHwSpi.h"

const DEVICE __y devSpiFlashDefaults = {   
  0, // u_int16 flags; //< present, block/char
  DevSpiFlashIdentify, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);   
  DevSpiFlashCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
  DevSpiFlashDelete, //ioresult (*Delete)(DEVICE *dev);
  DevSpiFlashIoctl, //ioresult (*Ioctl)(DEVICE *dev, s_int16 request, s_int32 arg); //Start, Stop, Flush, Check Media
  // Stream operations
  0, //u_int16  (*Read) (register __i0 DEVICE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
  0, //u_int16  (*Write)(register __i0 DEVICE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
  // Block device operations
  DevSpiFlashBlockRead, //ioresult (*BlockRead)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  DevSpiFlashBlockWrite, //ioresult (*BlockWrite)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  // Stream operations
  //DevSpiFlashInput, //ioresult (*Input)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
  //DevSpiFlashOutput, //ioresult (*Output)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
  //FILESYSTEM *fs;
  //DIRECTORY *root; //Zero if the device cannot hold directories
  //u_int16  deviceInstance;
  //u_int16  hardwareInfo[6]; ///< Device driver's info of this hardware
  //u_int16  deviceInfo[10]; // For filesystem use, size TBD   
};
void BlockRead(register __i0 DEVICE *ph, register u_int32 firstBlock, u_int16 *data);
 
#define END_FRAME 1
#define DONT_END_FRAME 0
  
ioresult DevSpiFlashCreate(register __i0 DEVICE *dev, void *name, u_int16 extraInfo) {
	SPIFLASH_HWINFO *hw = &(dev->hardwareInfo);
	//printf("DevSpiFlashCreate kutsuttu\n");
	memset(&c, 0, sizeof(c));
	memcpyYX(dev, &devSpiFlashDefaults, sizeof(DEVICE));
	hw->ph = (DEVICE*)name; //Get pointer to SPI physical from the second parameter.
	hw->currentWriteBuffer = NULL;
	hw->currentWriteBlock = 0;	
	hw->reservedSectors = extraInfo; //number of 512-byte sectors reserved (for VS1005 boot record)
	hw->totalBlocks = (extraInfo*2) - RESERVED_SECTORS;
	dev->deviceInstance = __nextDeviceInstance++;
	dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;	
	return dev->Ioctl(dev, IOCTL_RESTART, 0);
	
}

ioresult DevSpiFlashDelete(register __i0 DEVICE *dev) {
  return S_OK;
}

void SingleCycleCommand(register __i0 DEVICE *ph, u_int16 cmd) {
	ph->Ioctl(ph, IOCTL_START_FRAME, 0);
	ph->Write(ph, NULL, cmd, 1);
	ph->Ioctl(ph, IOCTL_END_FRAME, 0);
}

void MultipleCycleCommand(register __i0 DEVICE *ph, u_int16 *cmd, u_int16 cycles, u_int16 endFrame) {
	ph->Ioctl(ph, IOCTL_START_FRAME, 0);
	ph->Write(ph, cmd, 0, cycles);
	if (endFrame) {
		ph->Ioctl(ph, IOCTL_END_FRAME, 0);
	}
}

u_int16 SpiFlashWaitStatus(register __i0 DEVICE *ph) {
	u_int16 status;
	ph->Ioctl(ph, IOCTL_START_FRAME, 0);
	ph->Write(ph, NULL, SPI_FLASH_COMMAND_READ_STATUS_REGISTER, 1);
	do {
		status = ph->Read(ph,0,0,0);
	} while (status & 1);
	ph->Ioctl(ph, IOCTL_END_FRAME, 0);
	return status;
	
}

u_int16 GetREMS(register __i0 DEVICE *ph) { //Read manufacturer id
	u_int16 result = 0;
	MultipleCycleCommand(ph, "\p\x90\x00\x00\x00", 4, DONT_END_FRAME);
	ph->Read(ph, &result, 0, 2);
	ph->Ioctl(ph, IOCTL_END_FRAME, 0);		
	return result;
}

void FlashUnprotect(register __i0 DEVICE *ph) {
	SingleCycleCommand(ph, SPI_FLASH_COMMAND_WRITE_ENABLE);
	MultipleCycleCommand(ph, "\p\x01\x02", 2, END_FRAME); //Write Status Register (0x01): Sector Protect Off (0x02)
	SpiFlashWaitStatus(ph);
	SingleCycleCommand(ph, SPI_FLASH_COMMAND_WRITE_ENABLE);
}

const char *DevSpiFlashIdentify(register __i0 void *dev, char *buf, u_int16 bufsize) {
	DEVICE *ddev = dev;
	SPIFLASH_HWINFO *hw = &(ddev->hardwareInfo);
	DEVICE *ph=hw->ph;
	static char s[15];
	u_int16 rems = GetREMS(ph);
	sprintf(s,"SPI Flash %04x",rems);
	return s;
}

void PrintCache(void) {
	u_int16 i;
	//printf("\nCache: ");
	for (i=0; i<CACHE_BLOCKS; i++) {
		if (c.ca[i].flags) {
			//printf("%d:%d ",i,c.ca[i].sector);
		}
	}
}


u_int16 IsInCache(u_int16 sector) {
	u_int16 i;
	for (i=0; i<CACHE_BLOCKS; i++) {
		if ((c.ca[i].flags) && (c.ca[i].sector == sector)) {
			return 1;
		}
	}
	return 0;
}

void Erase4K (register __i0 DEVICE *ph, register u_int32 firstBlock) {
	u_int16 cmd[2];
	//printf("ERASE%ld ",firstBlock);	
	SingleCycleCommand(ph, SPI_FLASH_COMMAND_WRITE_ENABLE);
	SingleCycleCommand(ph, SPI_FLASH_COMMAND_CLEAR_ERROR_FLAGS);
	FlashUnprotect(ph);

	cmd[0] = (firstBlock>>7) | (SPI_FLASH_COMMAND_ERASE_SECTOR << 8);
	cmd[1] = (firstBlock<<9);	
	MultipleCycleCommand(ph, cmd, 4, END_FRAME);
	SpiFlashWaitStatus(ph);

}

void ProgramSector(register __i0 DEVICE *ph, register u_int32 firstBlock, u_int16 *data) {
	u_int16 cmd[2];	
	u_int16 i;
	//printf("PROGRAM%ld ",firstBlock);	
	FlashUnprotect(ph);
	cmd[0] = (firstBlock>>7) | (SPI_FLASH_COMMAND_WRITE << 8);
	cmd[1] = (firstBlock<<9); //first 256 bytes of block
	MultipleCycleCommand(ph, cmd, 4, DONT_END_FRAME);
	ph->Write(ph, data, 0, 256);
	ph->Ioctl(ph, IOCTL_END_FRAME, 0);
	SpiFlashWaitStatus(ph);
					
	FlashUnprotect(ph);
	cmd[1] |= (1 << 8); //"higher" 256 bytes of block
	MultipleCycleCommand(ph, cmd, 4, DONT_END_FRAME);
	ph->Write(ph, data, 256, 256); //Write 256 bytes from position 256 bytes
	ph->Ioctl(ph, IOCTL_END_FRAME, 0);
	SpiFlashWaitStatus(ph);
}



void Flush4K2(DEVICE *ph, u_int16 sector) {
	u_int16 i,j;	
	Erase4K(ph,sector);
	for (j=0; j<8; j++) {
		for (i=0; i<CACHE_BLOCKS; i++) {
			if (c.ca[i].flags && (c.ca[i].sector == sector+j)) {
				ProgramSector(ph,sector+j,c.ca[i].data);
				c.ca[i].flags = 0;
			}
		}
	}	
	cacheIsDirty = 0;	
}

void CacheCheck(DEVICE *ph) {
	u_int16 i,j;
	u_int16 sb;
	for (i=0; i<CACHE_BLOCKS; i++) {
		if ((c.ca[i].flags) && ((c.ca[i].sector & 0x7) == 0)) {
			//printf("consider %d ",i);
			for (j=0; j<8; j++) {
				if (!IsInCache(c.ca[i].sector + j)) goto non_continuous;
			}
			//printf("Continuous from %d. ",i);
			Flush4K2(ph, c.ca[i].sector);
			non_continuous: {}
		}
	}

}

	
void BlockRead(register __i0 DEVICE *ph, register u_int32 firstBlock, u_int16 *data) {
	u_int16 cmd[2];	
	cmd[0] = (firstBlock>>7) | (SPI_FLASH_COMMAND_READ << 8);
	cmd[1] = (firstBlock<<9);	
	//printf("R%ld ",firstBlock);
	MultipleCycleCommand(ph, cmd, 4, DONT_END_FRAME);
	ph->Read(ph, data, 0, 512);
	ph->Ioctl(ph, IOCTL_END_FRAME, 0);
}

u_int16 FFlush(DEVICE *ph) {	
	u_int16 i;
	if (cacheIsDirty) {	
		for (i=0; i<8; i++) {
			if (c.ca[i].flags == CB_NOT_PRESENT) {
				c.ca[i].sector = currentSectorInCache+i;
				BlockRead(ph,c.ca[i].sector,c.ca[i].data);
				c.ca[i].flags = CB_PRESENT;
			}
		}
	}
	CacheCheck(ph);
	cacheIsDirty = 0;
}

u_int16 PutCacheBlock(DEVICE *ph, u_int16 sector, u_int16 *data) {
	u_int16 i;
	//printf(" put(%d) ",sector);

	if (currentSectorInCache != (sector & ~7)) {
		FFlush(ph);
		currentSectorInCache = sector & ~7;
	}
	i = sector & 7;

	c.ca[i].sector = sector;
	c.ca[i].flags = CB_DIRTY;
	memcpy(c.ca[i].data,data,256);
	cacheIsDirty = CB_DIRTY;				
	CacheCheck(ph);
	return i+1;
}



ioresult DevSpiFlashBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	SPIFLASH_HWINFO *hw = &(dev->hardwareInfo);
	DEVICE *ph=hw->ph;	
	ObtainHwLocksBIP(HLB_USER_0, HLIO_NONE, HLP_NONE);
	FFlush(ph);
	//printf("SRD:%ld ",firstBlock);
	while(blocks--) {
		BlockRead(ph, firstBlock+RESERVED_SECTORS, data);
		firstBlock++;
		data += 256;
	}
	ReleaseHwLocksBIP(HLB_USER_0, HLIO_NONE, HLP_NONE);
	return S_OK;
}
ioresult DevSpiFlashBlockWrite(register __i0 DEVICE *dev,  u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	SPIFLASH_HWINFO *hw = &(dev->hardwareInfo);
	DEVICE *ph=hw->ph;	
	while(blocks--) {
		PutCacheBlock(ph, firstBlock+RESERVED_SECTORS,data);
		PrintCache();
		firstBlock++;
		data += 256;
	}
	return S_OK;
}


IOCTL_RESULT DevSpiFlashIoctl(register __i0 DEVICE *dev, s_int16 request, IOCTL_ARGUMENT arg) {
	ioresult errCode = S_OK;
	SPIFLASH_HWINFO *hw = &(dev->hardwareInfo);
	DEVICE *ph=hw->ph;	
	u_int16 i, n, cmd;
	//printf("DevSpiFlashIoCtl (%d) kutsuttu\n",request);
	switch (request) {
		case IOCTL_RESTART: {
			dev->fs = StartFileSystem(dev, "0");
			if (!dev->fs) {
				dev->flags = __MASK_PRESENT;
				return S_ERROR;
			}
			break;
		}
		case IOCTL_GET_GEOMETRY:
			if (arg) {
				((DiskGeometry*)arg)->sectorsPerBlock = 8;
				((DiskGeometry*)arg)->totalSectors = hw->totalBlocks;
			}
			return hw->totalBlocks; //obsolete
			break;
		default: {
			errCode = S_ERROR;
			break;
		}
	}
	return errCode;
}


