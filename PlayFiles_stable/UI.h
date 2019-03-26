#ifndef UI_H
#define UI_H

#define MAIN_MENU      (1)
#define ARTISTS_MENU   (2)
#define PLAYLISTS_MENU (3)
#define SONGS_MENU     (4)
#define INFO_MENU      (5)
#define LOAD_MENU      (6)
#define ERR_MENU		  (7)

#define UP             (1)
#define DOWN           (0)

#define TITLE			  (0)
#define LOCATION       (1)

void loadCriticalErrorMenu();
void loadScanningMenu();
void loadMainMenu();
void loadArtistsMenu();
void loadPlaylistsMenu();
void loadSongsMenu();
void loadInfoMenu();
void loadNoDriveMenu();
void moveArrowSelectionDown();
void moveArrowSelectionUp();
void moveBoxSelection(u_int16 direction);
u_int16 navigateMenus();
char *retrieveSongLocation(u_int16 *index, u_int16 offset);

#endif