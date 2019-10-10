//
//
// Functions for use of Reed Solomon codec
//
//

#include"rsFunc.h"


// Reads decoding results and makes corrections if needed (and possible)
// Returns:
// 0 : Success
// 1 : Syndrome failure, check inputs
// 2 : Decoder failure, too many errors to correct
//
// Only for NandFlash interface, data length is tied to 256 words and is followed by 3 words if meta
u_int16 NFRsFix(u_int16 *data, u_int16 *meta, u_int16 *corrected, u_int16 *totalErrors, u_int16 *locations, u_int16 *magnitudes) {
	
	u_int16 idx = 0;
	u_int16 numCorrections = 0;
	
	*corrected = 0;
	*totalErrors = 0;
	
	// Wait for decoding to end
	// wait for syndrome calculation, if needed:
	while (!(USEY(RS_ST) & RS_ST_DOK)
			&& !(USEY(RS_ST) & RS_ST_DERR)
			&& !(USEY(RS_ST) & RS_ST_DFAIL)
			&& !(USEY(RS_ST) & RS_ST_DFFAIL)) {
		// Check that there is actual decode ongoing to avoid getting stuck
		if (!(USEY(RS_CF) & RS_CF_DENA) && !(USEY(RS_ST) & RS_ST_DFBUSY)) {
			printf("RS:ERROR: waiting non-existing decode to end\n");
			return 1;
		}
	}
	
	// Check syndrome status:
	// return 0, all ok, no errors:
	if (USEY(RS_ST) & RS_ST_DOK) {
		*corrected = 0;
		*totalErrors = 0;
		//reset
		USEY(XP_ST) = XP_ST_RSDEC_RDY_INT | XP_ST_RSEC_RDY_INT;
		return 0;
	}
	// errors found, wait for decoder to complete
	else if (USEY(RS_ST) & RS_ST_DERR) {
		// Wait decoder results
		while (!(USEY(RS_ST) & RS_ST_DFRDY2)){
			;
		}
		
		// Check for lack of success:
		if (USEY(RS_ST) & RS_ST_DFFAIL) {
			// fail with too many errors
			//printf("RS::Too many errors to fix\n");
			*corrected = 0;
			*totalErrors = 0;
			
			//reset
			USEY(XP_ST) = XP_ST_RSDEC_RDY_INT | XP_ST_RSEC_RDY_INT;
			return 2;
		}
		else {
			// Read info
			// total error number
			USEY(XP_CF) &= 0x0FFF; // clear previous
			USEY(XP_CF) |= 0xb000; // select total errors
			*totalErrors = USEY(RS_DATA);
			
			// Correctable errors
			numCorrections = (USEY(RS_ST) >> RS_ST_DNERR_B) & 0x001f;
			*corrected = numCorrections;
			
			// Do the corrections
			// This can still be zero if errors are in parity
			if (numCorrections > 0) {
				USEY(XP_ADDR) = USEY(RS_DPTR);
				USEY(XP_CF) &= ~XP_CF_WRBUF_ENA;
				USEY(XP_CF) |= XP_CF_RDBUF_ENA;
				
				USEY(XP_IDATA);// dummy
				USEY(XP_IDATA);// dummy
			}
			
			for (idx = 0; idx < numCorrections; idx++) {
				u_int16 index = 0;
				u_int16 magnitude = 0;
				
				// Get index:
				index = USEY(XP_IDATA);
				
				// Get magnitude
				magnitude = USEY(XP_IDATA);
				
				//printf("i:%u,m:0x%04x ", index, magnitude);

				// fix
				if (index % 2 == 0) {
					// msB
					if (index < NAND_SECTOR_DATA_SIZE_BYTES) {
						*(data + ((index>>1) & 0x7fff)) =
						*(data + ((index>>1) & 0x7fff)) ^ ((magnitude<<8) & 0xff00);
					}
					else {
						*(meta + (((index - NAND_SECTOR_DATA_SIZE_BYTES)>>1) & 0x7fff)) =
						*(meta + (((index - NAND_SECTOR_DATA_SIZE_BYTES)>>1) & 0x7fff)) ^ ((magnitude<<8) & 0xff00);
					}
				}
				else {
					// lsB
					if (index < NAND_SECTOR_DATA_SIZE_BYTES) {
						*(data + ((index>>1) & 0x7fff)) =
						*(data + ((index>>1) & 0x7fff)) ^ (magnitude & 0x00ff);
					}
					else {
						*(meta + (((index - NAND_SECTOR_DATA_SIZE_BYTES)>>1) & 0x7fff)) =
						*(meta + (((index - NAND_SECTOR_DATA_SIZE_BYTES)>>1) & 0x7fff)) ^ (magnitude & 0x00ff);
					}
				}
				
				// store index and magnitude for further use
				if (locations != NULL && magnitudes != NULL) {
					locations[idx] = index;
					magnitudes[idx] = magnitude;
				}
				
			}
			// reset and return
			USEY(XP_ST) = XP_ST_RSDEC_RDY_INT | XP_ST_RSEC_RDY_INT;
			USEY(XP_CF) &= ~XP_CF_RDBUF_ENA;;
			return 0;
		}
	}
	// Syndrome calculator failed, propable cause user error
	else if (USEY(RS_ST) & RS_ST_DFAIL){
		*corrected = 0;
		*totalErrors = 0;
		//printf("RS_ST_DFAIL\n");
		//reset
		USEY(XP_ST) = XP_ST_RSDEC_RDY_INT | XP_ST_RSEC_RDY_INT;
		
		return 1;
	}
	else {
		printf("RS:UNEXPECTED: status: 0x%04x\n", USEY(RS_ST));
		//reset
		USEY(XP_ST) = XP_ST_RSDEC_RDY_INT | XP_ST_RSEC_RDY_INT;
		return 1;
	}
}



