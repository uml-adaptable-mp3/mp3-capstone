/// \file uimaker.h Automatic User Interface Generator


#ifndef UIMAKER_H
#define UIMAKER_H

#include <uimessages.h>
#include <stdbuttons.h>

/// Structure to hold information about the current state 
/// of a user interface and message passing.
typedef struct uiInfoStruct {
	StdButton *buttons;
	void (*modelCallback)(s_int16 index, u_int16 message, u_int32 value);	
	const char **modelStateText;
	UiMessageListItem __y *msgList;
	UiMessage *lastMessage;
	void (*uiCallback)(s_int16 index, u_int16 message, u_int32 value);	
	u_int16 tag[10];	//10 words of private state memory for the ui handler
} UiInfo;

#define UIMODULE (('A'<<8) + 'U')

#define UI_CREATE 1
#define UI_MESSAGE 2
#define UI_HANDLE 3

/// Create an user interface. \param a pointer to an uiInfoStruct
#define UserInterfaceCreate(a){CallKernelModule(UIMODULE,UI_CREATE,(a));}

/// Send a message to the user interface
#define UserInterfaceSendMessage(a){CallKernelModule(UIMODULE,UI_MESSAGE,(a));}

/// Handle one step of the user interface and retrieve an ui event if any
#define UserInterfaceGetEvent(a)(CallKernelModule(UIMODULE,UI_HANDLE,(a)))


#endif
