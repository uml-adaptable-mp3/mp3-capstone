#include <vstypes.h>
#include <exec.h>
#include <semaphor.h>
#include <string.h>
#include <vo_stdio.h>
#include <vs1005g.h>
#include <hwLocks.h>


struct HwLocks hwLocks;

const u_int16 __y hwLockOffset[HW_LOCK_GROUPS+1] =
  {0, BUFFER_HW_LOCKS, BUFFER_HW_LOCKS+IO_HW_LOCKS, TOTAL_HW_LOCKS};
const u_int16 __y hwLockN[HW_LOCK_GROUPS] =
  {BUFFER_HW_LOCKS, IO_HW_LOCKS, PERIP_HW_LOCKS};


#if 0
auto void SendUart(char ch) {
  while (PERIP(UART_STATUS) & UART_ST_TXFULL)
    ;
  PERIP(UART_DATA) = ch;
}

auto void SendUartX16(u_int16 n, char c) {
  int i;
  static int hex[16] = "0123456789abcdef";
  for (i=3; i>=0; i--) {
    SendUart(hex[(n >> 4*i)&0xf]);
  }
  SendUart(c);
}
auto void SendUartX32(u_int32 n, char c) {
  int i;
  static int hex[16] = "0123456789abcdef";
  for (i=7; i>=0; i--) {
    SendUart(hex[(n >> 4*i)&0xf]);
  }
  SendUart(c);
}

auto void SendUartNQ(char ch) {
  if (!(PERIP(UART_STATUS) & UART_ST_TXFULL)) {
    PERIP(UART_DATA) = ch;
  }
}
#endif

auto void InitHwLocks(void) {
  memset(&hwLocks, 0, sizeof(hwLocks));
  //  PERIP(XP_ST) = 0x7FFF; // Activate XPERIP interrupts
}


#if 0
auto void PrintHwLock(s_int16 group, s_int16 bit) {
  u_int32 bitMask = 1UL << bit;
  printf("PrintHwLock(%d, %2d), Q %08lx %08lx %08lx, l %08lx, %08lx %08lx\n",
	 group, bit,
	 hwLocks.queueBits[0], hwLocks.queueBits[1], hwLocks.queueBits[2],
	 hwLocks.lockBits[0], hwLocks.lockBits[1], hwLocks.lockBits[2]);
  if (hwLocks.lockBits[group] & bitMask) {
    if (hwLocks.queueBits[group] & bitMask) {
      struct HwLockQueue *q = hwLocks.queue[hwLockOffset[group]+bit];
      struct NODE *n = HeadNode(&q->queue);
      printf("  Locked & Queued, queue %p, Head(q) %p\n", q, n);
      while (n) {
	q = (struct HwLockQueue *)n;
	printf("    Node %p, taskP %p\n", q, q->taskP);
	n = NextNode(n);
      }
    } else {
      printf("  Locked\n");
    }
  }
  printf("End of lock\n");
}
#endif


/*
  QueueHwLock() sets up a semaphore for the bit it waits for,
  then waits until it is freed. After this call the hardware lock
  will be allocated to this task.
*/
auto enum LockError QueueHwLock(register u_int16 group, register u_int16 bit) {
  /* Note: function is entered and exit in Disable() state. */
  struct HwLockQueue **qp = hwLocks.queue+(hwLockOffset[group]+bit);
  struct HwLockQueue thisQueue;

  // Initialize an empty list.
  memset(&thisQueue, 0, sizeof(thisQueue));
  NewList(&thisQueue.queue);
  thisQueue.taskP = thisTask;

  // If there isn't a queue, make us the queue
  if (!(*qp)) {
    *qp = &thisQueue;
    hwLocks.queueBits[group] |= 1UL<<bit;
  }

  // Add us to the tail of the queue
  AddTail(&((*qp)->queue), &(thisQueue.node));
  Enable();
  Wait(SIGF_HW_LOCK_READY);
  Disable();
  // This wait will not be over until ReleaseHwLock() has released
  // thisQueue, and removed us from the queue. Because the function
  // does not remove the allocation bit, it is now allocated to us.

  return leOk;	
}



#if 1
auto enum LockError ObtainHwLocksBIP(register __reg_b u_int32 slb,
				     register __reg_c u_int32 slio,
				     register __reg_d u_int32 slp) {
#if 0
  return GetHwLocksBIP(slb, slio, slp, 1);
#else
  u_int32 __y lockBits[HW_LOCK_GROUPS];
  u_int16 group;
  u_int32 *hwllb = hwLocks.lockBits;
  u_int32 __y *lb = lockBits;

  lockBits[0] = slb;
  lockBits[1] = slio;
  lockBits[2] = slp;

  for (group=0; group<HW_LOCK_GROUPS; group++) {
    /* Disable() is required so that *hwllb doesn't change between comparison
       and potential quick allocation (if there are not queues) */
    Disable();
    
    if (*lb & *hwllb) { /* Do we need to queue? */
			/* We need to queue: check bit-by-bit which bits need queueing */
      int bit;
      u_int32 currBitMask = 1L;
      int n = hwLockN[group];
      /* The following Disable() / Enable() pairs make sure that there are
	 no too long interrupt service delays */
      Enable();
      for (bit=0; bit<n; bit++) {
	if (*lb & currBitMask) {
	  /* Here we have to disable so that we won't lose *hwllb between
	     here and QueueHwLock(). */
	  Disable();
	  if (*hwllb & currBitMask) {
	    /* This bit needs to be queued for so wait to get the allocation. */
	    QueueHwLock(group, bit);
	  } else {
	    *hwllb |= currBitMask;
	  }
	  Enable();
	}
	currBitMask <<= 1;
      }
      Disable();
    } else {
      /* No queueing required, we can just set up all our lock bits */
      *hwllb |= *lb;
    }
    Enable();
    hwllb++;
    lb++;
  }


  return leOk;
#endif
}
#endif


