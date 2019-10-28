P U R I T Y
Mapper for MLC NAND Flash

*Invocation*

purity

Without parameters, purity is loaded to memory and disk is taken to use.
If purity is already in memory, nothing happens.

purity c
       Clean the bookkeeping. Drop sectors which aren't used by filesystem.

purity D
       Continue desharding if paused and print desharding progress information.

purity i
       Print information of purity mapper

purity P
       Pause desharding

purity p
       Continue paused desharding

purity s
       Save the state of the mapper.

purity t
       Test the mapper. All mapped sectors are read.
       This is mostly used for debugging.

purity Z
       ZAP!! the disk. Disk is erased. All data is lost.
       New filesystem has to be created.

*Concepts*
Desharding - Rewriting physical blocks in continuous order so that shard
	     count decreases.
First Used SBA - Data area begin SBA block
LBA - Logical block address. These adresses are familiar 512 byte sectors
      shown to disk user (filesystem). LBAs are continous adress space.
Next Available SBA - First free SBA block.
SBA - Surface block address. SBA maps physical row and column to single address.
Shard - Shard maps chunk of LBAs to SBAs

*Supported flash geometry*

Purity MLC mapper supports and is tested 4kB write page and 1MB eraseblock.

*Mapper*

Purity mapper for MLC flash maps the physical interface for filesystem
to be used through LBAs. Bad blocks and error correction aren't part of
mapper. Those belong to physical layer (compact.c and compact.h).
Refreshing data which give read error however is handled by the mapper
in desharding task.

*Reserved space*

Physical layer reserves some pages for bad block handling. Mapper
reserves PURITY_SURFACE_START sectors for saving its own
state. Desharding reserves some space also. Its location on disk is
constantly moving as data is (re)written. These sectors are however
subtracted from filesystem. Reserving some space more is done to prevent
filling the disk to unusable high state.

*Shards*

Shards are the essential part of purity mapper. Shards are in array
and map physical flash blocks to logical blocks.  As such they can be
indexed, or iterated over. Shards are written after each other on
the disk and it is perfectly acceptable to have LBA 0x123 in bigger
SBA address than LBA 0xFEED.

*Read wear*

When one page is read many times it wears. Physical layer provides
information of pages which have generated too many errors and should
be refreshed. Purity mapper refreshes these pages by writing that
erase block again. This generates a new shard and may trigger
desharding.

*Desharing*

Desharding is procedure to make NAND flash to more writeable than
less than once. The porblem which desharding tries to solve is
the used free space. It can't be rewritten until it is erased.
Another problem is limited count of shards. See desharder.c file.

Deshard collects data and rewrites it tightly together. Desharder is
executed as separate task. 

There are three modes in desharder. Simplest one is paused
mode. Desharding is not done.

Desharding can be comibining where two shards are combined or one shard
is written after previously written shard. However end result is shard count
decreases by one.

Desharding can also try to free space from the old data by rewriting
the first shard and erasing the old start to new start. In this mode
desharding is using maximum time, so the user I/O is slowest.

When data is rewritten, it goes to end of the data area and the shard
bookkeeping between LBA <-> SBA link is updated in memory. This leaves
old data behind and makes holes how the data area on disk is used. To
reuse these unused areas, shard with first used SBA is rewritten to
the end. When data has been copied from the data area start to the end
of data-area and there is more than erase block old data, the old data
is erased and freed. This makes possible to think flash memory as
circle with geometric sector which is active data which just spins
around the disk surface.

Desharding consumes flash disk time progressively. 

If shard count is low, desharder rewrites single page and releases
flash access. In other end there is shards mode, which writes
DESHARD_MAX_WRITE_PAGES pages.

This gives desharder some advantage against file I/O. If file I/O
writes continous data sector after sector, desharder has rewritten
DESHARD_MAX_WRITE_PAGES * 8 pages when file I/O has written one page.

This slows down file I/O. USB mass media client should survive this,
but hard real time requirements wouldn't be met. Consider deshard
pausing while working with real time data. Make sure there are enough
shards available and deshard reservation is big enough.

*Removing data from flash disk*

When some files are removed from the disk, the data isn't freed from
mapper. It is required to call mapper to scrape the FAT32 filesystem and
drop the old data. This is done in start of the mapper automatically
or manually on command.

*When disk is full?*

Disk is full when new shard has to be created and there are no more
free shard space. Also if there is no free space left on the disk. The
free space might be available in small pieces of old data which
desharding hasn't erased. However it is unusable. Desharding may be
able to recover some of the unusable free space, but in situataion
where continous free space isn't available results can't be
quaranteed.

Disk can be full even when there is capacity to use. Two pathological
cases to fill the disk: Data is written starting from LBA 0 to end of
the disk. There will be one shard and 100% of capacity is used.

Writing MAX_SHARDS times sector to the start of disk to every erase
block. Data is used MAX_SHARDS * 512 bytes.

It is possible to fill temporarily disk. Some data is written multiple
times and caches are flushed. The write position advances and wraps
around the end of the disk. At some point the write would happen to
tail shard. Tail shard is the shard just above FUS. 

*Limitations*

Removing data from filesystem doesn't free shard. This has to be done
manually calling purity c

Writing disk full ends bad. First comes many deshards and then write will
fail. This may result unusable disk.

Resetting without saving state breaks the filesystem. Unload before reset.

High fill ratio and high shard count is quite bad situation. When more
data is written in this state there is requirement to free the old
data from the data-area. Shards are quite large and there are many of
them. It takes long to rewrite data to get enough space to deshard
shard count down. Also it is possible to run out of shards during
space freeing process.

*More info*

Source code: 

compact.c Implementation of the physical layer.
compact.h Declarations of the physical layer.
desharder.c Implementations of the desharding functions.
devMlcFlash.c Device driver implementation.
devMlcFlash.h Device driver declaration.
main.c Entry points of purity.
nfGeometry.h Physical layer. Nand flash geometry information.
nfInit.c Physical layer initialization implementation.
nfInit.h Physical layer initialization declaration.
rsFunc.c Reed-Solomon HW implementation.
rsFunc.h Reed-Solomon HW usage declration.
unimap.c Purity mapper implementation.
unimap.h Purity mapper declaration and configuration.
vo_fat_partial.c FAT32 filesystem functions for dropping sectors.

Notes:
design.pdf ugly drawing which somehow relates to Purity. Might
help to understand the concepts but please read the source code.

Readme.txt Is this file which tries to deliver information how to
use the Purity mapper for NAND flash.
