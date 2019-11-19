#include <vo_stdio.h>
#include <lcd.h>
#include <vo_gpio.h>
#include <string.h>
#include <vo_fat.h>
#include <sysmemory.h>
#include <vo_fat.h>
#include <vo_fatdirops.h>
#include <libaudiodec.h>

#include "UI.h"
#include "lcd-ili9341.h"

#define FILE_NAME_CHARS 256

extern volatile int fileNum;
extern u_int16 currentMenu;
extern s_int16 arrowSelection;
extern FILE *fp;
extern FILE *sampleFile;
extern u_int16 running;
extern u_int16 newSongSelected;
extern AUDIO_DECODER *audioDecoder;
extern char *filespec;
extern char *path;
extern u_int16 index;
extern SongInfo *metadata;
extern u_int16 idleMode;
extern u_int16 anyButtonPressed;

FILE *filehandle = NULL;
s_int16 endOfFile = -1;
u_int16 pageNum = 0;
static u_int16 lastTimeSeconds = 0;
char buffer[128];
u_int32 topOfList = 0;
static char currentTitle[31];


void loadMainMenu()
{
	LcdInit(1);
	// monitorVoltage();
	LcdTextOutXY(1,1, "MAIN MENU");
	lcd0.textColor = RED;
	LcdTextOutXY(1,50, "ARTISTS");
	LcdTextOutXY(1,300, "PLAYLISTS");
	LcdTextOutXY(200,50, "SONGS");
	LcdTextOutXY(200,300, "INFO");
	lcd0.textColor = WHITE;
	currentMenu = MAIN_MENU;
}

// Used across multiple menus by specifying the title being drawn
// void drawCommonMenuItems(char *title, u_int16 menu)
// {
// 	LcdInit(1);
// 	monitorVoltage();
// 	LcdTextOutXY(1,1, title);
// 	// Change font to red so it stands out
// 	lcd0.textColor = RED;
// 	//Draw common buttons and title to screen
// 	LcdTextOutXY(200,20, "BACK");

// 	// The info menu doesn't have options for navigation, so lets hide them
// 	if (menu != INFO_MENU)
// 	{
// 		LcdTextOutXY(10,20, "UP");
// 		LcdTextOutXY(10,310, "DOWN");
// 		LcdTextOutXY(190,310, "SELECT");
// 		// Done with red, change font to yellow for the box
// 		lcd0.textColor = YELLOW;
// 		// Draw the box around the first entry
// 		LcdTextOutXY(10, 30,  "--------------------------------");
// 		LcdTextOutXY(7, 40,  "|");
// 		LcdTextOutXY(230, 40,  "|");
// 		LcdTextOutXY(10, 50,  "--------------------------------");
// 		// Change the color back to white for any other function that draws
// 		lcd0.textColor = WHITE;
// 		pageNum = 0;
// 		arrowSelection = 0;
// 	}

// 	currentMenu = menu;
// }

// void drawMenuListOptions()
// {
// 	u_int16 i;
// 	char *entry;

// 	// Clear all options on the screen
// 	for (i = 0; i < 9; i++)
// 	{
// 	    LcdTextOutXY(20, 40 + (i * 30), "                              ");
// 	}

// 	// point to start of file
// 	fseek(filehandle, 0, SEEK_SET);

// 	// Skip lines that we're not interested in right now
// 	for (i = 0; i < pageNum * 9; i++)
// 	{
// 	    fgets(buffer, sizeof(buffer), filehandle);
// 	    //printf("skipped buffer: %s", buffer);
// 	}

// 	topOfList = ftell(filehandle);
// 	//fgets(buffer, sizeof(buffer), filehandle);
// 	//printf("Top of List buffer : %s", buffer);
// 	//fseek(filehandle, topOfList, SEEK_SET);

