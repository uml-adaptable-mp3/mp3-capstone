/**
   \file messages.h Message passing.
 */
#ifndef MESSAGES_H
#define MESSAGES_H
#include <vstypes.h>
#include "lists.h"

struct TASK;
struct MSGPORT {
    u_int16 mp_SigMask;     /**< signal mask or 0 for no action */
    __near void *mp_SigTask;/**< object to be signalled */
    struct LIST mp_MsgList; /**< message linked list  */
};

struct MESSAGE {
    struct MINNODE mn_Node; /**< For message port linking. */
    __near struct MSGPORT *mn_ReplyPort;  /**< message reply port */
    /* User-defined data */
};

/**
   Initializes a message port.
   \param port Message port to initialize.
   \param sigmask Signals to send when message arrives.
   \param sigtask Task to signal when message arrives.
 */
__near void InitPort( __near struct MSGPORT *port,
		      u_int16 sig_mask,
		      __near struct TASK *sig_task );
/**
   Waits until the message port has at least one message.

   \param port Message port to check.
 */
__near void WaitPort( __near struct MSGPORT *port );
/**
   Sends a message to a message port.

   \param port Message port to send the message to.
   \param message The message to send. Do not touch the message
   until you get it back!
 */
__near void PutMsg( __near struct MSGPORT *port,
		    __near struct MESSAGE *message );
/**
   Get the first message from a message port.

   \param port The message port to check.
   \return The removed message or NULL if there are no messages in the port.
 */
__near struct MESSAGE * __near GetMsg( __near struct MSGPORT *port );
/**
   Send the message back to its sender. If the reply port is not set,
   nothing happens.

   \param message The message to return.
 */
__near void ReplyMsg( __near struct MESSAGE *message );


#endif /* MESSAGES_H */
