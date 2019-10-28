#ifndef MEMTRACK_H
#define MEMTRACK_H

#include <vstypes.h>
#include <aucommon.h>

#ifndef ASM

auto void SwapMemTrackVectors(void);

#define MEM_TRACK_LIST_SIZE 128

struct MemTrackElement {
  u_int16 addr;		/* Address */
  u_int16 sizeAndY;	/* Bit 15: 0=X, 1=Y, bits 14:0=size */
  u_int16 owner;	/* Pointer to the owner */
};

extern struct MemTrackElement __mem_y memTrack[MEM_TRACK_LIST_SIZE];

#endif /* !ASM */

#endif /* !AUXI2SM_H */
