#ifndef DEVSPIFLASH_H
#define DEVSPIFLASH_H

typedef struct devSpiFlashHardwareInfoStruct {
	DEVICE *ph;
} devSpiFlashHwInfo;

ioresult DevSpiFlashCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo);
ioresult DevSpiFlashInput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
ioresult DevSpiFlashOutput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);	
ioresult DevSpiFlashBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevSpiFlashBlockWrite(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevSpiFlashDelete(register __i0 DEVICE *dev);
char* DevSpiFlashIdentify(register __i0 void *obj, char *buf, u_int16 bufsize);

#define SPI_WREN  0x06
#define SPI_WRDI  0x04
#define SPI_RDSR  0x05
#define SPI_WRSR  0x01
#define SPI_READ  0x03
#define SPI_WRITE 0x02
#define SPI_ERSE  0xD8


#endif