// 	// Grab the first 10 lines of data. 9 will be displayed, and the 10th will
// 	// be used to decide if reaching the end of the list should allow a new
// 	// list to be generated or not
// 	for (i = 0; i <= 9; i++)
// 	{
// 	    // First 9 to draw
// 	    if (i < 9)
// 	    {
// 	        // If the line exists, draw the line to the screen
// 	        if (fgets(buffer, sizeof(buffer), filehandle))
// 	        {
// 	        	 // strtok reads a buffer until a certain character is found as a delimitter
// 	        	 // in our case, "|" will be our delimitter
// 	        	 // read the first entry into the table, which will be the file name
// 	        	 // FILENAME | PATH | INDEX
// 	            entry = strtok(buffer, "|");
// 	            LcdTextOutXY(20, 40 + (i * 30), entry);
// 	        }
// 	        // The line didn't exist, mark the end of file line to know our
// 	        // box movement boundaries. No reason checking additional lines,
// 	        // so break from the loop now
// 	        else
// 	        {
// 	            endOfFile = i-1;
// 	            break;
// 	        }
// 	    }
// 	    // Last check to determine if there is another list after
// 	    else
// 	    {
// 	        // Not drawing to screen, just checking if it exists. If it doesn't,
// 	        // set end of file appropriately
// 	        if (!fgets(buffer, sizeof(buffer), filehandle))
// 	        {
// 	            endOfFile = i-1;
// 	        }
// 	        // There is another menu, so set end of file to value that will not stop
// 	        // us from navigating to it
// 	        else
// 	        {
// 	        	endOfFile = -1;
// 			 }
// 	    }
// 	}
// 	// move the pointer back to the top of the list for easier callback on selection
// 	fseek(filehandle, topOfList, SEEK_SET);
// 	fgets(buffer, sizeof(buffer), filehandle);
// 	printf("Top of List buffer : %s", buffer);
// 	fseek(filehandle, topOfList, SEEK_SET);
// }

// void loadArtistsMenu()
// {
// 	drawCommonMenuItems("ARTISTS MENU", ARTISTS_MENU);

// 	if (filehandle)
// 	{
// 		fclose(filehandle);
// 	}

// 	filehandle = fopen("S:LOOKUP/songs.txt", "rb");

// 	// If file was succesfully opened to be read
// 	if (filehandle)
// 	{
// 		drawMenuListOptions();
// 	}
// 	else
// 	{
// 		printf("failed to open file \n");
// 	}
// }

// void loadPlaylistsMenu()
// {
// 	drawCommonMenuItems("PLAYLIST MENU", PLAYLISTS_MENU);

// 	if (filehandle)
// 	{
// 		fclose(filehandle);
// 	}

// 	filehandle = fopen("S:LOOKUP/songs.txt", "rb");

// 	// If file was succesfully opened to be read
// 	if (filehandle)
// 	{
// 		drawMenuListOptions();
// 	}
// 	else
// 	{
// 		printf("failed to open file \n");
// 	}
// }

// void loadSongsMenu()
// {
// 	drawCommonMenuItems("SONGS MENU", SONGS_MENU);

// 	if (filehandle)
// 	{
// 		fclose(filehandle);
// 	}
// 	filehandle = fopen("S:LOOKUP/songs.txt", "rb");

// 	// If file was succesfully opened to be read
// 	if (filehandle)
// 	{
// 		drawMenuListOptions();
// 	}
// 	else
// 	{
// 		printf("failed to open file \n");
// 	}
// }

// void loadScanningMenu()
// {
// 	LcdInit(1);
// 	// Change font to red so it stands out
// 	lcd0.textColor = RED;
// 	LcdTextOutXY(10, 100, "Scanning disk for songs...");
// 	lcd0.textColor = WHITE;
// 	currentMenu = LOAD_MENU;
// }

void loadCriticalErrorMenu()
{
	LcdInit(1);
	// Change font to red so it stands out
	lcd0.textColor = RED;
	LcdTextOutXY(10, 20, "A critical error has occured!");
	LcdTextOutXY(10, 40, "Please try restarting the system");
	LcdTextOutXY(10, 60, "If the error persists, contact UML");
	lcd0.textColor = WHITE;
	currentMenu = ERR_MENU;
}

// void loadInfoMenu()
// {
// 	drawCommonMenuItems("INFO MENU", INFO_MENU);

// 	LcdTextOutXY(10, 100, (char *) metadata->title);
// 	LcdTextOutXY(10, 130, (char *) metadata->artist);
// 	LcdTextOutXY(10, 160, (char *) metadata->album);

// 	strcpy(currentTitle, (char *) metadata->title);
// }

