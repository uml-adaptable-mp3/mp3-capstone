#include <vo_stdio.h>
#include <lists.h>
#include <timers.h>
#include <audio.h>
#include <exec.h>
#include "cyclic.h"

struct Cyclic cyclic = {NULL};

#if 0
void DelayWithMask(register u_int16 ticks, u_int16 sigMask) {
  struct TIMER tmpTimer;
  if (ticks > 0) {
    SetSignal(0, SIGF_DELAY);
    tmpTimer.tm_Count = ticks;
    tmpTimer.tm_Load = 0;
    tmpTimer.tm_Task = thisTask;
    tmpTimer.tm_SigMask = SIGF_DELAY;
    tmpTimer.tm_Flags = 1; /* Started */
    Forbid();
    AddTimer(&tmpTimer);
    Wait(SIGF_DELAY | sigMask);
    RemoveTimer(&tmpTimer);
    Permit();
  }
}
#endif

void SetInCyclicQueue(register struct CyclicNode *node);


#if 0
void PrintCyclicQueue(const char *s) {
  struct CyclicNode *cyclicNode;
  Forbid();
  printf("Queue for %s, time %ld\n", s, ReadTimeCount());
  cyclicNode = (struct CyclicNode *)HeadNode(&cyclic.queue);
  while (cyclicNode) {
    printf("%-10s: Inter %7ld, Next %10ld\n",
	   cyclicNode->node.name, cyclicNode->interval, cyclicNode->nextActivation);
    cyclicNode = (struct CyclicNode *)NextNode((struct NODE *)cyclicNode);
  }
  Permit();
  printf("\n");
}
#endif


void CyclicMainLoop(void) {
  while (!cyclic.stop) {
    u_int16 delayTime = cyclic.maxLatency;
    u_int32 currentTime;
    struct CyclicNode *cyclicNode;
    Forbid();
    currentTime = ReadTimeCount();
    if ((cyclicNode = (struct CyclicNode *)HeadNode(&cyclic.queue))) {
      s_int32 longDelayTime;
      if ((s_int32)(currentTime - cyclicNode->nextActivation) >= 0) {
	cyclicNode->func(cyclicNode);
	DropCyclic(cyclicNode);
	do {
	  cyclicNode->nextActivation += cyclicNode->interval;
	} while ((s_int32)(currentTime - cyclicNode->nextActivation) >= 0);
	SetInCyclicQueue(cyclicNode);
	cyclicNode = (struct CyclicNode *)HeadNode(&cyclic.queue);
      }
      longDelayTime = cyclicNode->nextActivation - currentTime;
      if (longDelayTime <= 0) {
	/* Make sure there always is some CPU available for other processes. */
	  delayTime = 1;
      } else if (longDelayTime < delayTime) {
	delayTime = (u_int16)longDelayTime;
      }
    }
    Permit();
    Delay(delayTime);
  }
  cyclic.inUse = 0;
}

/* Creates the cyclic task */
struct Cyclic *CreateCyclic(register s_int16 stackSize, register s_int16 priority, register u_int16 maxLatency) {
  NewList(&cyclic.queue);
  cyclic.maxLatency = maxLatency;
  cyclic.stop = 0;
  if (cyclic.taskAndStack =
      CreateTaskAndStack(CyclicMainLoop, "cyclic",
			 stackSize, priority)) {
    cyclic.inUse = 1;
    return &cyclic;
  }
  return NULL;
}


void SetInCyclicQueue(register struct CyclicNode *node) {
  struct CyclicNode *cyclicNode;
  Forbid();
  cyclicNode = (struct CyclicNode *)HeadNode(&cyclic.queue);
  while (cyclicNode) {
    if ((s_int32)(cyclicNode->nextActivation - node->nextActivation) > 0) {
      AddBefore((struct NODE *)cyclicNode, (struct NODE *)node);
      goto finally;
    }
    cyclicNode = (struct CyclicNode *)NextNode((struct NODE *)cyclicNode);
  }
  AddTail(&cyclic.queue, (struct NODE *)node);
 finally:
  Permit();
}


/* Adds or modifies existing cyclic node */
struct CyclicNode *AddCyclic(register struct CyclicNode *node,
				       register u_int32 interval,
				       register u_int32 timeToNextActivation) {
  Forbid();
  DropCyclic(node);
  node->interval = interval;
  node->nextActivation = ReadTimeCount() + timeToNextActivation;
  SetInCyclicQueue(node);
 finally:
  Permit();
  return node;
}

/* Removes from cyclic */
struct CyclicNode *DropCyclic(register struct CyclicNode *node) {
  struct CyclicNode *cyclicNode;
  struct CyclicNode *res = NULL;

  Forbid();
  cyclicNode = (struct CyclicNode *)HeadNode(&cyclic.queue);
  while (cyclicNode) {
    /* Found node; remove it */
    if (node == cyclicNode) {
      RemNode(&node->node);
      res = node;
      goto finally;
    }
    cyclicNode = (struct CyclicNode *)NextNode((struct NODE *)cyclicNode);
  }
 finally:
  Permit();
  return res;
}
