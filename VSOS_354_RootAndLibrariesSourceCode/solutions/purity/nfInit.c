
#include "nfInit.h"

//__mem_y NandFlashBadBlocks2 *BBTable;
//NandFlashBadBlocks2 *BBTable;

extern u_int16 pageBuffer[NAND_PAGE_AND_SPARE_SIZE_WORDS];

extern __mem_y u_int16 oldExtClock4KHz;
extern __mem_y u_int16 oldClockX;
extern struct FsNandPhys fsNandPhys;

extern u_int16 readOpen;
extern u_int32 readRow;

extern u_int16 rowToBlockShift;

u_int16 holdBBTableSave;

void NandGetIdent(u_int16 address){
  NandDeselect();
  NandSelect();
  NandPutCommand(NAND_OP_READ_SIGNATURE);
  NandPutAddressOctet(address);
  NandGetOctets(10,nf.ident);
  NandDeselect();
}
#define NAND_OP_GET_FEATURE 0xEE
#define NAND_OP_SET_FEATURE 0xEF

void NandGetFeature(u_int16 address, u_int16 *feature) {
        NandDeselect();
        NandSelect();
        NandPutCommand(NAND_OP_GET_FEATURE);
        NandPutAddressOctet(address);
        NandGetOctets(4,feature);
        NandDeselect();
}

void NandSetFeature(u_int16 address, u_int16 *feature) {
        NandDeselect();
        NandSelect();
        NandPutCommand(NAND_OP_SET_FEATURE);
        NandPutAddressOctet(address);
        NandPutOctets(4,feature);
        printf("Feature set\n");
        NandDeselect();
}
#if 1
void NandInit() {
        u_int16 feature[2];
        
        NAND_LOCK();
        oldExtClock4KHz = 3072;
        oldClockX = 12;
        
        fsNandPhys.fifoAddr = __BYTEBUS_PERIP_MEM_BASE;
        fsNandPhys.waitns = 70;
        
        PERIP(GPIO0_MODE) &= ~(GPIO0_CS1 | GPIO0_CLE | GPIO0_ALE); //Set as GPIO(0)
        PERIP(GPIO0_MODE) |= 0xff | GPIO0_READY | GPIO0_RD | GPIO0_WR; //Set as PERIP(1)
        PERIP(NF_CF) = NF_CF_INT_ENA + 3; /* int enabled + some waits */
        PERIP(GPIO0_SET_MASK)   = GPIO0_CS1; /* deasserted */
        PERIP(GPIO0_CLEAR_MASK) = GPIO0_CLE | GPIO0_ALE; //CLE 0, ALE 0
        PERIP(GPIO0_DDR)   |= GPIO0_CS1 | GPIO0_CLE | GPIO0_ALE; //Set as Output(1)

        NAND_SELECT();
        NandPutCommand(0xff); //NAND RESET
        NAND_RELEASE();

        NandGetFeature(0x01, feature);
        printf("NAND Feature: Timing: 0x%04x, 0x%04x\n", feature[0], feature[1]);
        
//      feature[0] = 0x0500; // mode 5, async
//      NandSetFeature(0x01, feature);

//      NandGetFeature(0x01, feature);
//      printf("NAND Feature: Timing: 0x%04x, 0x%04x\n", feature[0], feature[1]);
        
        NandGetIdent(0);
        printf("NAND Flash Ident: 0x%04x%04x%02x\n",nf.ident[0],nf.ident[1],nf.ident[2]>>8);
        
//      NandGetFeature(0x80, feature);
//      printf("NAND Feature: Drive: 0x%04x, 0x%04x\n", feature[0], feature[1]);
        
        NAND_UNLOCK();
}
#else
void NandInit() {
	NAND_LOCK();
	oldExtClock4KHz = 3072;
	oldClockX = 12;

	fsNandPhys.fifoAddr = __BYTEBUS_PERIP_MEM_BASE;
	fsNandPhys.waitns = 70;

	PERIP(GPIO0_MODE) &= ~(GPIO0_CS1 | GPIO0_CLE | GPIO0_ALE); //Set as GPIO(0)
	PERIP(GPIO0_MODE) |= 0xff | GPIO0_READY | GPIO0_RD | GPIO0_WR; //Set as PERIP(1)
	PERIP(NF_CF) = NF_CF_INT_ENA + 3; /* int enabled + some waits */
	PERIP(GPIO0_SET_MASK)   = GPIO0_CS1; /* deasserted */
	PERIP(GPIO0_CLEAR_MASK) = GPIO0_CLE | GPIO0_ALE; //CLE 0, ALE 0
	PERIP(GPIO0_DDR)   |= GPIO0_CS1 | GPIO0_CLE | GPIO0_ALE; //Set as Output(1)

	NAND_SELECT();
	NandPutCommand(0xff); //NAND RESET
	NAND_RELEASE();
	
	NandGetIdent(0);
	printf("NAND Flash Ident: 0x%04x%04x%02x\n",nf.ident[0],nf.ident[1],nf.ident[2]>>8);
	
	NAND_UNLOCK();
}
#endif

