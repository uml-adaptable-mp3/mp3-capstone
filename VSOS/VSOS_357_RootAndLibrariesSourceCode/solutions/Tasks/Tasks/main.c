/// \file main.c Tasks
/// \author Henrik Herranen, VLSI Solution Oy

// DL3 files require VSOS3 kernel version 0.3x to run.
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <stdlib.h>
#include <exec.h>
#include <lists.h>
#include <timers.h>
#include <imem.h>
#include <hwLocks.h>

#if 0
#define USE_FORBID
#endif

#if 0
#define TEST_HWLOCK_QUEUE
#include <cyclic.h>
#endif

extern const u_int16 __mem_y hwLockOffset[HW_LOCK_GROUPS+1];
extern const u_int16 __mem_y hwLockN[HW_LOCK_GROUPS];

const char *lockGroupNames[3] = {"BUFFER", "IO", "PERIP"};

const char *lockNames[3][32] = {
  {"HLB_0", "HLB_1", "HLB_2", "HLB_3",
   "HLB_4", "HLB_5", "HLB_USER_0", "HLB_USER_1",
   "HLB_USER_2", "HLB_USER_3"},
  {"HLIO_NF", "HLIO_NFCE/HLIO_0_11", "HLIO_SPI1", "HLIO_SPI1CS/HLIO_1_04",
   "HLIO_SPI0", "HLIO_SPI0CS/HLIO_1_00", "HLIO_JTAG", "HLIO_UART",
   "HLIO_I2S", "HLIO_I2SCLK", "HLIO_SD", "HLIO_ETH/HLIO_DIA",
   "HLIO_1_15", "HLIO_SPDIF", "HLIO_0_14", "HLIO_0_15",
   "HLIO_PWRBTN", "HLIO_AUX0/HLIO_L2I2", "HLIO_AUX1/HLIO_L2I1", "HLIO_AUX2/HLIO_L1I3",
   "HLIO_AUX3/HLIO_L3I2/HLIO_MIC2P", "HLIO_AUX4/HLIO_L3I1/HLIO_MIC2N", "HLIO_MIC1P/HLIO_L1I2", "HLIO_MIC1N/HLIO_L1I1",
   "HLIO_FM", "HLIO_USB", "HLIO_PWM"},
  {"HLP_DAC", "HLP_USB", "HLP_ETH", "HLP_SPISLAVE",
   "HLP_NAND", "HLP_SD", "HLP_RSEN", "HLP_RSDE",
   "HLP_SPI0", "HLP_SPI1", "HLP_MAC1", "HLP_MAC0",
   "HLP_GPIO0", "HLP_GPIO1", "HLP_GPIO2", "HLP_MAC2",
   "HLP_I2S", "HLP_UART", "HLP_TIMER0", "HLP_TIMER1",
   "HLP_TIMER2", "HLP_FM", "HLP_SRC", "HLP_DAOSET",
   "HLP_RTC", "HLP_REGU", "HLP_PWM", "HLP_SAR",
   "HLP_PLL", "HLP_RFVCO", "HLP_XPERIP_IF"}
};

void PrintLocks(void) {
  int i;
  int currGroup = 0;
  int currLockInGroup = 0;

  printf("\nHardware locks:\n");
#if 0
  printf("  L: %03lx %03lx %03lx\n",
	 hwLocks.lockBits[0],
	 hwLocks.lockBits[1],
	 hwLocks.lockBits[2]);
  printf("  Q: %03lx %03lx %03lx\n",
	 hwLocks.queueBits[0],
	 hwLocks.queueBits[1],
	 hwLocks.queueBits[2]);
#endif
  for (i=0; i<TOTAL_HW_LOCKS; i++) {
    int lBit, qBit;
    u_int32 lockBits, queueBits;
    struct TASK *queueTask;

    if (i >= hwLockOffset[currGroup+1]) {
      currGroup++;
      currLockInGroup=0;
    }

    /* Following needs to be done with interrupts disabled to make
       sure lock/queue status reads are an atomic operation. */
    Disable();
    lockBits = hwLocks.lockBits[currGroup];
    queueBits = hwLocks.queueBits[currGroup];
    queueTask = hwLocks.queue[i]->taskP;
    Enable();

    lBit = (u_int16)(lockBits>>currLockInGroup) & 1;
    qBit = (u_int16)(queueBits>>currLockInGroup) & 1;
    if (lBit | qBit) {
      printf("  %-6s %2d %s  %s",
	     lockGroupNames[currGroup], currLockInGroup,
	     lBit ? "LOCKED":"free  ",
	     lockNames[currGroup][currLockInGroup]);
      if (qBit) {
	printf(", QUEUE_TASK 0x%04x (\"%s\")",
	       queueTask, queueTask ? queueTask->tc_Node.name : "null");
      }
      printf("\n");
    }
    currLockInGroup++;
  }
}


