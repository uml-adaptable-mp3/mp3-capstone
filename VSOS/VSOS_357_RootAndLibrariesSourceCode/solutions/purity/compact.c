
#include "compact.h"


u_int16 readOpen;
u_int32 readRow;
u_int16 rowToBlockShift;

#define NAND_OP_CHANGE_READ_COL 0x05
#define NAND_OP_CHANGE_READ_COL_END 0xe0


#if 1 // DO NOT CHANGE
// CHANGED from VS1005G ROM (fifoAddr) IS NOT COMPATIBLE WITH ROM VERSION
// ROM version adds 0x00 byte to start of reed solomon encode and destroys everything
void NandPutOctets(register __c0 s_int16 length, register __i2 u_int16 *buf) {
	while (length > 0) {
		register s_int16 len = length;
		if (len > 32/*512*/)
		len = 32/*512*/;
		
		NandWaitIdle();
		/* Configure */
		
		PERIP(NF_LEN) = len; //length;
		PERIP(NF_PTR) = __BYTEBUS_PERIP_MEM_BASE;
		length -= len;
		len = (len+1)/2; /**< The number of bytes not needed anymore */
		XpFifoWrite(len, __BYTEBUS_PERIP_MEM_BASE, buf);
		buf += len;
		PERIP(NF_CTRL) = 0/*NF_CTRL_WRITESEL*/ | NF_CTRL_USEPERIP | NF_CTRL_ENA;
	}
}
#else
void NandPutOctets(register __c0 s_int16 length, register __i2 u_int16 *buf);
#endif

s_int16 QueueRefresh(s_int32 row) {
	u_int16 idx;
	u_int32 *entry = nf.refreshQueue;
	
	if (nf.entriesToRefresh ==  NFRefreshQLen){
		printf("WARNING::QUEUEREFRESH::Losing refreshrequest @ row %li, queue full\n", row);
		return 1;
	}
	for(idx = 0; idx < nf.entriesToRefresh; idx++){
		if(*entry == (u_int32)row) {
			return 0;
		}
		entry++;
	}
	
	*entry = row;
	nf.entriesToRefresh++;
	return 0;
}


/*
// DEBUG ONLY
u_int16 IsItEmptyOrWhat(register u_int32 row) {
	const u_int16 column = 512;
	static u_int16 data[3];
	
	
	NAND_LOCK();
	NAND_SELECT();
	
	NandPutCommand(NAND_OP_READ_A);
	NandPutAddressOctet(column);
	NandPutAddressOctet(column>>8);
	NandPutAddressOctet((u_int16)row);
	NandPutAddressOctet((u_int16)(row>>8));
	NandPutAddressOctet((u_int16)(row>>16));
	NandPutCommand(NAND_OP_COMMIT_DATA_ADDRESS);
	NandGetOctets(6, data); // just the interesting bits
	
	//printf("Check got : 0x%04x, 0x%04x, 0x%04x\n", data[0],data[1],data[2]);
	
	NAND_RELEASE();
	NAND_UNLOCK();
	
	if(data[0] != 0xffff || data[1] != 0xffff || data[2] != 0xffff) {
		//already written
		return 0;
	}
	else {
		return 1;
	}
}// DEBUG ONLY
*/

u_int16 IsAddressBad(u_int32 row) {
	if ((row >> rowToBlockShift) >= nf.totalAccessibleEraseBlocks) {
		// Why, yes. It indeed is
		printf("FAILURE:row address 0x%lx is outside mapped memory\n", row);
		return 1;
	}
	return 0;
}


u_int32 NandReadPhysicalAndLbaMapped(register u_int32 row, register u_int16 column, register u_int16 bytes, register u_int16 *data) {
	u_int32 mappedRow = NfTrA(row, 0);
	if (IsAddressBad(row)) {
		printf("FAILURE:Bad address on read, not reading\n");
		return 0;
	}
	return NandReadPhysicalAndLba(mappedRow, column, bytes, data);
}



