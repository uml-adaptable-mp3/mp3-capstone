/**
   \file mempools.h Equal-size allocation from a (static) memory pool.
   Implements CreatePool(), AllocPool(), and FreePool().
   These functions have constant-time execution.
 */
#ifndef MEM_POOLS_H
#define MEM_POOLS_H
#include <vstypes.h>

/** Initializes a memory pool.
    \param block nsize*nelems words of memory.
    \param nsize Size of one element.
    \param nelems Number of elements.
*/
__near void CreatePool(__near void * register __i2 block,
		       register __a0 s_int16 nsize,
		       register __a1 s_int16 nelems);
/*void CreatePool(void *block, short esize, short nelems);*/
/** Allocates one element from the memory pool.
    \param block Memory pool.
    \param addr Pointer to a variable that gets the block address.
    \return 0 for success.
 */
__near short AllocPool(__near void * register __i2 block,
		       __near void ** register __i3 addr);
/** Frees one element to the memory pool. The element must have been
    allocated from the memory pool.
    \param block Memory pool.
    \param addr Memory to free.
 */
__near void FreePool(__near void * register __i2 block,
		     __near void * register __i3 addr);

#endif /* MEM_POOLS_H */
