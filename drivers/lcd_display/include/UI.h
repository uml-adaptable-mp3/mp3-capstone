#ifndef _UI_H_
#define _UI_H_

#include <vstypes.h>

// #define MAIN_MENU      (1)
// #define ARTISTS_MENU   (2)
// #define PLAYLISTS_MENU (3)
// #define SONGS_MENU     (4)
// #define INFO_MENU      (5)
// #define LOAD_MENU      (6)
// #define ERR_MENU		  (7)

// #define UP             (1)
// #define DOWN           (0)

// #define TITLE			  (0)
// #define LOCATION       (1)

// void loadScanningMenu();
// void loadArtistsMenu();
// void loadPlaylistsMenu();
// void loadSongsMenu();
// void loadInfoMenu();
// void loadNoDriveMenu();
// void moveArrowSelectionDown();
// void moveArrowSelectionUp();
// void moveBoxSelection(u_int16 direction);
// u_int16 navigateMenus();
// char *retrieveSongLocation(u_int16 *index, u_int16 offset);

ioresult UI_init();
void loadHeader();
void loadMainMenu();
void loadNowPlaying();
void displayBatteryPercentage(u_int16 battery_level);
void displaySongPlaybackBar(u_int16 elapsed_time, u_int16 song_length);


void loadCriticalErrorMenu();


#endif  // _UI_H_