// if bytes == NAND_SECTOR_SIZE_BYTES == 512 entire sector is read with Reed Solomon, flags, lba and all 
u_int32 NandReadPhysicalAndLba(register u_int32 row, register u_int16 column, register u_int16 bytes, register u_int16 *data) {
	static u_int32 lba;
	static u_int16 parity[5];
	static u_int16 openParity[8];
	static u_int16 meta[SECTOR_META_SIZE_WORDS];
	u_int16 itsEmpty = 0;
	u_int16 result = 0;
	u_int16 correctedErrors = 0;
	u_int16 totalErrors = 0;
	u_int16 flags = 0;
	
	NAND_LOCK();
	NAND_SELECT();
	
	nf.lastReadRSFail = 0;
	nf.fixedErrorsInLastRead = 0;

	if (readOpen == 1 && readRow == row) {
		NandPutCommand(NAND_OP_CHANGE_READ_COL);		
		NandPutAddressOctet(column);
		NandPutAddressOctet(column>>8);
		NandPutCommand(NAND_OP_CHANGE_READ_COL_END);
	}
	else {
		NandPutCommand(NAND_OP_READ_A);
		NandPutAddressOctet(column);
		NandPutAddressOctet(column>>8);
		NandPutAddressOctet((u_int16)row);
		NandPutAddressOctet((u_int16)(row>>8));
		NandPutAddressOctet((u_int16)(row>>16));
		NandPutCommand(NAND_OP_COMMIT_DATA_ADDRESS);
		readOpen = 1;
		readRow = row;
	}

	if (bytes == NAND_SECTOR_DATA_SIZE_BYTES) {           // NF with Reed-solomon
		// RSdec-NF
		USEY(RS_CF) &= ~0xFF00;
		USEY(RS_CF) |= (RS_CF_DNF|              // NFmode ("R-S on-the-fly")
		RS_CF_DSTR | RS_CF_DENA | RS_CF_DMODE); // start+ena+10-bit
	}
	
	NandGetOctets(bytes, data); // data
	NandGetOctets(SECTOR_META_SIZE_WORDS<<1, meta); // lba, flags
	
	if (bytes == NAND_SECTOR_DATA_SIZE_BYTES) {           // NF with Reed-solomon
		USEY(RS_CF) &= ~(RS_CF_DNF | RS_CF_DENA);         // Disable RS-NF interface coupling before parity
		NandGetOctets(10, parity);
		
		if ( (parity[0] == 0xFFFF)&&(parity[1] == 0xFFFF)&&(parity[2] == 0xFFFF)&&(parity[3] == 0xFFFF)&&(parity[4] == 0xFFFF) ) {
			static const u_int16 ffParity[] = {0x159c, 0x2547, 0x5cfd, 0xc05a, 0x4c07};
			// organize parity for finalization of decode
			RsParity10Reorder(ffParity, openParity);
			itsEmpty = 1;
		}
		else {
			// organize parity for finalization of decode
			RsParity10Reorder(parity, openParity);
		}
		
		XpFifoWrite(NFpLen10, NFpAd, openParity);
		
		// Call decoder
		NFRsDecode(NFpAd, NFpLen10, 0, 1); // from address, 8symbols, no start, end
		
		// Call Fixer
		result = NFRsFix(data, meta, &correctedErrors, &totalErrors, NULL, NULL);
		
		// And the result is ....
		//
		// Well, result. 0= OK, 1= mistake by somebody, 2= too may errors
		//
		if (result == 1) {
			printf("RS-result %u, System error\n", result);
			itsEmpty = 0;
		}
		if (result == 2) {
			nf.lastReadRSFail = 1;
			printf("RS-result %u, data uncorrectable, row, 0x%lx\n", result, row);
			itsEmpty = 0;
		}
		
//		if (totalErrors > 0) {
//			printf("Read: row:%lu, col:%u, %u errors\n", row, column, totalErrors);
//		}
	
		// if errors were corrected data wear has occured and block should be refreshed at some point
		if (totalErrors >= NFRefreshTreshold) {
			printf("NOTE::High-ish errorcount in block %lu, total errors: %u \n", row, totalErrors);
			QueueRefresh(row);
		}
	}
	
	lba = (u_int32)meta[0];
	lba |= (u_int32)meta[1]<<16;
	flags = meta[3];
	nf.fixedErrorsInLastRead = totalErrors;

	NAND_RELEASE();
	NAND_UNLOCK();
	return lba;
}


