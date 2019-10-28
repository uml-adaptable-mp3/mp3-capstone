#ifndef K9F4G_H
#define K9F4G_H


// PHYSICAL PROPERTIES OF THE NAND FLASH 
// THESE MUST MATCH THE NAND FLASH WHICH IS INSTALLED ON BOARD
// Note that VSOS "sector" is always 512 bytes, regardless of the physical geometry of the storage. This is not a limitation.
#define NF_EBSIZE_SECTORS 		256		//Size of one eraseblock / 512 bytes. 
#define NF_EBSHIFT 				8		//2 raised to the power of this must be NF_ERASEBLOCK_SECTORS (8->256, 9->512, 10->1024,...)
#define NF_PAGESIZE_SECTORS		8		//Size of one page / 512 bytes
#define NF_TOTAL_ERASE_BLOCKS	4096

// Layout parameters - choose these carefully
#define NF_ERASEBLOCKS 			3980 	// Number of eraseblocks to use for data and report as size of disk
#define NF_JOURNAL_ERASEBLOCKS 	10 		// Number of eraseblocks on the flash to use for journaling (circular)
#define NF_FIRST_DATA_ERASEBLOCK 10		// Leave this many eraseblocks untouched from the beginning of the flash (for boot area)
#define NF_RESERVED_AREA_START 	4000	// If factory prommer is used, same number must be used in the prommer



// Helper macros - do not modify
#define NF_EBMASK (NF_EBSIZE_SECTORS-1)
#define EB(s) ((u_int16)((s)>> NF_EBSHIFT))
#define SUBSEC(s) ((u_int16)(s) & NF_EBMASK)
#define STARTSEC(e) ((u_int32)(e) << NF_EBSHIFT)


#endif

