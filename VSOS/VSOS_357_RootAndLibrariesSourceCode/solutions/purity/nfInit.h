#ifndef NF_INIT_H
#define NF_INIT_H

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <vstypes.h>
#include <vsos.h>
#include <vsNand.h>
#include <timers.h>
#include <hwlocks.h>
#include <swap.h>
#include <itype.h>
#include <string.h>


#include "compact.h"




#define MAGIC_FDFE 0xfdfe // used to detect bad block table

struct NandFlashBadBlocks2 {
	u_int16 FDFE_marker;
	u_int16 bads;
	u_int16 from[NAND_MAX_BAD_BLOCKS];
	u_int16 to[NAND_MAX_BAD_BLOCKS];
	u_int16 pageOffset[NAND_MAX_BAD_BLOCKS];
};

__mem_y struct NandFlashBadBlocks2 BBTable;



#define ISPRINT(a) (isprint(a) ? (a) : ('.'))
void PrintBuffer2(u_int16 *data, s_int16 words);
struct NandFlashDescriptor *CreateMlcFlash (struct NandFlashDescriptor *self);
void CreateBBTable();
void SaveBBTable(void);

s_int16 MapBB(u_int32 row, u_int16 hasData);
u_int32 NfTrA(u_int32 row, u_int16 isErase);

#endif