#ifdef TEST_HWLOCK_QUEUE
void MyCyclicFunc(register struct CyclicNode *cyclicNode) {
  static int i=1;
  printf("####CYC %d####\n", i++);
  ObtainHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_PWM_B);
}
struct CyclicNode myCyclicNode = {{0}, MyCyclicFunc};
#endif


void PrintInterrupts(register u_int16 verbose) {
  const char *intName[] = {
    "DAC", "USB", "XPERIP", "SPI0", "SPI1", "MAC1", "MAC0", "GPIO0",
    "GPIO1", "GPIO2", "MAC2", "I2S", "UART_TX", "UART_RX", "TIMER0", "TIMER1",
    "TIMER2", "FM", "SRC", "DAOSET", "RTC", "RDS", "SRX", "STX",
    "POSD", "REGU", "PWM", "SAR", "ERR", "ERR", "ERR", "ERR"
  };
  u_int32 h, l;
  u_int16 i;

  printf("Interrupts:\n%s", verbose ? "" : "  ");
  h = ((u_int32)PERIP(INT_ENABLE1_HP) << 16) | PERIP(INT_ENABLE0_HP);
  l = ((u_int32)PERIP(INT_ENABLE1_LP) << 16) | PERIP(INT_ENABLE0_LP);
  for (i=0; i<32; i++) {
    int pri = ((u_int16)l&1) | (((u_int16)h&1)<<1);
    l >>= 1;
    h >>= 1;
    if (pri) {
      if (verbose) {
	u_int16 addr = (u_int16)(ReadIMem((0x20+i))>>6L);
	printf("  INT %2d INT_%-7s , pri %d, vector 0x%04x",
	       i, intName[i], pri, addr);
	if (verbose) {
	  printf("= ");
	  RunLibraryFunction("TRACE", ENTRY_1, addr);
	}
	printf("\n");
      } else {
	printf(" %d:%s:%d", i, intName[i], pri);
      }
    }
  }
  if (!verbose) {
    printf("\n");
  }
}




extern __near struct LIST readyQueue;
extern __near struct LIST waitQueue;
extern __near struct LIST timerQueue;

#define MAX_TASKS 16

struct TaskStore {
  u_int16 queueNumber;
  struct TIMER *tim;
  struct TASK *task;
  u_int16 count;
} __mem_y taskStore[MAX_TASKS];

void PrintXY(register u_int16 *addr, register u_int16 size) {
  int i;
  for (i=0; i<size; i++) {
    printf("    %04x -%x: %04x %04x\n", addr, size-i-1, *addr, *((u_int16 __mem_y *)addr));
    addr++;
  }

}

struct Reg {
  s_int16 offset;
  const char *name;
} const reg[] = {
  {    10, "i0"},
  {   -10, "i1"},
  {     2, "i2"},
  {    -2, "i3"},
  {     0, NULL},
  {     1, "i4"},
  {   -11, "i5"},
  {-32768, "i6"},
  {    14, "i7"},
  {     0, NULL},
  {     9, "a2"},
  {    -7, "a1"},
  {     7, "a0"},
  {    -9, "b2"},
  {    -5, "b1"},
  {     5, "b0"},
  {     0, NULL},
  {     8, "c2"},
  {    -4, "c1"},
  {     4, "c0"},
  {    -8, "d2"},
  {    -3, "d1"},
  {     3, "d0"},
  {     0, NULL},
  {    -6, "p1"},
  {     6, "p0"},
  {    -1, "ls"},
  {    12, "le"},
  {   -12, "lc"},
  {    11, "mr0"},
  {   -14, "lr0"},
};

