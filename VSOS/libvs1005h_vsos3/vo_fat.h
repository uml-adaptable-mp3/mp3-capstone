#ifndef VO_FAT_H
#define VO_FAT_H

#include <vstypes.h>

typedef struct FragmentStruct {//6 words
    u_int32 startSector; /*high bit set if last fragment*/
    u_int32 sizeBytes; /*NOTE: different than in VS1000! */
    u_int32 startByteOffset;
} Fragment;
	
typedef struct FatDeviceInfoStruct { //14 words
	u_int16 rootEntCnt;
	u_int16 numFats;
	u_int32 totalClusters;
	u_int16 fatSectorsPerCluster;
	u_int16 fatBits;
	u_int32 fatStart;
	u_int32 dataStart;
	u_int32 rootStart; //for FAT12/16 it's Sector; for FAT32 it's Cluster	
	//u_int32 partitionStart;//just an idea...
	u_int32 allocationStartCluster; //< don't allocate before this cluster (updated by allocate, relevant when 2 files are written)
} FatDeviceInfo;
	
typedef struct FatFileInfoStruct { //11 words
	Fragment currentFragment;
	u_int32 startCluster;	
	u_int32 directoryEntrySector;
	u_int16 directoryEntryNumber;//0..31			
} FatFileInfo;


u_int32 VoFatFindFreeSpace(register __i0 VO_FILE *f,u_int32 requiredClusters);

extern u_int16 vo_fat_allocationSizeClusters; /// How many clusters are allocated at a time for writing to files (default 1)
extern FILESYSTEM *userFileSystem;
extern const FILESYSTEM FatFileSystem;
/// Length of file name operations buffer (unicode16 characters)
#define __LONG_FILE_NAME_LENGTH 128

typedef struct DirectoryEntryStruct {
	u_int16 filename[5];
	u_int16 attr;
	u_int16 ntres;
	u_int16 creatTime;
	u_int16 creatDate;
	u_int16 lastAccessDate;
	u_int16 firstClusHi;
	u_int16 writeTime;
	u_int16 writeDate;
	u_int16 firstClusLo;
	u_int16 fileSizeLo;
	u_int16 fileSizeHi;
} DirectoryEntry;
#define __ATTR_READONLY 0x01
#define __ATTR_HIDDEN 0x02
#define __ATTR_SYSTEM 0x04
#define __ATTR_VOLUMEID 0x08
#define __ATTR_DIRECTORY 0x10
#define __ATTR_ARCHIVE 0x20
#define __ATTR_LONGNAME 0x0F


void PrintBuffer(u_int16 *buf);
ioresult FatFindFirst(register __i0 VO_FILE *f, const char *name, char *result, u_int16 len);
ioresult FatFindNext(register __i0 VO_FILE *f, char *result, u_int16 len);

u_int32 VoFatReadClusterRecord(register __i0 VO_FILE *f, u_int32 fatCluster);
u_int16 MegaFatGetByte(register u_int16 *buf, register u_int16 n);
u_int16 MegaFatGetWord(register u_int16 *buf, register u_int16 n);
u_int32 MegaFatGetLong(register u_int16 *buf, register u_int16 n);
auto void VoFatClusterPos(register __i0 VO_FILE *f, register u_int32 fatCluster, u_int32 *sector, u_int16 *offset);
#endif