void moveBoxSelection(u_int16 direction)
{
	u_int16 i;

	// Clear the current box from the screen by just drawing over it with blank spaces
	LcdTextOutXY(10, 30 + (arrowSelection * 30),  "                                ");
	LcdTextOutXY(7, 40 + (arrowSelection * 30),  " ");
	LcdTextOutXY(230, 40 + (arrowSelection * 30),  " ");
	LcdTextOutXY(10, 50 + (arrowSelection * 30),  "                                ");

	// User wants to move the selection up
	if (direction == UP)
	{
		// If the user is already at the top of the list, determine if there is a previous
		// list to navigate to or not
		if (arrowSelection == 0)
		{
			// list page number is not 0, so there is a previous list to navigate to
			if (!pageNum == 0)
			{
				// decrement list since we're navigating back
				pageNum--;
				// move the box to the last option on the previous list
				arrowSelection = 8;
				// redraw the options to the screen
				drawMenuListOptions();
			}
		}
		// We're not on the first option, so we can just decrement the counter by one and be done
		else
		{
			arrowSelection--;
		}
	}
	// User wants to move the selection down
	else
	{
		// If end of file was set, then we don't want to allow the user to move beyond the last
		// populated option
		if (arrowSelection != endOfFile)
		{
			// Moving down on last option requires a redraw of the screen options
			if (arrowSelection == 8)
			{
				// increment page number since we're moving to a new page
				pageNum++;
				// reset the box to the top of the list
				arrowSelection = 0;

				drawMenuListOptions();
			}
			else
			{
				arrowSelection++;
			}
		}
	}

	// Draw the box at the new determined position
	lcd0.textColor = YELLOW;
	LcdTextOutXY(10, 30 + (arrowSelection * 30),  "--------------------------------");
	LcdTextOutXY(7, 40 + (arrowSelection * 30),  "|");
	LcdTextOutXY(230, 40 + (arrowSelection * 30),  "|");
	LcdTextOutXY(10, 50 + (arrowSelection * 30),  "--------------------------------");
	lcd0.textColor = WHITE;
}

// char *retrieveSongLocation(u_int16 *index, u_int16 offset)
// {
// 	u_int16 i;
// 	char *charIndex;

// 	// Skip lines that we're not interested in right now
// 	for (i = 0; i < arrowSelection + offset; i++)
// 	{
// 		// We want to make sure that the line that is being read actually exists
// 		// This should always be the case when a user selects an entry, but when
// 		// the next song is chosen automatically when a song ends, that might not be the case
// 		// so, check if it exists and if it doesn't, we need to point back to the top of the
// 		// list and then start reading lines
// 	    if (!fgets(buffer, sizeof(buffer), filehandle))
// 	    {
// 	    	fseek(filehandle, 0, SEEK_SET);
// 	    	fgets(buffer, sizeof(buffer), filehandle);
// 		 }
// 	    //printf("Discarded buffers : %s \n", buffer);
// 	}

// 	// Now fill the buffer with the line of interest
// 	fgets(buffer, sizeof(buffer), filehandle);
// 	//printf("Used buffer : %s \n", buffer);

// 	// We're done grabbing the line of interest, move pointer back to top of list
// 	// for most convenience on future selections
// 	fseek(filehandle, topOfList, SEEK_SET);

// 	// strtok will split the buffer string into the fields that we are interested in
// 	// The first field is the title, which we don't need for locating the song on the disk
// 	// so we're going to have it skipped by calling the function and not using it
// 	strtok(buffer, "|");
// 	//printf("Title : %s \n", buffer);

// 	// Now the next column is the index number. It'll be returned as a char, so we need to subtract
// 	// '0' from it to convert it to the appropriate integer
// 	charIndex = strtok(NULL, "|");
// 	//printf("Index char : %s \n", charIndex);
// 	*index = atoi(charIndex);
// 	//printf("Index int : %i \n", index);

// 	// If a song is selected and something is currently playing, we want to set newSongSelected as
// 	// an interrupt. Otherwise, we want to start running the system
// 	if (running)
// 	{
// 		newSongSelected = 1;
// 	}
// 	else
// 	{
// 		running = 1;
// 	}