s_int16 MaxStackUsage(register u_int16 *addr, register u_int16 size) {
  u_int16 xData, yData;

  addr += size-1;

  xData = *addr;
  yData = *((u_int16 __mem_y *)addr);

#if 0
  if (addr && size <= 0x200) {
    int i;
    for (i=0; i<size; i++) {
      printf("%04x: %04x %04x\n", addr, *addr, *((u_int16 __mem_y *)addr));
      addr--;
    }
    addr += size;
  }
#endif

  while (size && *addr == xData && *((u_int16 __mem_y *)addr) == yData) {
    addr--;
    size--;
  }

  return size;
}


void TraceStack(register u_int16 *stackStart, register u_int16 stackSize,
		register const u_int16 *s) {
  if (s <= stackStart || s >= stackStart+stackSize) {
    /* Weird start point; just start from last word */
    s = stackStart+stackSize-1;
  }
  while (s > stackStart && s < stackStart+stackSize) {
    u_int16 *prev = (u_int16 *)(*(u_int16 __mem_y *)s);
    u_int16 regLR0 = (*(u_int16 *)(s-1));
#if 0
    printf("    ### TraceStack start %x, size %x, s %x, *s %x\n",
	   stackStart, stackSize, s, *s);
#endif
    if ((u_int16 *)(*s) == s) {
      printf("    Next: PC 0x%04x @ stack 0x%04x, ", regLR0, s);
      if (regLR0 == (u_int16)exit) {
	printf("IROM::exit");
      } else {
	RunLibraryFunction("TRACE", ENTRY_1, (u_int16)regLR0);
      }
      printf("\n");
      /* If prev looks good, use it, otherwise continue one word at a time */
      if (prev > stackStart && prev < s) {
	s = prev;
      } else {
	s--;
      }
    } else {
      /* Didn't find match, continue one word at a time */
      s--;
    }
#if 0
    printf("    ### -> s %x\n", s);
#endif
  }
}

/*
  A task that is in wait has the following context relative to task->tc_SPReg:

	Offset	X	Y
	-e	i7	lr0	lr0 = function return address
	-d	PChi	PClo	PC = current program counter
	-c	le	lc
	-b	mr0	i5
	-a	i0	i1
	-9	a2	b2
	-8	c2	d2
	-7	a0	a1
	-6	P0	P1	(Product register contents)
	-5	b0	b1
	-4	c0	c1
	-3	d0	d1
	-2	i2	i3
	-1	i4	ls
	 0	-	fC	fc = forbidCount

 */

DLLENTRY(main)
ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int i;
  int tasks = 0;
  ioresult res = S_ERROR;
  struct TASK *task;
  int verbose = 1;
  u_int16 *traceLib = NULL;

#if 0
  __mem_y u_int16 *pp = (void *)taskStore;
  for (i=0; i<64; i++) {
    printf("%x ", *pp++);
  }
  printf("\n");
#endif

#ifdef TEST_HWLOCK_QUEUE
  AddCyclic(&myCyclicNode, TICKS_PER_SEC/10, TICKS_PER_SEC/10);
#endif

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: Tasks [-v|+v] [-h]\n"
	     "-v|+v\tVerbose on/off\n"
	     "-h\tShow this help\n");
      res = S_OK;
      goto finally;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else {
      printf("E: Unknown parameter \"%s\"\n", p);
      goto finally;
    }
    p += strlen(p)+1;
  }

  if (verbose) {
    /* Preload TRACE.DL3. This will make all later calls to it faster
       because it doesn't need to be loaded for each invocation. */
    traceLib = LoadLibrary("TRACE");
  }

#ifdef USE_FORBID
  Forbid();