#if 1
auto u_int16 AttemptHwLocksBIP(register __reg_b u_int32 slb,
			       register __reg_c u_int32 slio,
			       register __reg_d u_int32 slp) {
#if 0
  return GetHwLocksBIP(slb, slio, slp, 0);
#else
  u_int32 __y lockBits[HW_LOCK_GROUPS];
  u_int16 group;
  u_int32 *hwllb = hwLocks.lockBits;
  u_int32 __y *lb = lockBits;
  u_int16 returnCode = leOk;

  lockBits[0] = slb;
  lockBits[1] = slio;
  lockBits[2] = slp;
  Disable();

  for (group=0; group<HW_LOCK_GROUPS; group++) {
    if (*lb++ & *hwllb++) { /* Is any resource allocated? */
      returnCode = leAlreadyAllocated;
    }
  }

  if (returnCode == leOk) {
    hwllb -= HW_LOCK_GROUPS;
    lb -= HW_LOCK_GROUPS;
    for (group=0; group<HW_LOCK_GROUPS; group++) {
      *hwllb++ |= *lb++;
    }
  }
 
  Enable();
  return returnCode;
#endif
}
#endif



auto enum LockError ReleaseHwLock(register u_int16 group,
				  register u_int16 bit,
				  register u_int32 *hwlqb) {
  u_int32 bitMask = 1UL<<bit;
  struct HwLockQueue **qp = hwLocks.queue+(hwLockOffset[group]+bit);
  struct LIST *queueList1 = &((*qp)->queue);
  /* This function has been called in Disable() state, so we
     can be sure that no other process / interrupt get CPU now */

  if (*hwlqb & bitMask) {
    // Let the first in queue know that it can continue execution
    // The signal can be given already now because we are in Disable() state
    Signal((*qp)->taskP, SIGF_HW_LOCK_READY);
    if (HeadNode(queueList1) == TailNode(queueList1)) {
      // There is only one in the queue, remove whole queue
      *qp = NULL;
      *hwlqb &= ~bitMask;
    } else {
      // Else make copy of queue to next element, then make that the queue,
      // so that first element space can be freed.
      struct NODE *n2 = NextNode(HeadNode(queueList1));
      struct HwLockQueue *hwlq2 = (struct HwLockQueue *)n2;
      struct LIST *queueList2 = &(hwlq2->queue);
      struct NODE *tmpNode;
      // Remove first node
      RemNode(HeadNode(queueList1));
      // Move other nodes to list 2
      while ((tmpNode = HeadNode(queueList1))) {
	RemNode(tmpNode);
	AddTail(queueList2, tmpNode);
      }
      *qp = hwlq2;
    }
    /* Now the signal to the next task which is in QueueHwLock()'s Wait()
       has been given, and the queue has been reordered, or removed if
       necessary. We will not release the hardware resource as it is
       now explicitly in the ownership of the process that received
       our Signal() */
  } else {
    hwLocks.lockBits[group] &= ~bitMask;
  }

  return leOk;
}



auto enum LockError ReleaseHwLocksBIP(register __reg_b u_int32 slb,
				      register __reg_c u_int32 slio,
				      register __reg_d u_int32 slp) {
  u_int32 __y lockBits[HW_LOCK_GROUPS];
  s_int16 group;
  u_int32 *hwllb = hwLocks.lockBits+(HW_LOCK_GROUPS-1);
  u_int32 *hwlqb = hwLocks.queueBits+(HW_LOCK_GROUPS-1);
  u_int32 __y *lb = lockBits+(HW_LOCK_GROUPS-1);
  enum LockError err = leOk;

  lockBits[0] = slb;
  lockBits[1] = slio;
  lockBits[2] = slp;

  for (group=HW_LOCK_GROUPS-1; group>=0; group--) {
    Disable(); // Make lock check and removal atomic
    if (*hwlqb & *lb) { /* Are there queues, so do we need to free one-by-one?*/
      s_int16 bit;
      // Let's remove the locks / queues one by one
      while ((bit = MSBSetU32(*lb)) >= 0) {
	*lb &= ~(1UL<<bit);
	ReleaseHwLock(group, bit, hwlqb);
      }
    } else {
      /* There were no queues, we can just release all our lock bits */
      *hwllb &= ~(*lb);
    }
    Enable();
    hwllb--;
    hwlqb--;
    lb--;
  }

  return leOk;
}