ioresult OnfiInit() {
	u_int16 i;
	
	NAND_LOCK();
	NAND_SELECT();
	NandPutCommand(0xec);
	Delay(10);
	NandPutAddressOctet(0x00);
	Delay(10);
	NandGetOctets(128, pageBuffer);
	NAND_RELEASE();
	NAND_UNLOCK();
	
	if (pageBuffer[0] != 0x4f4e) return SysError("No ONFI Flash");
	printf("Detected ONFI Flash: ");
	for (i=32/2; i<64/2; i++) {
		fputc(pageBuffer[i]>>8,stdout);
		fputc(pageBuffer[i]&0xff,stdout);
	}
	printf("\n");
	
	PrintBuffer2(pageBuffer,128);
	
	nf.pageSizeBytes = Swap16(pageBuffer[80/2]);
	nf.spareSizeBytes = Swap16(pageBuffer[84/2]);
	nf.sectorsPerPage = nf.pageSizeBytes / 512;
	nf.pagesPerEraseBlock = Swap16(pageBuffer[92/2]);
	nf.sectorsPerEraseBlock = nf.pagesPerEraseBlock * nf.sectorsPerPage;
	nf.eraseBlocks = Swap16(pageBuffer[96/2]);
	nf.totalSectors = nf.eraseBlocks * (u_int32)nf.sectorsPerEraseBlock;		
	{
		u_int16 *n = (void*)&nf;
		printf("Geometry: ");
		for (i=0; i<7; i++) {
			printf("%d, ",*n++);
		}
		printf("Sectors: %ld, Size: %ldK\n",nf.totalSectors,nf.totalSectors/2);
	}
	
	// Determine suitable shift amount to get block address from row address
	// ie: bits in page address
	if (nf.pagesPerEraseBlock == 128) {
		rowToBlockShift = 7;
	}
	else if (nf.pagesPerEraseBlock == 256) {
		rowToBlockShift = 8;
	}
	else if (nf.pagesPerEraseBlock == 512) {
		rowToBlockShift = 9;
	}
	else if (nf.pagesPerEraseBlock == 1024) {
		rowToBlockShift = 10;
	}
	else {
		printf("Error: unsupported pages per eraseblock\n");
		return S_ERROR;
	}
	
	if (nf.eraseBlocks != (NAND_RESERVED_AREA_START + 2 + NAND_MAX_BAD_BLOCKS)) {
		// See if parameters are reasonable
		printf("FAILURE:Eraseblock count in nandflash and nfGeometry.h not compatible. Check geometry selection.\n");
		return S_ERROR;
	}
	
	return S_OK;
}


