#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysmemory.h>
#include <lists.h>
#include <timers.h>
#include <exec.h>
#include "memstore.h"
#include <volink.h>
#include <string.h>




extern const void *pIFlist;

extern struct LIST *xFlist;
extern struct LISTY __mem_y *yFlist;
struct LIST *myiFlist;

struct MEMORYNODE {
	struct MINNODE node;
	u_int16 size;
};

struct MEMORYNODEY {
	struct MINNODEY node;
	u_int16 size;
};

#if 1
void PrintFreeMem(void) {
	myiFlist = pIFlist;
	fprintf(stderr,"\nFree memory:\n");
	{
		u_int16 sum = 0;
		void *m = HeadNodeI(myiFlist);
		while (m) {
			sum += GetSizeNodeI(m);
			fprintf(stderr," I:%04x..%04x\n", IMEMADDR(m), IMEMADDR(m)+GetSizeNodeI(m)-1);
			m = NextNodeI(m);			
		}
		fprintf(stderr,"I: %5u words (%5ld bytes)\n",sum,((u_int32)sum)*4);
	}
	{
		u_int16 sum = 0;
		struct MEMORYNODE *m = (void *)HeadNode(xFlist);
		while (m) {
			sum += m->size;
			fprintf(stderr," X:%04x..%04x\n", XMEMADDR(m), XMEMADDR(m)+m->size-1);
			m = (void *)NextNode((void *)m);
		}
		fprintf(stderr,"X: %5u words (%5ld bytes)\n",sum,((u_int32)sum)*2);
	}
	{
		u_int16 sum = 0;
		struct MEMORYNODEY __mem_y *m = (void __mem_y *)HeadNodeY(yFlist);
		while (m) {
			sum += m->size;
			fprintf(stderr," Y:%04x..%04x\n", YMEMADDR(m), YMEMADDR(m)+m->size-1);
			m = (void __mem_y *)NextNodeY((void __mem_y *)m);
		}
		fprintf(stderr,"Y: %5u words (%5ld bytes)\n",sum,((u_int32)sum)*2);
	}
	//	Delay(1000);
}
#endif


#if 0
void PrintFreeMem(void) {
	u_int16 elemTot = 0;
#if 0
	myIFlist = *pIFlist;
	printf("  # PrintFreeMem I %p, x %p %p, y %p %p, 1234 %d\n",
	       myIFlist, &xFlist, xFlist, &yFlist, yFlist, 1234);
#endif
	{
		void *m = HeadNodeI(myiFlist);
		while (m) {
			printf("I %04x .. %04x\n", IMEMADDR(m), IMEMADDR(m)+GetSizeNodeI(m)-1);
			m = NextNodeI(m);			
		}
	}
	{
		struct MEMORYNODE *m = (void *)HeadNode(xFlist);
		while (m) {
			printf("X %04x .. %04x\n", XMEMADDR(m), XMEMADDR(m)+m->size-1);
			m = (void *)NextNode((void *)m);
		}
	}
	{
		struct MEMORYNODEY __mem_y *m = (void __mem_y *)HeadNodeY(yFlist);
		while (m) {
			printf("Y %04x .. %04x\n", YMEMADDR(m), YMEMADDR(m)+m->size-1);
			m = (void __mem_y *)NextNodeY((void __mem_y *)m);
		}
	}
	//	Delay(1000);
}
#endif

