/**
   \file lists.h Doubly-linked lists.
*/
#ifndef OS_LISTS_H
#define OS_LISTS_H
#include <vstypes.h>

/** Doubly-linked list header structure. lh_Tail is always NULL.
    An empty list has its lh_Head point to lh_Tail and lh_TailPred
    points to lh_Head.
 */
struct LIST {
    __near struct NODE *head; /**< Points to the start of the list. */
    __near struct NODE *tail; /**< Contains NULL. */
    __near struct NODE *tailPred; /**< Points to the last entry in the list. */
};

struct LISTY {
    __mem_y __near struct NODEY *head; /**< Points to the start of the list. */
    __mem_y __near struct NODEY *tail; /**< Contains NULL. */
    __mem_y __near struct NODEY *tailPred; /**< Points to the last entry in the list. */
};

/*
 * These are in I memory.
 * You can't actually allocate or manipulate these from C right now.
struct LISTI {
    void *head_ptail;
    void *htail_tailPred
};
struct MINNODEI {
    void *succ_pred;
};
struct NODEI {
    void *succ_pred;
    u_int32 name_size;
};
 *
 */
u_int16 GetSizeNodeI(register __a1 __near void *node);
void SetSizeNodeI(register __a1 __near void *node, register __a0 u_int16 sz);

/** Node structure with name and priority fields.
    Priority is used by Enqueue(). */
struct NODE {
    __near struct NODE *succ;
    __near struct NODE *pred;
    __near char *name;
    s_int16 pri;
};
struct NODEY {
    __mem_y __near struct NODEY *succ;
    __mem_y __near struct NODEY *pred;
    __mem_y __near char *name;
    s_int16 pri;
};

/** Minimal node structure with just linking. */
struct MINNODE {
    __near struct NODE *succ;
    __near struct NODE *pred;
};
struct MINNODEY {
    __mem_y __near struct NODE *succ;
    __mem_y __near struct NODE *pred;
};

/** Inserts a node into a list according to priority.
    \param list The list to add to.
    \param node The node to add.
    \note One node can only exist in one list at a time.
 */
__near void Enqueue(__near register __i3 struct LIST *list,
		    __near register __i2 struct NODE *node);

/** Adds a node to the head (top) of the list.
    \param list The list to add to.
    \param node The node to add.
 */
__near void AddHead(__near register __i3 struct LIST *list,
		    __near register __i2 struct NODE *node);
__near void AddHeadY(__mem_y __near register __i3 struct LISTY *list,
		     __mem_y __near register __i2 struct NODEY *node);
__near void AddHeadI(__near register __a1 void *list,
		    __near register __a0 void *node);

void AddBefore(__near register __i3 struct NODE *ndest,
	       __near register __i2 struct NODE *node);
void AddBeforeY(__near register __i3 __mem_y struct NODEY *ndest,
		__near register __i2 __mem_y struct NODEY *node);
void AddBeforeI(__near register __i3 void *ndest,
		__near register __i2 void *node);
void AddAfter(__near register __i3 struct NODE *ndest,
	      __near register __i2 struct NODE *node);
void AddAfterY(__near register __i3 __mem_y struct NODEY *ndest,
	       __near register __i2 __mem_y struct NODEY *node);
void AddAfterI(__near register __i3 void *ndest,
	       __near register __i2 void *node);

/** Adds a node to the tail (bottom) of the list.
    \param list The list to add to.
    \param node The node to add.
 */
__near void AddTail(__near register __i3 struct LIST *list,
		    __near register __i2 struct NODE *node);
__near void AddTailY(__mem_y __near register __i3 struct LISTY *list,
		     __mem_y __near register __i2 struct NODEY *node);
__near void AddTailI( __near register __a1 void *list,
		      __near register __a0 void *node);
/** Removes a node from a list. Do not call if the node is not in a list!
    \param node The node to remove.
 */
__near void RemNode(__near register __i3 struct NODE *node);
__near void RemNodeY(__mem_y __near register __i3 struct NODEY *node);
__near void RemNodeI( __near register __i3 void *node);
/** Removes the head (top) node from a list.
    \param list The list to remove from.
 */
__near struct NODE * __near RemHead(__near register __i3 struct LIST *list);
__mem_y __near struct NODEY * __near RemHeadY(__mem_y __near register __i3 struct LISTY *list);
__near void * __near RemHeadI(__near register __i3 void *list);
/** Removes the tail (bottom) node from a list.
    \param list The list to remove from.
    \return The removed node or NULL.
 */
__near struct NODE * __near RemTail(__near register __i3 struct LIST *list);
__mem_y __near struct NODEY * __near RemTailY(__mem_y __near register __i3 struct LISTY *list);
__near void * __near RemTailI(__near register __i3 void *list);
/** Initializes a list header.
    \param list The list to initialize.
    \return The removed node or NULL.
 */
__near void NewList(__near register __i3 struct LIST *list);
__near void NewListY(__mem_y __near register __i3 struct LISTY *list);
__near void NewListI(__near register __i3 void *list);
/** Finds the head (top) node of a list without removing it.
    \param list The list to check.
    \return The head node or NULL.
 */
__near struct NODE * __near HeadNode(const __near register __i3 struct LIST *list);
__mem_y __near struct NODEY * __near HeadNodeY(__mem_y const __near register __i3 struct LISTY *list);
__near void * __near HeadNodeI(const __near register __i3 void *list);
/** Finds the next node.
    \param node The current node.
    \return The next node or NULL.
 */
__near struct NODE * __near NextNode(const __near register __i3 struct NODE *node);
__mem_y __near struct NODEY * __near NextNodeY(__mem_y const __near register __i3 struct NODEY *node);
__near void * __near NextNodeI(const __near register __i3 void *node);
/** Finds the tail (bottom) node of a list without removing it.
    \param list The list to check.
    \return The tail node or NULL.
 */
__near struct NODE * __near TailNode(const __near register __i3 struct LIST *list);
__mem_y __near struct NODEY * __near TailNodeY(__mem_y const __near register __i3 struct LISTY *list);
__near void * __near TailNodeI(const __near register __i3 void *list);
/** Finds the previous node.
    \param node The current node.
    \return The previous node or NULL.
 */
__near struct NODE * __near PrevNode(const __near register __i3 struct NODE *node);
__mem_y __near struct NODEY * __near PrevNodeY(__mem_y const __near register __i3 struct NODEY *node);
__near void * __near PrevNodeI(const __near register __i3 void *node);

#ifndef __VSDSP__
#define NewListI NewList
#define HeadNodeI HeadNode
#define NextNodeI NextNode
#define TailNodeI TailNode
#define PrevNodeI PrevNode
#define RemNodeI RemNode
#define AddAfterI AddAfter
#define AddBeforeI AddBefore
#define AddHeadI AddHead
#define AddTailI AddTail
#define RemHeadI RemHead
#define RemTailI RemTail
#endif

#endif /* OS_LISTS_H */