#endif

  /* Put thisTask in front of the queue. */
  taskStore[0].queueNumber = 3;
  taskStore[0].task = thisTask;
  tasks = 1;

  /* Collect information on all tasks.
     We need to disable multitasking so as not to give the scheduler
     a chance to muck the task lists up. */
  for (i=0; i<3; i++) {
    static const struct LIST *l[3] = {&readyQueue, &waitQueue, &timerQueue};
    if (i==2) {
      struct TIMER *tim = (struct TIMER *)HeadNode(l[i]);
      while (tim) {
	taskStore[tasks].queueNumber = i;
	taskStore[tasks].tim = tim;
	taskStore[tasks].task = tim->tm_Task;
	taskStore[tasks].count = tim->tm_Count;
	tasks++;
	tim = (struct TIMER *)NextNode((struct NODE *)tim);
      }
    } else {
      task = (struct TASK *)HeadNode(l[i]);
      while (task) {
	taskStore[tasks].queueNumber = i;
	taskStore[tasks].task = task;
	tasks++;
	task = (struct TASK *)NextNode((struct NODE *)task);
      }
    }
  }


  printf("\n");
  /* Print information on the tasks */
  for (i=0; i<tasks; i++) {
    static const char *n[4] = {"readyQueue", "waitQueue", "timerQueue", "RUNNING"};
    static const char *taskStates[7] = {
      "TS_INVALID", "TS_ADDED", "TS_RUN", "TS_READY", "TS_WAIT", "TS_EXCEPT",
      "TS_REMOVED"
    };
    s_int16 maxStackUsage;

    task = taskStore[i].task;
    maxStackUsage = MaxStackUsage(task->tc_Stack, task->tc_StackSize);
    if (taskStore[i].queueNumber == 2) {
      /* Timer instead of task */
      struct TIMER *tim = taskStore[i].tim;
      printf("Timer queue 0x%04x for task 0x%04x (\"%s\")\n",
	     tim, task, task->tc_Node.name);
      printf("  Tick count: 0x%04x\n\n", taskStore[i].count);
    } else {
      u_int16 pc = (taskStore[i].queueNumber != 3) ?
	*((u_int16 __mem_y *)task->tc_SPReg-0xd) : GetLR0();
      /* Task */
      printf("Task 0x%04x, priority %d, in %s, name \"%s\"\n",
	     task, task->tc_Node.pri, n[taskStore[i].queueNumber],
	     task->tc_Node.name);
      printf("  State: %d (%s)\n", task->tc_State,
	     (task->tc_State > 6) ? "<?>" : taskStates[task->tc_State]);
      printf("  Stack: Start 0x%04x, size 0x%x, in use 0x%x, max used 0x%x (0x%x free)\n",
	     task->tc_Stack, task->tc_StackSize,
	     (u_int16 *)(task->tc_SPReg)-(u_int16 *)(task->tc_Stack),
	     maxStackUsage, task->tc_StackSize-maxStackUsage);
      if (verbose) {
	printf("  Stack Trace: current PC 0x%04x, ", pc);
	RunLibraryFunction("TRACE", ENTRY_1, pc);
	printf("\n");
	if (taskStore[i].queueNumber != 3) {
	  TraceStack(task->tc_Stack, task->tc_StackSize, (u_int16 *)(*((u_int16 *)task->tc_SPReg-1)));
	} else {
	  TraceStack(task->tc_Stack, task->tc_StackSize, (u_int16 *)GetI6());
	}

	if (taskStore[i].queueNumber < 2) {
	  struct Reg *r = reg;
	  int j;
	  printf("  Registers:\n");
	  printf("   ");
	  for (j=0; j<sizeof(reg)/sizeof(reg[0]); j++) {
	    u_int16 val;
	    if (!r->name) {
	      printf("\n   ");
	    } else {
	      if (r->offset == -32768) {
		val = (u_int16)((u_int16 *)task->tc_SPReg-0xe);
	      } else if (r->offset < 0) {
		val = *((u_int16 __mem_y *)task->tc_SPReg+r->offset);
	      } else {
		val = *((u_int16 *)task->tc_SPReg-r->offset);
	      }
	      printf(" %s:0x%04x", r->name, val);
	    }
	    r++;
	  }
	  printf("\n");
	}
      }
      printf("\n");

#if 0
      PrintXY((u_int16 *)task->tc_SPReg-15, 16);
#endif

    }
  } /* for (i=0; i<tasks; i++) */


#ifdef USE_FORBID
  Permit();
#endif

  PrintInterrupts(verbose);

  if (verbose) {
    PrintLocks();
  }

 finally:
  if (traceLib) {
    DropLibrary(traceLib);
    traceLib = NULL;
  }

  return S_OK;
}