// 	// The last field we need is the path. This will just be returned and used by the calling function
// 	// directly
// 	return strtok(NULL, "|");
// }

// void loadNoDriveMenu()
// {
// 	LcdInit(1);
// 	// Change font to red so it stands out
// 	lcd0.textColor = RED;
// 	LcdTextOutXY(1, 70, "No Drive Detected!");
// 	LcdTextOutXY(1, 90, "Please try restarting the system");
// 	LcdTextOutXY(1, 110, "Make sure the drive is plugged in");
// 	LcdTextOutXY(1, 130, "and is formatted as FAT");
// 	LcdTextOutXY(1, 150, "If the error persists, contact UML");
// 	lcd0.textColor = WHITE;
// 	currentMenu = ERR_MENU;
// }

// u_int16 navigateMenus()
// {
// 	switch (currentMenu)
// 	{
// 		case MAIN_MENU :
// 			if (GpioReadPin(BUTTON10))
// 			{
// 				loadPlaylistsMenu();
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON10)) {}
// 				printf("BUTTON10 \n");
// 				return 1;
// 			}
// 			else if (GpioReadPin(BUTTON11))
// 			{
// 				loadSongsMenu();
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON11)) {}
// 				printf("BUTTON11 \n");
// 				return 1;
// 			}
// 			else if (GpioReadPin(BUTTON12))
// 			{
// 				loadArtistsMenu();
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON12)) {}
// 				printf("BUTTON12 \n");
// 				return 1;
// 			}
// 			else if (GpioReadPin(BUTTON13))
// 			{
// 				loadInfoMenu();
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON13)) {}
// 				printf("BUTTON13 \n");
// 				return 1;
// 			}
// 			break;
// 		case INFO_MENU :
// 			if (lastTimeSeconds != audioDecoder->cs.playTimeSeconds)
// 			{
// 				lastTimeSeconds = (int) audioDecoder->cs.playTimeSeconds;
// 				if (lastTimeSeconds < 99*60)
// 				{

// 				}
// 			}
// 			if (GpioReadPin(BUTTON12))
// 			{
// 				loadMainMenu();
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON12)) {}
// 				printf("BUTTON12 \n");
// 				return 1;
// 			}
// 			break;
// 		case ARTISTS_MENU :
// 			if (GpioReadPin(BUTTON10))
// 			{
// 				moveBoxSelection(DOWN);
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON10)) {}
// 				printf("BUTTON10 \n");
// 				return 1;
// 			}

// 			if (GpioReadPin(BUTTON11))
// 			{
// 				moveBoxSelection(UP);
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON11)) {}
// 				printf("BUTTON11 \n");
// 				return 1;
// 			}

// 			if (GpioReadPin(BUTTON12))
// 			{
// 				loadMainMenu();
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON12)) {}
// 				printf("BUTTON12 \n");
// 				return 1;
// 			}
// 			break;

// 		case SONGS_MENU :
// 			if (GpioReadPin(BUTTON10))
// 			{
// 				moveBoxSelection(DOWN);
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON10)) {}
// 				printf("BUTTON10 \n");
// 				return 1;
// 			}

// 			else if (GpioReadPin(BUTTON11))
// 			{
// 				moveBoxSelection(UP);
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON11)) {}
// 				printf("BUTTON11 \n");
// 				return 1;
// 			}

// 			else if (GpioReadPin(BUTTON12))
// 			{
// 				loadMainMenu();
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON12)) {}
// 				printf("BUTTON12 \n");
// 				return 1;
// 			}

// 			else if (GpioReadPin(BUTTON13))
// 			{
// 				path = retrieveSongLocation(&index, 0);
// 				printf("%s - %i \n", path, index);
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON13)) {}
// 				printf("BUTTON13 \n");
// 				return 1;
// 			}
// 			break;

// 		case PLAYLISTS_MENU :
// 			if (GpioReadPin(BUTTON12))
// 			{
// 				loadMainMenu();
// 				anyButtonPressed = 1;
// 				idleMode = 0;
// 				while (GpioReadPin(BUTTON12)) {}
// 				printf("BUTTON12 \n");
// 				return 1;
// 			}
// 			break;

// 		default :
// 			break;
// 	}

// 	return 0;
// }
