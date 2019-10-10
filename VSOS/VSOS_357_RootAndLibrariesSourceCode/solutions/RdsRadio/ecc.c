
#include "ecc.h"


#define DBG 0

#if 0
u_int16 G[] = {
    0x1B9, /* 0110111001 */
    0x372, /* 1101110010 */
    0x35D, /* 1101011101 */
    0x303, /* 1100000011 */
    0x3BF, /* 1110111111 */
    0x2C7, /* 1011000111 */
    0x037, /* 0000110111 */
    0x06E, /* 0001101110 */
    0x0DC, /* 0011011100 */
    0x1B8, /* 0110111000 */
    0x370, /* 1101110000 */
    0x359, /* 1101011001 */
    0x30B, /* 1100001011 */
    0x3AF, /* 1110101111 */
    0x2E7, /* 1011100111 */
    0x077  /* 0001110111 */
};
#endif




u_int16 CheckSyndrome(register u_int16 *data, register u_int16 check, register u_int16 offset) {
	u_int16 syndrome = 0;
	u_int16 idx = 0;
	u_int16 input = *data;
	
	// MSb first for data!
	// syndrome index 9 is in LEFTmost bit, ie. shifts left, input to [6]
	
	for (idx = 0; idx < 16; idx++) {
		u_int16 fb = input ^ syndrome;
		
		//plain shift syndrome [0] (6) goes(remains) zero
		syndrome <<= 1; // msb out
		if ((fb & 0x8000U) != 0) {
			// add generator
			syndrome ^= 0x6e40;
		}
		input <<= 1;
	}
	
	syndrome ^= (offset<<6);
	
	if (syndrome == (check<< 6)) {
		return 0;
	}
	else {
		return 1;
	}
}


#if 0
void InitCrc(register struct rdsCrc *rdsCrc) {
	u_int16 i,j;
	rdsCrc->nextBlock = 0;
	memset(rdsCrc->correctTab, 0, sizeof(rdsCrc->correctTab));
	/* correct 1 error */
	for (i=0;i<16;i++) {
		/* correct 2 errors */
		for (j=0;j<16;j++) {
			/* correct 3 errors */
			int k;
			for (k=0;k<16;k++) {
				rdsCrc->correctTab[G[i]^G[j]^G[k]] = (1<<i)^(1<<j)^(1<<k);
			}
			rdsCrc->correctTab[G[i]^G[j]] = (1<<i)^(1<<j);
		}
		rdsCrc->correctTab[G[i]] = (1<<i);
	}
}
#endif



#if 0
u_int16 RdsCrc(register register u_int16 wData) {
	register int i;
	register u_int16 wCrc = 0;
	register u_int16 *gp = &G[15];
	for (i=0; i<16; i++) {
		if ((wData & 0x8000U))
		wCrc ^= *gp;
		gp--;
		wData <<= 1;
	}
	return wCrc;
}

u_int16 FixBlock(register struct rdsBlock *rdsBlock, register struct rdsCrc *rdsCrc) {
	static const u_int16 O[4] = {OFFSET_A, OFFSET_B, OFFSET_C, OFFSET_D};
	/* invalid -- perform error correction */
	rdsBlock->data ^= rdsCrc->correctTab[rdsBlock->crc^O[rdsCrc->nextBlock]];
	if (RdsCrc(rdsBlock->data) == (rdsBlock->crc^O[rdsCrc->nextBlock])) {
		PERIP(RDS_FORCESYNC) = 1; // keep sync..
		rdsBlock->type ^= 4; /* make it ok */
		PERIP(RDS_FORCESYNC) = 0;
	}
	
	switch (rdsBlock->type) {
		case 0:
		rdsCrc->nextBlock = 1;
		break;
		case 1:
		rdsCrc->nextBlock = 2;
		break;
		case 2:
		rdsCrc->nextBlock = 3;
		break;
		case 3:
		rdsCrc->nextBlock = 0;
		break;
	}
	return 0;
}
#endif

