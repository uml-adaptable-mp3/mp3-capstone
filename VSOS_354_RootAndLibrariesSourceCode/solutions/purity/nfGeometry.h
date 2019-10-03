
#ifndef NFGEOMETRY_H
#define NFGEOMETRY_H



//#define NAND_MAX_BAD_BLOCKS XXX
// Quite typical initial bad block count is max100 per LUN (often 4096 blocks)
// Some additional reserve must be prepared for new bad blocks emerging

// size of bad block table is large-ish. Per 100 instances: 2+ (3*100) words == 302 words
// 84 is maximum size if one sector is used
// 2*84=168 elements are maximum for two sector table

// if table exceeds size of one page modifications are needed!


//#define NAND_RESERVED_AREA_START XXXX
// First block of reserved area of memory
// this depends of total block count and bad block 
// End of nand flash has bad block table stored at block NAND_RESERVED_AREA_START and NAND_RESERVED_AREA_START+1
// After bad block table are blocks used by bad block table to replace bad blocks...


//
// SELECT ONE OF FOLLOWING:
//

#if 0
// 2048 blocks, 100 initial bad blocks
#define NAND_MAX_BAD_BLOCKS 120 // Reasonable for 4096 total blocks
#define NAND_RESERVED_AREA_START 	(2048UL - 2 - NAND_MAX_BAD_BLOCKS)
#endif

#if 1
// 4096 blocks, 100 initial bad blocks
#define NAND_MAX_BAD_BLOCKS 120 // Reasonable for 4096 total blocks
#define NAND_RESERVED_AREA_START 	(4096UL - 2 - NAND_MAX_BAD_BLOCKS)
#endif

#if 0
// 8192 blocks, 200 initial bad blocks
#define NAND_MAX_BAD_BLOCKS 240 // Reasonable for 8192 total blocks
#define NAND_RESERVED_AREA_START 	(8192UL - 2 - NAND_MAX_BAD_BLOCKS)
#endif



#define NAND_FIRST_DATA_BLOCK 4 // first accessable block


#endif
