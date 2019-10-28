#ifndef DEV_SD_SD_H
#define DEV_SD_SD_H

#include <vsos.h>


#define SDSD_PERIP_BUFNO 0x4
#define SDSD_PERIP_ADDR (SDSD_PERIP_BUFNO*0x100) /**< Hardware buffer address, hardcoded */

#define SDSD_F_OCR31_B		15
#define SDSD_F_4BIT_B		10
#define SDSD_F_CMD_BREAK_B	 5
#define SDSD_F_EXT_CSD_B	 4
#define SDSD_F_V2_B		 3
#define SDSD_F_MMC_B		 2
#define SDSD_F_SD_B		 1
#define SDSD_F_HC_B		 0
#define SDSD_F_MULTIPLE_BLOCK_READ_B	 6
#define SDSD_F_MULTIPLE_BLOCK_WRITE_B	 7
#define SDSD_F_TRY_4BIT_B		11

#define SDSD_F_OCR31		(1<<SDSD_F_OCR31_B)	// power op done (ACMD41)
#define SDSD_F_4BIT		(1<<SDSD_F_4BIT_B)	// same as 4-bit mode bit index in SD_CF
#define SDSD_F_CMD_BREAK	(1<<SDSD_F_CMD_BREAK_B)	// same as cmdbreak bit index in SD_ST
#define SDSD_F_EXT_CSD		(1<<SDSD_F_EXT_CSD_B)	// mmccmd8 returned ok
#define SDSD_F_V2		(1<<SDSD_F_V2_B)	// cmd8 returned ok
#define SDSD_F_MMC		(1<<SDSD_F_MMC_B)	// this is mmc card
#define SDSD_F_SD		(1<<SDSD_F_SD_B)	// this is sd card
#define SDSD_F_HC		(1<<SDSD_F_HC_B)	// HC bit in SD.flags
#define SDSD_F_MULTIPLE_BLOCK_READ	(1<<SDSD_F_MULTIPLE_BLOCK_READ_B)	// Is MBR in progress?
#define SDSD_F_MULTIPLE_BLOCK_WRITE	(1<<SDSD_F_MULTIPLE_BLOCK_WRITE_B)	// Is MBW in progress?
#define SDSD_F_TRY_4BIT		(1<<SDSD_F_TRY_4BIT_B)

// Is MBW in progress?

#define SDSD_OCR_HC (1L<<30)

#define SDSD_DEF_MMCRCA  0xFACE		// TBD: DEFAULT MMC RCA
#define SDSD_ARG_CMD8    ((1<<8) | (0xAA))	// 2.7-3.3V, check pattern
#define SDSD_ARG_NONE    0x00000000L	// Argument for commands with Stuff/Dont care
#define SDSD_ARG_OCR_2V8 (1L<<15)	// 17: 2.7V - 2.8V, arg for OCR, ACMD41
#define SDSD_ARG_OCR_2V9 (1L<<16)	// 17: 2.8V - 2.9V, arg for OCR, ACMD41
#define SDSD_ARG_OCR_3V0 (1L<<17)	// 17: 2.9V - 3.0V, arg for OCR, ACMD41
#define SDSD_ARG_OCR_3V1 (1L<<18)	// 18: 3.0V - 3.1V, arg for OCR, ACMD41
#define SDSD_ARG_OCR_3V2 (1L<<19)	// 19: 3.1V - 3.2V, arg for OCR, ACMD41
#define SDSD_ARG_OCR_3V3 (1L<<20)	// 20: 3.2V - 3.3V, arg for OCR, ACMD41
#define SDSD_ARG_OCR_3V4 (1L<<21)	// 20: 3.3V - 3.4V, arg for OCR, ACMD41
#define SDSD_ARG_OCR_3V5 (1L<<22)	// 20: 3.4V - 3.5V, arg for OCR, ACMD41
#define SDSD_ARG_OCR_3V6 (1L<<23)	// 23: 3.5V - 3.6V, arg for OCR, ACMD41
//                       0x00ff8000L	// Typical response to OCR (ACMD41)


#ifndef ASM

typedef struct DevSdSdHardwareInfo {
  u_int16 flags;
  u_int16 rca;
  u_int32 size;
  u_int32 nextBlock;
} SD_HWINFO;

ioresult DevSdSdCreate(register __i0 DEVICE *dev, const void *name, u_int16 extraInfo);
ioresult DevSdSdInput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
ioresult DevSdSdOutput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
ioresult DevSdSdBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevSdSdBlockWrite(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevSdSdDelete(register __i0 DEVICE *dev);
const char* DevSdSdIdentify(register __i0 void *dev, char *buf, u_int16 bufsize);
IOCTL_RESULT DevSdSdIoctl(register __i0 DEVICE *dev, s_int16 request, IOCTL_ARGUMENT arg);

typedef struct SDDevice {
	vo_flags   flags; ///< VSOS Flags
	const char*    (*Identify)(register __i0 void *self, char *buf, u_int16 bufsize); ///< Return some kind of decorative name for this object.
	ioresult (*Create)(register __i0 DEVICE *self, const void *name, u_int16 extraInfo); ///< Start device, populate descriptor, find and start filesystem.
	ioresult (*Delete)(register __i0 DEVICE *self); ///< Flush, Stop, Clean and Free the descriptor of the device
	IOCTL_RESULT (*Ioctl)(register __i0 DEVICE *self, s_int16 request, IOCTL_ARGUMENT arg); ///< Reset, Start, Stop, Restart, Flush, Check Media etc
	u_int16  (*Read) (register __i0 DEVICE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	u_int16  (*Write)(register __i0 DEVICE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
	ioresult (*BlockRead)(register __i0 DEVICE *self, u_int32 firstBlock, u_int16 blocks, u_int16 *data); ///< Read block at LBA
	ioresult (*BlockWrite)(register __i0 DEVICE *self, u_int32 firstBlock, u_int16 blocks, u_int16 *data); ///< Write block at LBA
	FILESYSTEM *fs; ///< pointer to the filesystem driver for this device
	u_int16  deviceInstance; ///< identifier to detect change of SD card etc
        SD_HWINFO hwInfo; // must be 6 words!!!!!!!
	u_int16  deviceInfo[16]; ///< Filesystem driver's info of this device
} SD_DEVICE;

extern const SD_DEVICE __mem_y devSdSdDefaults;

#endif /* !ASM */

#endif /* !DEV_SD_SD_H */
