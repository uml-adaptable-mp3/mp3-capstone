#ifndef __SYS_MEMORY_H__
#define __SYS_MEMORY_H__

#include <stdlib.h>
#include <vstypes.h>

#ifdef __VSDSP__
#define NATIVE_ALLOC_ALIGN 4
#define NATIVE_ALLOCI_ALIGN 2
#else
#define NATIVE_ALLOC_ALIGN 16 /* node pointers need more space */
#define NATIVE_ALLOCI_ALIGN 16 /* node pointers need more space */
#endif

void *__malloc(size_t size); /* size * sizeof(char) bytes */
void *__calloc(size_t number, size_t size); /* size * sizeof(char) bytes */
void *__realloc(void *ptr, size_t size); /* size * sizeof(char) bytes */
void __free(void *ptr);

void *malloc(size_t size); /* size * sizeof(char) bytes */
void *calloc(size_t number, size_t size); /* size * sizeof(char) bytes */
void *realloc(void *ptr, size_t size); /* size * sizeof(char) bytes */
void free(void *ptr);

__mem_y void *__mallocy(size_t size); /* size * sizeof(char) bytes */
__mem_y void *__callocy(size_t number, size_t size); /* size * sizeof(char) bytes */
__mem_y void *__reallocy(__mem_y void *ptr, size_t size); /* size * sizeof(char) bytes */
void __freey(__mem_y void *ptr);

__mem_y void *mallocy(size_t size); /* size * sizeof(char) bytes */
__mem_y void *callocy(size_t number, size_t size); /* size * sizeof(char) bytes */
__mem_y void *reallocy(__mem_y void *ptr, size_t size); /* size * sizeof(char) bytes */
void freey(__mem_y void *ptr);


void __InitMemAllocI(void *istart, size_t ichars);
void *__AllocMemI(size_t size);
void *__AllocMemAbsI(u_int16 addr, size_t size);
void __FreeMemI(void *ptr, size_t size);
void PrintFreeMemI(void);



void *AllocMemX(size_t size, size_t align); /* size * sizeof(u_int16) bytes */
void *__AllocMemX(size_t size, size_t align); /* size * sizeof(u_int16) bytes */
void *ReAllocMemX(void *ptr, size_t oldsize, size_t newsize, size_t align);
void *__ReAllocMemX(void *ptr, size_t oldsize, size_t newsize, size_t align);
__mem_y void *AllocMemY(size_t size, size_t align);
__mem_y void *__AllocMemY(size_t size, size_t align);
__mem_y void *ReAllocMemY(__mem_y void *ptr, size_t oldsize, size_t newsize, size_t align);
__mem_y void *__ReAllocMemY(__mem_y void *ptr, size_t oldsize, size_t newsize, size_t align);
void *AllocMemXY(size_t size, size_t align);
void *__AllocMemXY(size_t size, size_t align);
void *ReAllocMemXY(void *ptr, size_t oldsize, size_t newsize, size_t align);
void *__ReAllocMemXY(void *ptr, size_t oldsize, size_t newsize, size_t align);
void FreeMemX(void *ptr, size_t size);
void __FreeMemX(void *ptr, size_t size);
void FreeMemY(__mem_y void *ptr, size_t size);
void __FreeMemY(__mem_y void *ptr, size_t size);
void FreeMemXY(void *ptr, size_t size);
void __FreeMemXY(void *ptr, size_t size);

void *AllocMemAbsX(u_int16 addr, size_t size);
__mem_y void *AllocMemAbsY(u_int16 addr, size_t size);
void *AllocMemAbsXY(u_int16 addr, size_t size);
void *__AllocMemAbsX(u_int16 addr, size_t size);
__mem_y void *__AllocMemAbsY(u_int16 addr, size_t size);
void *__AllocMemAbsXY(u_int16 addr, size_t size);


void __InitMemAlloc(void *xstart, size_t xchars,
		    __mem_y void *ystart, size_t ychars);
void InitMemAlloc(void *xstart, size_t xchars,
		  __mem_y void *ystart, size_t ychars);

#ifdef __VSDSP__

#ifdef DIRECT_CALL_MALLOC
#define malloc(a) __malloc(a)
#define calloc(a,b) __calloc(a,b)
#define realloc(a,b) __realloc(a,b)
#define free(a) __free(a)

#define mallocy(a) __mallocy(a)
#define callocy(a,b) __callocy(a,b)
#define reallocy(a,b) __reallocy(a,b)
#define freey(a) __freey(a)
#endif/*DIRECT_CALL_MALLOC*/

#else

#define mallocy(a) malloc(a)
#define callocy(a,b) calloc(a,b)
#define reallocy(a,b) realloc(a,b)
#define freey(a) free(a)

#define AllocMemX __AllocMemX
#define ReAllocMemX __ReAllocMemX
#define AllocMemY __AllocMemY
#define ReAllocMemY __ReAllocMemY
#define AllocMemXY __AllocMemXY
#define ReAllocMemXY __ReAllocMemXY
#define FreeMemX __FreeMemX
#define FreeMemY __FreeMemY
#define FreeMemXY __FreeMemXY
#define AllocMemAbsX __AllocMemAbsX
#define AllocMemAbsY __AllocMemAbsY
#define AllocMemAbsXY __AllocMemAbsXY
#define InitMemAlloc __InitMemAlloc


#endif

extern void *sysMemory_xs;
extern __mem_y void *sysMemory_ys;

#ifdef __VSDSP__
#define XMEMADDR(a) ((u_int16)(a))
#define YMEMADDR(a) ((u_int16)(a))
#define IMEMADDR(a) ((u_int16)(a))
#else
extern void *sysMemory_is;
#define XMEMADDR(a) ((u_int16 *)(a)-(u_int16 *)sysMemory_xs)
#define YMEMADDR(a) ((u_int16 *)(a)-(u_int16 *)sysMemory_ys)
#define IMEMADDR(a) ((u_int16 *)(a)-(u_int16 *)sysMemory_is)
#endif

void PrintFreeMem(void);
void CheckMem(void);


#endif /*__SYS_MEMORY_H__*/
