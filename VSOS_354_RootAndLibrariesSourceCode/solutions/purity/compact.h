

#ifndef COMPACT_H
#define COMPACT_H

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <vstypes.h>
#include <vsos.h>
#include <vsNand.h>
#include <timers.h>
#include <hwlocks.h>

#include "nfGeometry.h"

#include "nfInit.h"
#include "rsFunc.h"



// How many correctable errors per sector generate refresh request for affected block
#define NFRefreshTreshold 3

//#define HLB_BYTEBUS HLB_5
#define HLB_BYTEBUS HLB_5_B
#define __BYTEBUS_PERIP_MEM_BASE (0x0100*HLB_BYTEBUS)
#define NAND_SECTORS_PER_PAGE 8

#define NAND_SECTOR_NO_PARITY_SIZE_BYTES 518 // 512 bytes data, 4bytes lsn, 2bytes flags,
#define NAND_SECTOR_DATA_SIZE_BYTES 512 // just data
#define NAND_SECTOR_PARITY_SIZE_BYTES 10
#define HLB_NF_BUFFERS (HLB_5)

#define SECTOR_SIZE_BYTES 528 // 512 bytes data, 4bytes lsn, 2bytes flags, 10bytes ReedSolomon
#define SECTOR_SIZE_WORDS (SECTOR_SIZE_BYTES/2)

#define SECTOR_META_SIZE_WORDS ((4+2)>>1) // 4 bytes lba, 2 bytes flags




#define NAND_PAGE_AND_SPARE_SIZE_BYTES 4314 //Maximum
#define NAND_PAGE_AND_SPARE_SIZE_WORDS ((NAND_PAGE_AND_SPARE_SIZE_BYTES+1)/2)




#define NAND_SELECT() PERIP(GPIO0_CLEAR_MASK) = GPIO0_CS1
#define NAND_RELEASE() PERIP(GPIO0_SET_MASK) = GPIO0_CS1


#define NAND_LOCK() ObtainHwLocksBIP(HLB_NF_BUFFERS, HLIO_0_14|HLIO_0_15|HLIO_NF|HLIO_CS_SELECT, HLP_RSEN|HLP_NAND|HLP_XPERIP_IF);
#define NAND_UNLOCK() ReleaseHwLocksBIP(HLB_NF_BUFFERS, HLIO_0_14|HLIO_0_15|HLIO_NF|HLIO_CS_SELECT, HLP_RSEN|HLP_NAND|HLP_XPERIP_IF);

#define NFppaddr   0x200     // perip mem: 0x200-0x400-1 => 512W, 1024B max
#define NFdLen   512                    // data bytes, use even number, <1023
#define NFsLen   256                    // entire spare bytes (max 16*16bytes)
#define NFmLen   6                      // mapper bytes, use even number
#define NFpLen   10   //DO NOT EDIT//   // R-S NF parity as byte
#define NFpLen10 8    //DO NOT EDIT//   // R-S NF parity symbols, 8x10-bit
#define NFdAd    NFppaddr               // start addr of payload data
#define NFmAd    (NFdAd+NFdLen/2)       // start addr of mapper data
#define NFpAd    (NFmAd+NFmLen/2)       // start addr of parity symbols

#define NFRefreshQLen 8

struct NandFlashDescriptor {
	u_int16 pageSizeBytes;
	u_int16 spareSizeBytes;
	u_int16 sectorsPerPage;
	u_int16 pagesPerEraseBlock;
	u_int16 sectorsPerEraseBlock;
	u_int16 eraseBlocks;
	u_int16 entriesToRefresh;
	u_int16 totalAccessibleEraseBlocks;
	u_int16 readOnly;
	u_int16 lastReadRSFail;
	u_int16 fixedErrorsInLastRead;
	u_int32 totalSectors;
	u_int16 ident[5];
	u_int32 refreshQueue[NFRefreshQLen];
		
	struct NandFlashDescriptor *(*Create)(struct NandFlashDescriptor *self);
	u_int32 (*Read)(register u_int32 row, register u_int16 column, register u_int16 bytes, register u_int16 *data);
	ioresult (*ProgramPage)(register u_int32 row, register u_int16 *data);
   	ioresult (*Erase)(register u_int32 row);
};

struct NandFlashDescriptor nf;


s_int16 QueueRefresh(s_int32 row);

u_int32 NandReadPhysicalAndLbaMapped(register u_int32 row, register u_int16 column, register u_int16 bytes, register u_int16 *data);
ioresult NandProgramPagePhysicalMapped(register u_int32 row, register u_int16 *data);
ioresult NandErasePhysicalMapped(register u_int32 row);

u_int32 NandReadPhysicalAndLba(register u_int32 row, register u_int16 column, register u_int16 bytes, register u_int16 *data);
ioresult NandProgramPagePhysical(register u_int32 row, register u_int16 *data);
ioresult NandErasePhysical(register u_int32 row);


#endif
