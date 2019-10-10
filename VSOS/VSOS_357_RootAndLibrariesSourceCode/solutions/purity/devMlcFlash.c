#include "devMlcFlash.h"
#include "compact.h"
#include "unimap.h"
#include <string.h>
#include <vo_fat.h>
#include <mutex.h>

void MlcErase(u_int32 sba) {
	//nf.Erase(sba >> pageSizeShift);
}
ioresult DevMlcFlashCreate(register __i0 DEVICE *dev, void *name, u_int16 extraInfo) {
	u_int16 i;
	//u_int32 realDataStart;

	FatDeviceInfo *di=(void*)&devMlcFlash.deviceInfo;

	dev->deviceInstance = __nextDeviceInstance++;
	dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;
	
	LoadState();

	devMlcFlash.fs = StartFileSystem(&devMlcFlash,"");
	//realDataStart = di->dataStart + (u_int32)(di->fatSectorsPerCluster * 2);
	if (devMlcFlash.fs)UpdateFatBorders();
	//x// printf("FAT RealDataStart: %lx\n", realDataStart);

	return S_OK;
}



IOCTL_RESULT DevMlcFlashIoctl(register __i0 DEVICE *dev, s_int16 request, IOCTL_ARGUMENT arg) {
	ioresult errCode = S_OK;
	DevMlcFlashHwInfo *hw = (void*)&(dev->hardwareInfo);
	switch (request) {
		case IOCTL_RESTART: {
			//x// printf("RESTART");
			dev->fs = StartFileSystem(dev, "0");
			if (!dev->fs) {
				//dev->flags = __MASK_PRESENT | __MASK_SEEKABLE |__MASK_READABLE | __MASK_WRITABLE | __MASK_OVERLAPPED | __MASK_OPEN;
				//dev->flags = __MASK_PRESENT | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE | __MASK_OVERLAPPED;
				errCode = S_ERROR;
			} else {
				//dev->flags = __MASK_PRESENT | __MASK_SEEKABLE |__MASK_READABLE | __MASK_WRITABLE | __MASK_OVERLAPPED | __MASK_OPEN;
			}
			break;
		}
		case IOCTL_GET_GEOMETRY:
			if (arg) {
				//((DiskGeometry*)arg)->sectorsPerBlock = nf.sectorsPerPage;
				((DiskGeometry*)arg)->sectorsPerBlock = 1;
				/* This should prevent filling the flash through filesystem 
				 * Usable sectors = total - start - wrap reservation - 2 * deshard area - constant.
				 */
				((DiskGeometry*)arg)->totalSectors = pur.usableSectors - 
					PURITY_SURFACE_START -
					(4 * PURITY_DESHARDING_BLOCKS * nf.sectorsPerEraseBlock) - 32768;
				//((DiskGeometry*)arg)->totalSectors = nf.totalSectors-32768;
				//((DiskGeometry*)arg)->totalSectors = 64;
			}
			return S_OK;
			break;
		default: {
			errCode = S_ERROR;
			break;
		}
	}
	return errCode;
}

const char *DevMlcFlashIdentify(register __i0 void *dev, char *buf, u_int16 bufsize) {
	return "MLC Flash";
}


ioresult DevMlcFlashBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	ObtainHwLocksBIP(DESHARDER_LOCK, 0,0);
	while(blocks--) {
		ReadLba(firstBlock, data);
		data += 256;
		firstBlock++;
	}
	ReleaseHwLocksBIP(DESHARDER_LOCK, 0,0);
	return S_OK;
}


ioresult DevMlcFlashBlockWrite(register __i0 DEVICE *dev,  u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	ObtainHwLocksBIP(DESHARDER_LOCK, 0,0);
	while(blocks--) {
		StoreLba(firstBlock, data);
		firstBlock++;
		data += 256;
	}
	ReleaseHwLocksBIP(DESHARDER_LOCK, 0,0);
	return S_OK;
}
#pragma msg 30 off
DEVICE devMlcFlash = {
  __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE, // u_int16 flags; //< present, block/char
  DevMlcFlashIdentify, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);
  DevMlcFlashCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
  CommonOkResultFunction,//DevMlcFlashDelete, //ioresult (*Delete)(DEVICE *dev);
  DevMlcFlashIoctl, //ioresult (*Ioctl)(DEVICE *dev, s_int16 request, s_int32 arg); //Start, Stop, Flush, Check Media
  // Stream operations
  0, //u_int16  (*Read) (register __i0 DEVICE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
  0, //u_int16  (*Write)(register __i0 DEVICE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
  // Block device operations
  DevMlcFlashBlockRead, //ioresult (*BlockRead)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  DevMlcFlashBlockWrite, //ioresult (*BlockWrite)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
};
#pragma msg 30 on