// Reads parity from encoder
void NFReadEncParity(u_int16 *data, u_int16 bytes) {
	u_int16 idx = 0;
	
	// Nand flash mode does not generate RS interrupts:
	while ((USEY(RS_CF) & RS_CF_EENA)) {
		;
	}
	USEY(RS_CF) &= ~(RS_CF_EENA);          // Disable RS-NF
	
	USEY(RS_ELEN) = 0;
	USEY(RS_EPTR) = 0; // select encoder parity to output
	
	USEY(XP_CF) &= ~0xf000; // select encoder result parity
	
	for (idx = 0; idx < (bytes>>1 & 0x7fff); idx++) {
		USEY(RS_CF) &= ~0x00f0; // start from zero
		USEY(RS_CF) |= (idx << 4) & 0x00f0; // set index
		
		*(data + idx) = USEY(RS_DATA);
	}
	
	if (bytes % 2 != 0) {
		// one more
		*(data + (bytes>>1)) = USEY(RS_DATA);
	}
	return;
}



void NFRsDecode(u_int16 wAddr, u_int16 bytes, u_int16 start, u_int16 endLast) {

	// make sure that decoder is free.
	if (start == 1) {
		while (!(USEY(RS_ST) & RS_ST_DFRDY1)
				|| (USEY(RS_ST) & RS_ST_DFBUSY)) {
			;
		}
		USEY(RS_CF) = 0;
		USEY(XP_ST) = XP_ST_RSDEC_RDY_INT | XP_ST_RSENC_RDY_INT | XP_ST_RSEC_RDY_INT; 
	}
	
	USEY(RS_DPTR) = wAddr;  // run from
	USEY(RS_DLEN) = bytes;  // run through decode (message)
	
	if (start) {
		USEY(RS_CF) =
		RS_CF_DSTR | // decoder start
		RS_CF_DMODE; // set when decoding 10bit code (nand flash)
	}
	
	if (endLast == 1) {
		// Write 10bit parity information correctly:
		USEY(RS_CF) |= RS_CF_D10B;
	}
	USEY(RS_CF) |= RS_CF_DENA; // run
		
	if (endLast) {
		volatile u_int16 stat;
		stat = USEY(RS_CF) & RS_CF_DENA;

		// wait until all data is in decoder before starting correcting
		while (stat) {
			stat = USEY(RS_CF) & RS_CF_DENA;
		}
		// End codeword, this starts data repair if deemed necessary
		USEY(RS_CF) |= RS_CF_DEND;
	}
	return;
}


// Reordering for 10bit parity from 8bit mode in 16 bit words(memory interface)
// to 10bit mode(decoder input)
void RsParity10Reorder(u_int16 *source_pntr, u_int16 *target_pntr) {
	
	*(target_pntr +0) = ((*(source_pntr +0) & 0xff00) >> 8)
		| ((*(source_pntr +0) & 0x0003) << 8); //0

	*(target_pntr +1) = ((*(source_pntr +0) & 0x00fc) >> 2)
		| ((*(source_pntr +1) & 0x0f00) >> 2); //1
	
	*(target_pntr +2) = ((*(source_pntr +1) & 0xf000) >> 12)
		| ((*(source_pntr +1) & 0x003f) << 4); //2

	*(target_pntr +3) = ((*(source_pntr +1) & 0x00c0) >> 6)
		| ((*(source_pntr +2) & 0xff00) >> 6); //3
	
	*(target_pntr +4) = ((*(source_pntr +2) & 0x00ff))
		| ((*(source_pntr +3) & 0x0300)); //4
	
	*(target_pntr +5) = ((*(source_pntr +3) & 0xfc00) >> 10)
		| ((*(source_pntr +3) & 0x000f) << 6); //5
	
	*(target_pntr +6) = ((*(source_pntr +3) & 0x00f0) >> 4)
		| ((*(source_pntr +4) & 0x3f00) >> 4); //6
	
	*(target_pntr +7) = ((*(source_pntr +4) & 0xc000) >> 14)
		| ((*(source_pntr +4) & 0x00ff) << 2); //7
	
    return;
}
