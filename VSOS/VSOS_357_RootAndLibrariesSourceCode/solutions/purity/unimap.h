#ifndef UNIMAP_H
#define UNIMAP_H
#include <timers.h>
#include "compact.h"
#include <taskandstack.h>
#include <hwlocks.h>

extern DEVICE devMlcFlash;
void CreateMapper();
void PrintShards();

/* Configuration constants */
/* Which drive letter to use. USE CAPS OR EVIL THINGS HAPPEN. */
#define PURITY_DRIVE_LETTER 'M'
/* Where the flash usage begins. 0 is used for state. */
#define PURITY_SURFACE_START (2L * nf.sectorsPerEraseBlock)
/* Deshard is triggered MAX - THRESHOLD */
#define PURITY_DESHARD_THRESHOLD (20)

/* Start deshard at PURITY_MAX_SHARDS / 2 + PURITY_SMALL_HYSTERESIS and
 * end when         PURITY_MAX_SHARDS / 2 - PURITY_SMALL_HYSTERESIS.
 * Desharding smallest shards together.*/
#define PURITY_SMALL_HYSTERESIS (PURITY_MAX_SHARDS / 10)

/*** Space reserving constants. ***/
/* Design tip: minimum desharding block count = 
               available erase blocks / PURITY_MAX_SHARDS.
 */
/* DISK:   How many erase blocks should be reserved for desharding. */
#define PURITY_DESHARDING_BLOCKS 70UL
/* MEMORY: Max shard count. */
#define PURITY_MAX_SHARDS 300
/* Max pages written before Yield() Must be greater than 2 */
#define DESHARD_MAX_WRITE_PAGES 16
/* Which lock is used */
#define DESHARDER_LOCK HLB_USER_0

/* Private stuff. Don't touch my privates or bad things happen. */

#define PURITY_SECTORS_PER_PAGE 8

struct PageAddressStruct {
	s_int32 firstLba;    ///< start logical block address of page
	s_int32 nextSba;     ///< surface block address for next write
	s_int32 lastSba;     ///< surface block address for write stop
	u_int32 lbaPresent;  ///< bitfield of read and maybe modified logical blocks
};
struct PageStruct {
	struct PageAddressStruct addr;
	u_int16 data[NAND_PAGE_AND_SPARE_SIZE_WORDS];
};
struct Purina {
	s_int32 nextAvailableSba; ///< start of the next free eraseblock
	//u_int16 pgSectors; ///< how many sectors are currently in the pageBuffer
	u_int16 shards; ///< how many shards there are now
	u_int16 maxShards; ///< size of shard array
	s_int32 firstUsedSba; ///< Lowest SBA in use.
	u_int32 usableSectors; ///< When deshard was started. The Magic Zero is used to disable.
	struct PageStruct pg[2];
};
struct Shard {
	s_int32 startLba; ///< first logical block address of shard
	s_int32 startSba; ///< first surface block address of shard
	s_int32 sectors;  ///< how many 512-byte sectors this shard has
};

extern struct Purina __mem_y pur;
extern struct Shard __mem_y shard[PURITY_MAX_SHARDS];
extern u_int32 realDataStart;
extern u_int16 pageBuffer[NAND_PAGE_AND_SPARE_SIZE_WORDS];
extern __mem_y struct PageAddressStruct deshardPage;

void LoadState();
void SaveState();
void UpdateFatBorders();

s_int32 ShardFindSector(register s_int32 lba);
ioresult ShardAddSector(register s_int32 lba, register s_int32 sba);
ioresult StoreLba(u_int32 lba, u_int16 *data);
ioresult ReadLba(u_int32 lba, u_int16 *data);
void NextWritePage(register __mem_y struct PageStruct *page, register u_int16 align);
__mem_y struct Shard *  PushFUS();

void TestMapper();
void InfoMapper();
void EraseDisk();
void ReclaimFreeSpace();
void KillDesharder();
ioresult SimpliSharder();
void SimpliRefresher();
void DeshardPause();
/* Deshard task stuff */

#define DESHARD_DEAD       0
#define DESHARD_RUN        (1 << 0)
#define DESHARD_PAUSE      (1 << 1)
#define DESHARD_PAUSED     (1 << 2)
#define DESHARD_ALIVE      (1 << 4)
#define DESHARD_DIE        (1 << 5)

struct DeshardTask{
	struct TaskAndStack *tas;
	u_int16 deshardMode;
	u_int16 pagesToWrite;
	u_int16 sortNeeded;
	s_int16 blockStep;    // How many useless sectors is +1 in pages to write.
	u_int16 noPairShards; // Number of shards when no pair situation was found.
	u_int32 sectors;
	u_int32 uselessSectors;
};
extern struct DeshardTask deshardControl;
#endif
