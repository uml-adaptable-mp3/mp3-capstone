#ifndef NFLAYOUT_H
#define NFLAYOUT_H

#define NF_MAX_BAD_BLOCKS 		40		// Use this many entries for the bad block table

typedef struct {
	u_int16 flags;
	u_int32 lsn;
	u_int16 ecc[5];
} StdMeta;

typedef struct {
	u_int16 from;
	u_int16 to;
} Translate16;

typedef struct {
	s_int16 from;
	s_int16 to;
} TranslateS16;

typedef struct {
	u_int16 FDFE_marker;
	u_int16 bads;
	Translate16 bad[NF_MAX_BAD_BLOCKS];
} NandFlashBadBlocks;


#endif