ioresult NandProgramPagePhysicalMapped(register u_int32 row, register u_int16 *data) {
	if (IsAddressBad(row)) {
		printf("FAILURE:Bad address on write, not writing\n");
		return S_ERROR;
	}
	if(nf.readOnly) {
		printf("FAILURE:Read only set on write, not writing\n");
		return S_ERROR;
	}

	return NandProgramPagePhysical(NfTrA(row, 0), data);
}

ioresult NandProgramPagePhysical(register u_int32 row, register u_int16 *data) {
	u_int16 result;
	u_int16 idx = 0;
	static u_int16 sectParity[5]; // one sector of re-ordered RS parity
	u_int16 flags = 0xfffc;
	

// DEBUG ONLY
//	if(IsItEmptyOrWhat(row) != 1) {
//		printf("Row 0x%lx is trying to be written again. This will not end well\n", row);
//	}
// DEBUG ONLY
	
	readOpen = 0;
	readRow = 0;
	
	NAND_LOCK();
	NAND_SELECT();
	
	NandPutCommand(NAND_OP_PREPARE_TO_PROGRAM);
	NandPutAddressOctet(0);
	NandPutAddressOctet(0);
	NandPutAddressOctet((u_int16)row);
	NandPutAddressOctet((u_int16)(row>>8));
	NandPutAddressOctet((u_int16)(row>>16));
	
	for (idx = 0; idx < nf.sectorsPerPage; idx++) {
		// Reed Solomon is per sector
		USEY(RS_CF) &= ~(RS_CF_ENF | RS_CF_ESTR | RS_CF_EENA | RS_CF_EMODE); // Reset RS ENC (force new start)
		USEY(RS_CF) |= (RS_CF_ENF | RS_CF_ESTR | RS_CF_EENA | RS_CF_EMODE);  // NFmode+start+ena+10-bit
		
		NandPutOctets(NAND_SECTOR_DATA_SIZE_BYTES +4, data+(idx * SECTOR_SIZE_WORDS)); // data and lsn, no flags
		NandPutOctets(2, &flags);

		USEY(RS_CF) &= ~(RS_CF_ENF|RS_CF_EENA);          // Disable RS-NF
		NFReadEncParity(sectParity, NFpLen);
		NandPutOctets(NAND_SECTOR_PARITY_SIZE_BYTES, sectParity);
		
		NandWaitIdle();
	}
	
	NandPutCommand(NAND_OP_PERFORM_PROGRAM);
	
	result = NandGetStatus() & 1;
	
	NAND_RELEASE();
	NAND_UNLOCK();
	
	if (result /*|| ((row < 0x7900) && ((row&0x10)==0x10))*/) {
		s_int16 res = 0;
		printf("NOTE::MAP FAILURE, write, row 0x%lx\n", row);
		res = MapBB(row,1);
		if (res == S_ERROR) {
			return S_ERROR;
		}
	}
	
	return result;
}


ioresult NandErasePhysicalMapped(register u_int32 row) {
	
	if (IsAddressBad(row)) {
		printf("FAILURE:Bad address on erase, not erasing\n");
		return S_ERROR;
	}
	if(nf.readOnly) {
		printf("FAILURE:Read only set on erase, not erasing\n");
		return S_ERROR;
	}
	
	return NandErasePhysical(NfTrA(row, 1));
}

ioresult NandErasePhysical(register u_int32 row) {
	u_int16 result=9;
	
	readOpen = 0;
	readRow = 0;
	
	NAND_LOCK();
	NAND_SELECT();
	NandPutCommand(0x60);
	NandPutAddressOctet((u_int16)row);
	NandPutAddressOctet((u_int16)(row>>8));
	NandPutAddressOctet((u_int16)(row>>16));
	NandPutCommand(0xD0);
		
	result = NandGetStatus() & 1;
	
	NAND_RELEASE();
	NAND_UNLOCK();
		
	if (result) {
		s_int16 res = 0;
		printf("NOTE::MAP FAILURE, erase, row 0x%lx\n", row);
		res = MapBB(row,0); // no data associated with this
		if (res == S_ERROR) {
			return S_ERROR;
		}
	}
	
	return result;
}






