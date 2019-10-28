#ifndef CYCLIC_H
#define CYCLIC_H

#include <vstypes.h>
#include <lists.h>
#include "taskandstack.h"
#include <volink.h>

struct Cyclic {
  s_int16 inUse;
  s_int16 stop;
  u_int16 maxLatency; /* Tells how often the cyclic task wakes up even
			 if nothing happens, in 1/TICKS_PER_SEC seconds */
  struct TaskAndStack *taskAndStack;
  struct LIST queue;
};

extern struct Cyclic cyclic;

struct CyclicNode {
  struct NODE node;		/* For bookkeeping */
  void(*func)(register struct CyclicNode *perNode);
  u_int32 interval;		/* Interval for func. calls (i/TICKS_PER_SEC) */
  u_int32 nextActivation;	/* Next time for activation */
  u_int16 userTag;		/* Free to use by the user;
				   cyclic doesn't use this for anything. */
};

/*
  Creates the cyclic task.
  maxLatency is time in 1/TICKS_PER_SEC (milliseconds) for how often
  to engage the cyclic task even if nothing is currently happening.
  Typical good values vary between TICKS_PER_SEC/10 and TICKS_PER_SEC.
*/
struct Cyclic *CreateCyclic(register s_int16 stackSize, register s_int16 priority, register u_int16 maxLatency);

/* Deletes the cyclic task */
struct Cyclic *DeleteCyclic(void);

/* Adds or modifies existing cyclic node */
struct CyclicNode *AddCyclic(register struct CyclicNode *node, register u_int32 interval, register u_int32 timeToNextActivation);

/* Removes from cyclic */
struct CyclicNode *DropCyclic(register struct CyclicNode *node);

#endif /* CYCLIC_H */
