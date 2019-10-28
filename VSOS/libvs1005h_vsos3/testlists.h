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
    __y __near struct NODEY *head; /**< Points to the start of the list. */
    __y __near struct NODEY *tail; /**< Contains NULL. */
    __y __near struct NODEY *tailPred; /**< Points to the last entry in the list. */
};

/** Node structure with name and priority fields.
    Priority is used by Enqueue(). */
struct NODE {
    __near struct NODE *succ;
    __near struct NODE *pred;
    __near char *name;
    s_int16 pri;
};
struct NODEY {
    __y __near struct NODEY *succ;
    __y __near struct NODEY *pred;
    __y __near char *name;
    s_int16 pri;
};

/** Minimal node structure with just linking. */
struct MINNODE {
    __near struct NODE *succ;
    __near struct NODE *pred;
};
struct MINNODEY {
    __y __near struct NODE *succ;
    __y __near struct NODE *pred;
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
__near void AddHeadY(__y __near register __i3 struct LISTY *list,
		     __y __near register __i2 struct NODEY *node);
#if 1
void AddBefore(__near register __i3 struct NODE *ndest,
	       __near register __i2 struct NODE *node);
void AddBeforeY(__near register __i3 __y struct NODEY *ndest,
		__near register __i2 __y struct NODEY *node);
void AddAfter(__near register __i3 struct NODE *ndest,
	      __near register __i2 struct NODE *node);
void AddAfterY(__near register __i3 __y struct NODEY *ndest,
	       __near register __i2 __y struct NODEY *node);
#else
void AddBefore(struct NODE *ndest, struct NODE *node);
void AddBeforeY(__y struct NODEY *ndest, __y struct NODEY *node);
void AddAfter(struct NODE *ndest, struct NODE *node);
void AddAfterY(__y struct NODEY *ndest, __y struct NODEY *node);
#endif

/** Adds a node to the tail (bottom) of the list.
    \param list The list to add to.
    \param node The node to add.
 */
__near void AddTail(__near register __i3 struct LIST *list,
		    __near register __i2 struct NODE *node);
__near void AddTailY(__y __near register __i3 struct LISTY *list,
		     __y __near register __i2 struct NODEY *node);
/** Removes a node from a list. Do not call if the node is not in a list!
    \param node The node to remove.
 */
__near void RemNode(__near register __i3 struct NODE *node);
__near void RemNodeY(__y __near register __i3 struct NODEY *node);
/** Removes the head (top) node from a list.
    \param list The list to remove from.
 */
__near struct NODE * __near RemHead(__near register __i3 struct LIST *list);
__y __near struct NODEY * __near RemHeadY(__near register __i3 struct LISTY *list);
/** Removes the tail (bottom) node from a list.
    \param list The list to remove from.
    \return The removed node or NULL.
 */
__near struct NODE * __near RemTail(__near register __i3 struct LIST *list);
__y __near struct NODEY * __near RemTailY(__y __near register __i3 struct LISTY *list);
/** Initializes a list header.
    \param list The list to initialize.
    \return The removed node or NULL.
 */
__near void NewList(__near register __i3 struct LIST *list);
__near void NewListY(__y __near register __i3 struct LISTY *list);
/** Finds the head (top) node of a list without removing it.
    \param list The list to check.
    \return The head node or NULL.
 */
__near struct NODE * __near HeadNode(const __near register __i3 struct LIST *list);
__y __near struct NODEY * __near HeadNodeY(__y const __near register __i3 struct LISTY *list);
/** Finds the next node.
    \param node The current node.
    \return The next node or NULL.
 */
__near struct NODE * __near NextNode(const __near register __i3 struct NODE *node);
__y __near struct NODEY * __near NextNodeY(__y const __near register __i3 struct NODEY *node);
/** Finds the tail (bottom) node of a list without removing it.
    \param list The list to check.
    \return The tail node or NULL.
 */
__near struct NODE * __near TailNode(const __near register __i3 struct LIST *list);
__y __near struct NODEY * __near TailNodeY(__y const __near register __i3 struct LISTY *list);
/** Finds the previous node.
    \param node The current node.
    \return The previous node or NULL.
 */
__near struct NODE * __near PrevNode(const __near register __i3 struct NODE *node);
__y __near struct NODEY * __near PrevNodeY(__y const __near register __i3 struct NODEY *node);

#endif /* OS_LISTS_H */

