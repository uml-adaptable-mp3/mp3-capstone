
#ifndef DEV_SPIFLASH_H
#define DEV_SPIFLASH_H

#include <vsos.h>
#ifndef ASM

typedef struct DevSpiFlashHardwareInfo {
  DEVICE *ph; // BUS device: the SPI bus where the SPI flash is connected to
  void *currentWriteBuffer; //4K
  u_int16 currentWriteBlock; //First block of 4K
  u_int16 reservedSectors;
  u_int16 totalBlocks;
} SPIFLASH_HWINFO;

ioresult DevSpiFlashCreate(register __i0 DEVICE *dev, const void *name, u_int16 extraInfo);
ioresult DevSpiFlashInput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
ioresult DevSpiFlashOutput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
ioresult DevSpiFlashBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevSpiFlashBlockWrite(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevSpiFlashDelete(register __i0 DEVICE *dev);
const char* DevSpiFlashIdentify(register __i0 void *dev, char *buf, u_int16 bufsize);
IOCTL_RESULT DevSpiFlashIoctl(register __i0 DEVICE *dev, s_int16 request, IOCTL_ARGUMENT arg);

typedef struct CacheBlockStruct {
	u_int16 flags;
	u_int16 sector;
	u_int16 data[256];
} CacheBlock;
#define CB_NOT_PRESENT 0
#define CB_PRESENT 1
#define CB_DIRTY 2

#define RESERVED_SECTORS (2*65536/512) // Reserve 128KB from the beginning of flash for boot code
extern DEVICE spi;
extern DEVICE spiFlash;


#endif /* !ASM */

#endif /* !DEV_SD_SD_H */
