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
  struct CyclicNode *perNode;
  Forbid();
  printf("Queue for %s, time %ld\n", s, ReadTimeCount());
  perNode = (struct CyclicNode *)HeadNode(&cyclic.queue);
  while (perNode) {
    printf("%-10s: Inter %7ld, Next %10ld\n",
	   perNode->node.name, perNode->interval, perNode->nextActivation);
    perNode = (struct CyclicNode *)NextNode((struct NODE *)perNode);
  }
  Permit();
  printf("\n");
}
#endif


void CyclicMainLoop(void) {
  while (!cyclic.stop) {
    u_int16 delayTime = cyclic.maxLatency;
    u_int32 currentTime;
    struct CyclicNode *perNode;
    Forbid();
    currentTime = ReadTimeCount();
    if ((perNode = (struct CyclicNode *)HeadNode(&cyclic.queue))) {
      u_int32 longDelayTime;
      if ((s_int32)(currentTime - perNode->nextActivation) >= 0) {
	perNode->func(perNode);
	DropCyclic(perNode);
	do {
	  perNode->nextActivation += perNode->interval;
	} while ((s_int32)(currentTime - perNode->nextActivation) >= 0);
	SetInCyclicQueue(perNode);
	perNode = (struct CyclicNode *)HeadNode(&cyclic.queue);
      }
      longDelayTime = perNode->nextActivation - currentTime;
      if (longDelayTime < delayTime) {
	delayTime = (u_int16)longDelayTime;
	/* Make sure there always is some CPU available for other processes. */
	if (!delayTime) {
	  delayTime = 1;
	}
      }
    }
    Permit();
    Delay(delayTime);
  }
  cyclic.inUse = 0;
}

/* Creates the cyclic task */
struct Cyclic *CreateCyclic(register s_int16 stackSize,
				    register s_int16 priority,
				    register u_int16 maxLatency) {
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
  struct CyclicNode *perNode;
  Forbid();
  perNode = (struct CyclicNode *)HeadNode(&cyclic.queue);
  while (perNode) {
    if ((s_int32)(perNode->nextActivation - node->nextActivation) > 0) {
      AddBefore((struct NODE *)perNode, (struct NODE *)node);
      goto finally;
    }
    perNode = (struct CyclicNode *)NextNode((struct NODE *)perNode);
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
  struct CyclicNode *perNode;
  struct CyclicNode *res = NULL;

  Forbid();
  perNode = (struct CyclicNode *)HeadNode(&cyclic.queue);
  while (perNode) {
    /* Found node; remove it */
    if (node == perNode) {
      RemNode(&node->node);
      res = node;
      goto finally;
    }
    perNode = (struct CyclicNode *)NextNode((struct NODE *)perNode);
  }
 finally:
  Permit();
  return res;
}