void PrintBuffer2(u_int16 *data, s_int16 words) {
	u_int16 *d = data;
	while (words>0) {
		u_int16 i;
		d=data;
		for (i=0; i<16; i++) printf("%04x ",*d++);
		printf("  ");
		d=data;
		for (i=0; i<16; i++) {
			printf("%c",ISPRINT(*d>>8));
			printf("%c",ISPRINT(*d&0xff));
			d++;
		}
		data += 16;
		words -= 16;
		printf("\n");
	}
}

// S_OK if succeeded
// S_ERROR if failed
s_int16 GetOneBBTable(u_int16 offset) {
	u_int32 lsn = 0;
	s_int16 len = sizeof(BBTable);
	u_int32 row = NAND_RESERVED_AREA_START << rowToBlockShift;
	u_int16 column = 0; // From Start
	u_int16 idx = 0;
	
	
	if (offset > 1) {
		printf("ERROR:BBTable offset is bad\n");
		return S_ERROR;
	}
	row += (nf.pagesPerEraseBlock * offset);
	
	// Read the actual thing
	idx = 0;
	while (len > 0) {
		len -= NAND_SECTOR_DATA_SIZE_BYTES>>1;
		lsn = NandReadPhysicalAndLba(row, column + (SECTOR_SIZE_BYTES*idx), NAND_SECTOR_DATA_SIZE_BYTES, &pageBuffer[(NAND_SECTOR_DATA_SIZE_BYTES>>1)*idx]);
		idx++;
		if (nf.lastReadRSFail == 1) {
			// Reed Solomon noped out, data is not ok
			return S_ERROR;
		}
	}
	
	// Check validity
	if(pageBuffer[0] == MAGIC_FDFE) {
		memcpyXY(&BBTable, pageBuffer, sizeof(BBTable));
		PrintBuffer2(pageBuffer, sizeof(BBTable));
		return S_OK;
	}
	else {
		return S_ERROR;
	}
}

s_int16 ReadBBTable() {
	u_int32 row = NAND_RESERVED_AREA_START << rowToBlockShift;
	u_int16 column = 0; // From Start
	u_int32 lsn = 0;
	s_int16 result = 0;
	
	nf.totalAccessibleEraseBlocks = NAND_RESERVED_AREA_START - NAND_FIRST_DATA_BLOCK;
	nf.readOnly = 0;
	
	printf("This memory should have %i user accessible eraseblocks\n", nf.totalAccessibleEraseBlocks);
	
	printf("\nRead pre-exixting bad block table table from row 0x%lx\n\n", row);
	

	result = GetOneBBTable(0);
	if (result == S_OK) {
		goto success;
	}
	result = GetOneBBTable(1);
	if (result == S_OK) {
		// if we ended up here, first instance of table is not in very good shape
		// Better rewite
		printf("Note:Rewriting damaged BBTable\n");
		SaveBBTable();
		goto success;
	}
	
	printf("Not Found. Create new table\n");	
	CreateBBTable();
	
	success:
	printf("Ok, %d bad block(s).\n",BBTable.bads);
	if (BBTable.bads == NAND_MAX_BAD_BLOCKS) {
		// Next block failure leads to catastrophy
		printf("NOTE:Bad Block Table is full, revert to read only. Next error will result in data loss\n");
		nf.readOnly = 1;
	}
	return S_OK;
}


