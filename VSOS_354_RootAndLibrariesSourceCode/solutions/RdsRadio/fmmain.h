#ifndef FM_MAIN_H
#define FM_MAIN_H

#include <vstypes.h>

typedef struct RADIOCHANNEL {
	char name[10];
	u_int32 frequency;
} RADIOCHANNEL;

extern RADIOCHANNEL newChannel, currentChannel;


#endif