#if 0
u_int16 RdsBlockFix (register u_int16 *data, register u_int16 check, register u_int16 offset) {
	u_int16 syndrome[10];
	u_int16 idx = 0;
	u_int16 input = *data;
	u_int16 result = *data;
	u_int16 par = check ^ offset;
	u_int16 fixes = 0;
	
	// first and current error locations
	u_int16 initial = 0;
	u_int16 current = 0;
	
	memset(syndrome, 0, sizeof(syndrome));
	
	// insert data to syndrome register
	for (idx = 0; idx < 16; idx++) {
		u_int16 lb = syndrome[9]; // loopback
		u_int16 bin = input >> 15; // bit in
		
		syndrome[9] = syndrome[8] ^ bin;
		syndrome[8] = syndrome[7] ^ lb ^ bin;
		syndrome[7] = syndrome[6] ^ lb;
		syndrome[6] = syndrome[5];
		syndrome[5] = syndrome[4] ^ lb;
		syndrome[4] = syndrome[3] ^ lb ^ bin;
		syndrome[3] = syndrome[2] ^ lb ^ bin;
		syndrome[2] = syndrome[1];
		syndrome[1] = syndrome[0] ^ bin;
		syndrome[0] = lb ^ bin;
		
		input <<= 1;
	}
	
	// insert check-bits to syndrome register
	for (idx = 0; idx < 10; idx++) {
		u_int16 lb = syndrome[9]; // loopback
		u_int16 bin = (par >> 9) & 0x0001; // bit in
		
		syndrome[9] = syndrome[8] ^ bin;
		syndrome[8] = syndrome[7] ^ lb ^ bin;
		syndrome[7] = syndrome[6] ^ lb;
		syndrome[6] = syndrome[5];
		syndrome[5] = syndrome[4] ^ lb;
		syndrome[4] = syndrome[3] ^ lb ^ bin;
		syndrome[3] = syndrome[2] ^ lb ^ bin;
		syndrome[2] = syndrome[1];
		syndrome[1] = syndrome[0] ^ bin;
		syndrome[0] = lb ^ bin;
		
		par <<= 1;
	}
	
	// fixes
	for (idx = 0; idx < 26; idx++) {
		u_int16 lb = syndrome[9]; // loopback
		
		if (syndrome[0] == 0 && syndrome[1] == 0 && syndrome[2] == 0
		&& syndrome[3] == 0 && syndrome[4] == 0 && syndrome [9] == 1){
			u_int16 iii = 0;
	    	    
			for (iii = 0; iii < 5; iii++) {
				u_int16 fix;
				
				if ((idx+iii) > 15) {
					// past the data?
					if (syndrome[9-iii] != 0) {
						// uncorrectable
						fixes = 0xfffe;
						break;
					}
					else {
						// no problem yet
						continue;
					}
				}
				
				fix = syndrome[9-iii] << (15- (idx+iii));
				if (fix != 0) {
					if (fixes == 0) {
						if (initial != 0) {
							if(DBG)puts("ERROR: Cant find first error!");
						}
						initial = idx+iii;
					}
					else {
						if (((idx+iii) -initial) >= 5) {
							// Too many errors
							if(DBG)printf("INITIAL: %u, current %u\n",initial, (idx+iii));
							fixes = 0xffff;
							break;
						}
					}
					fixes++;
				}
				
				result ^= fix;
			}
			
			//no more than one error burst
			break;
			
		}
		syndrome[9] = syndrome[8];
		syndrome[8] = syndrome[7] ^ lb;
		syndrome[7] = syndrome[6] ^ lb;
		syndrome[6] = syndrome[5];
		syndrome[5] = syndrome[4] ^ lb;
		syndrome[4] = syndrome[3] ^ lb;
		syndrome[3] = syndrome[2] ^ lb;
		syndrome[2] = syndrome[1];
		syndrome[1] = syndrome[0];
		syndrome[0] = lb;
	}
	
	// check for decode failure; too many errors
	if (fixes == 0) {
		for (idx = 0; idx < 5; idx++) {
			if (syndrome[idx] != 0) {
				// uncorrectable error
				fixes = 0xffff;
				break;
			}
		}
	}
	else {
		*data = result;
	}
	
	return fixes;
}
#endif