void SaveBBTable(void) {
	u_int32 row = NAND_RESERVED_AREA_START << rowToBlockShift;
	u_int16 idx = 0;
	u_int16 len = sizeof(BBTable);
	u_int16 words = 0;
	
	printf("Formatting BBTable to pageBuffer\n");
	memset(pageBuffer, 0, sizeof(pageBuffer));

	idx = 0;
	while (len > 0) {
		if (len >= NAND_SECTOR_DATA_SIZE_BYTES>>1) {
			words = NAND_SECTOR_DATA_SIZE_BYTES>>1;
			len -= NAND_SECTOR_DATA_SIZE_BYTES>>1;
		}
		else {
			words = len;
			len = 0;
		}
		
		memcpyYX(&pageBuffer[SECTOR_SIZE_WORDS*idx], ((__mem_y u_int16 *)&BBTable + ((NAND_SECTOR_DATA_SIZE_BYTES>>1)*idx)),words);
		idx++;
	}
	
	printf("Bad block table currently:\n");
	printf("\tMarker: 0x%04x\n", BBTable.FDFE_marker);
	printf("\tbads: %u\n", BBTable.bads);
	//printf("\telement 0: 0x%04x, 0x%04x, 0x%04x\n", BBTable.bad[0].from,BBTable.bad[0].to,BBTable.bad[0].pageOffset);
	for (idx = 0; idx < BBTable.bads; idx++) {
		printf("\telement %u: 0x%04x, 0x%04x, 0x%04x\n", idx, BBTable.from[idx],BBTable.to[idx],BBTable.pageOffset[idx]);
	}
	
	printf("Saving BBTable to row 0x%lx\n",row);
	
	// Unmapped
	NandErasePhysical(row);
	
	// Unmapped
	NandProgramPagePhysical(row, pageBuffer);
	
	row += nf.pagesPerEraseBlock;
	
	// Unmapped
	NandErasePhysical(row);
	// Unmapped
	NandProgramPagePhysical(row, pageBuffer);

}



void CreateBBTable() {
	u_int32 row = 0;
	u_int16 column = 0; // From Start
	u_int32 lsn = 0;
	u_int16 idx;
	
	// Determine location of first spare in block
	column = nf.pageSizeBytes;
	
	memsetY(&BBTable, 0, sizeof(BBTable));

	BBTable.bads = 0;	
	BBTable.FDFE_marker = MAGIC_FDFE;
	
	// Erase everything.
	// well, not boot area, though. That would make things difficult
	printf("\nNAND FLASH NOT INITIALIZED, ERASING...\n");
	holdBBTableSave = 1; // Saving during this loop causes false positives when 
	for (idx = NAND_FIRST_DATA_BLOCK; idx < nf.eraseBlocks; idx++){
		row = (u_int32)idx << rowToBlockShift;
		
		// Read possible marker for bad block,
		// Note that this will fail in most impressive manner if memory is already written
		// This can only be done to uninitialized memory
		lsn = NandReadPhysicalAndLba(row, column, 2, pageBuffer);
		if (pageBuffer[0] != 0xffff) {
			// not ok
			printf("NOTE:BBmarker @ row %lu\n", row);
			MapBB(row,0); // no data associated with this
		}
		else {
			NandErasePhysical(row);
		}
	}
	holdBBTableSave = 0;
	printf(".. done, found %u bad blocks\n\n", BBTable.bads);
	
	// save the new table
	SaveBBTable();
}



