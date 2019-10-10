
#ifndef UIMSG_U16_SCAN

// UNMODIFIED from uimessages.h
// UIMSG_BUT_PREVIOUS 0x0107 //Switch to the previous track
// UIMSG_BUT_NEXT 0x0108 //Switch to the next track


// to Model
#define UIMSG_U16_SCAN_FM     0x0281 // Scan FM, parameter = number of channels
#define UIMSG_U32_SET_FM_FREQ 0x030c // Set fm frequency in kHz

// from Model
#define UIMSG_U16_FM_SCAN_RES 0x0282 // Send pointer to array of u_int32 frequencies in kHz
#define UIMSG_U32_FM_FREQ     0x030d // Current fm frequency in kHz


#define UIMSG_TEXT_RDS_PROGRAMME_SERVICE 0x050d // Programme Service Name, pointer to char[]
#define UIMSG_TEXT_RDS_RADIOTEXT         0x050e // Radiotext message, pointer to char[]




#endif