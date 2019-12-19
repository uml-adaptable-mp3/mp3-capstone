#ifndef _UI_H_
#define _UI_H_

#include <vstypes.h>

ioresult uiInit();
void uiLoadHeader();
void uiLoadMainMenu();
void uiLoadNowPlaying();

void uiMetadataDecodeCallBack(s_int16 index, u_int16 message, u_int32 value);
void uiLoadCriticalErrorMenu();

void uiResetSong();
void uiHideSongPlaybackBar();
void uiUpdatePlaybackTime(u_int16 new_time);
void uiUpdatePercentComplete(u_int16 percent_complete);
void uiShowPlayPause(u_int16 isPaused);
void uiCursorUp();
void uiCursorDown();
void uiDisplayVolume();
void uiDisplayBattery();
void uiDisplayMode(u_int16 mode);
char* uiCursorSelect();
void uiLoadPlaylistNames();
int getUIState(void);

#endif  // _UI_H_