s_int16 MapBB(u_int32 row, u_int16 hasData) {
	u_int16 replacement = NAND_RESERVED_AREA_START +2; // remember to add space for the actual table
	u_int16 idx = 0;
	u_int16 location = BBTable.bads;
	
	printf("Adding new mapping to BBTable\n");
	
	if (BBTable.bads >= NAND_MAX_BAD_BLOCKS) {
		printf("FAILURE:Too many bad blocks to map\n");
		return S_ERROR;
	}
	
	replacement += BBTable.bads;
	
	// look for pre-existing mapping
	// no double entries, bads still incremented to step past problematic block in spare
	for (idx = 0; idx < BBTable.bads; idx++) {
		if (BBTable.from[idx] == ((u_int16)(row >> rowToBlockShift))) {
			// save here insted of end
			location = idx;
			break;
		}
	}
	
	BBTable.from[location] = (u_int16)(row >> rowToBlockShift);
	BBTable.to[location] = replacement;
	BBTable.pageOffset[location] = (u_int16)(row & (nf.pagesPerEraseBlock-1));
	
	printf("Mapped new bad: block 0x%x is now 0x%x, page offset 0x%x\n",BBTable.from[location], BBTable.to[location],BBTable.pageOffset[location]);
	BBTable.bads++;
	
	if(hasData) {
		// NandFlash still has data that needs to be saved
		// Now that bad block table is up to date we can translate address and drop the bugger to new location
		u_int32 newRow = NfTrA(row,0);
		NAND_LOCK();
		NAND_SELECT();
		
		NandPutCommand(0x85);
		NandPutAddressOctet((u_int16)newRow);
		NandPutAddressOctet((u_int16)(newRow>>8));
		NandPutAddressOctet((u_int16)(newRow>>16));
		
		// finish 'em
		NandPutCommand(0x10); // Confirm Code
		NAND_RELEASE();
		NAND_UNLOCK();
	}
	
	// save before additional disaster
	if (holdBBTableSave != 1) {
		// saving is disabled during initial search (only!)
		SaveBBTable();
	}
	
	if (BBTable.bads == NAND_MAX_BAD_BLOCKS) {
		// Next block failure leads to catastrophy
		printf("NOTE:Bad Block Table is full, revert to read only. Next error will result in data loss\n");
		nf.readOnly = 1;
	}
}


// This function adds size of boot area to sector and maps around bad blocks
u_int32 NfTrA(u_int32 row, u_int16 isErase) {
	u_int32 target = 0;
	u_int16 idx = 0;
	
	// only reserved blocks froms start
	// this may be overrun if there is badblock here
	target = row + ((u_int32)NAND_FIRST_DATA_BLOCK << rowToBlockShift); //how many eraseblocks set aside for boot data
	
	for (idx = 0; idx < BBTable.bads; idx++) {
		// Note that nothing is ever mapped to 0, need to check arises from possible block failures in spare area
		if ((BBTable.from[idx] == (u_int16)(target >> rowToBlockShift)) && (BBTable.to[idx] != 0)){ // using target
			// hit
			if (isErase != 0 && BBTable.pageOffset[idx] != 0) {
				// This block will no longer have mixed location and is from now on entirely in spare
				BBTable.pageOffset[idx] = 0;
				SaveBBTable();
			}
			       
			if (((u_int16)(row & (nf.pagesPerEraseBlock-1)) >= BBTable.pageOffset[idx]) && BBTable.pageOffset[idx] != 0) {
				// Mixed mapping location for this block. Pages up to offset are in old block, above it in new block 
				// page address needs to be mapped low
				// map to entirely new block
				target = (u_int32)BBTable.to[idx]<< rowToBlockShift; // new and shiny block
				target += (u_int32)(nf.pagesPerEraseBlock-1) & (row - (u_int32)BBTable.pageOffset[idx]); // add subtracted page address offset
			}
			else {
				if (BBTable.pageOffset[idx] == 0) {
					// actual, erased, wellbehaved, singlelocation bad block
					target = (u_int32)BBTable.to[idx]<< rowToBlockShift; // block
					target += (u_int32)(nf.pagesPerEraseBlock-1) & (row); // add page address offset
				}
				else {
					// mapping past bad block does not come to play here despite hit
					// page remains same, drama is in higher page address
					// nothing here, not even chickens
				}
			}
			return target;
		}
	}
	
	return (target);
}



struct NandFlashDescriptor *CreateMlcFlash (struct NandFlashDescriptor *self) {
	nf.Create = CreateMlcFlash;
	nf.Read = NandReadPhysicalAndLbaMapped;
	nf.ProgramPage = NandProgramPagePhysicalMapped;
	nf.Erase = NandErasePhysicalMapped;
	NandInit();
	OnfiInit();
	
	readOpen = 0;
	readRow = 0;
	holdBBTableSave = 0;
	
	ReadBBTable();
	
	return &nf;
}
