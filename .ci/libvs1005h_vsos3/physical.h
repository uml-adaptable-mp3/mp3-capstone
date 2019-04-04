/**
   \file physical.h File system layer 6: Physical layer.
   This is the lowest layer of a file system. This layer takes care
   of the actual, physical connection to a device.
 */

#ifndef FS_PHYSICAL_H
#define FS_PHYSICAL_H

#include <vstypes.h>

/**
   Current version number. 8 MSBs contain version number, 8 LSBs size of
   the structure in words.

   Version number history:
   <OL>
	<LI>0x030B - 140925
		Read and Write now also can return number of errors.
	<LI>0x020B - 140515
		Sectors are now always 256 words (512 bytes),
		while pages can be any power of two >= 256 words (512 bytes)
	<LI>0x010B - 060707
		Original revision.
   </OL>
*/
#define FS_PHYSICAL_VERSION 0x030B


/**
   File system Physical layer basic structure. Each Physical layer
   internal structure should begin with this.
*/
struct FsPhysical {
  /** Version number. 8 MSBs contain version number, 8 LSBs size of
      the structure in words. */
  u_int16 version;
  /** Actual page size of the nand IC. In 16-bit words. Small page=256 */
  u_int16 pageSize;
  /** In sectors (1 sector = 512 bytes = 256 words) */
  u_int16 eraseBlockSize;
  /** The size of the memory unit in erasable blocks */
  u_int16 eraseBlocks;
  /** Creates a physical layer. param is a device-specific parameter,
      usually 0. */
  struct FsPhysical *(*Create)(u_int16 param);
  /** Delete a physical layer */
  s_int16 (*Delete)(struct FsPhysical *p);
  /** Read sectors. meta is physical-specific data and not necessarily
      used. If either data or meta is NULL, that information is not
      returned. Setting both pointers to NULL is an error condition. 
      Returns number of read sectors, or, if the return value is
      negative, the following holds:
      If -1 >= result > -256, there were -result recoverable errors.
      If result <= -256, there were -result/256 unrecoverable errors.
      Example: If result is -2, there were 2 recoverable errors. */
  s_int16 (*Read)(struct FsPhysical *p, s_int32 firstSector, u_int16 sectors,
		  u_int16 *data, u_int16 *meta);
  /** Write sectors. meta is physical-specific data and not necessarily
      used. If either data or meta is NULL, that information is not
      written. Setting both pointers to NULL is an error condition. 
      Returns number of written sectors, or, if the return value is
      negative, the following holds:
      If -1 >= result > -256, there were -result recoverable errors.
      If result <= -256, there were -result/256 unrecoverable errors.
      Example: If result is -2, there were 2 recoverable errors.

      To write a page in the nand flash, (pageSize/256) sectors must be
      written by calling Write() one or several times. The sectors must
      be written in sequence, from the beginning to the end of the page.

      Operation in out-of-sequence conditions:
      - If a write of a page is in progress and a Write() call to another
      page is made, the implementation will finalize the page write with 
      minimum amount of operations: either do a partial write or fill the
      rest of the page with zeros.
      - If the first Write() call inside a page is not to the first sector 
      of the page, the Write() call will do a partial write if possible.
      If partial writes are not possible the Write() call will fail. */
  s_int16 (*Write)(struct FsPhysical *p, s_int32 firstSector, u_int16 sectors,
		   u_int16 *data, u_int16 *meta);
  /** Erase en erase block. \e sector is the sector number of the first
      sector in the block. Returns 0 on success, non-0 on failure.
      If the result is negative, the absolute number is the number
      of errors in the erase operation. */
  s_int16 (*Erase)(struct FsPhysical *p, s_int32 sector);
  /** Frees the bus for other devices.
      Returns 0 on success, non-0 on failure. */
  s_int16 (*FreeBus)(struct FsPhysical *p);
  /** Re-initializes bus after a fatal error (eg memory card removal) 
      Returns 0 on success, non-0 on failure. */
  s_int16 (*Reinitialize)(struct FsPhysical *p);
};


#endif /* !FS_PHYSICAL_H */
