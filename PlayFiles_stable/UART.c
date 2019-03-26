/*************************/
/* INTERNAL HEADER FILES */
/*************************/
#include <vo_stdio.h>
#include <libaudiodec.h>
#include <aucommon.h>
#include <ctype.h>
#include <uimessages.h>
#include <lcd.h>

/*************************/
/* EXTERNAL HEADER FILES */
/*************************/
#include "UI.h"
#include "battery.h"


/********************/
/* EXTERN VARIABLES */
/********************/
extern FILE *filehandle;
extern u_int16 currentMenu;
extern u_int16 repeatSongStatus;
extern u_int16 shuffle;
extern s_int16 volume;
extern u_int16 mute;
extern volatile int fileNum;
extern int running;
extern AUDIO_DECODER *audioDecoder;
extern u_int16 charging;
extern u_int16 index;
extern char *path;
extern u_int16 anyButtonPressed;
extern u_int16 idleMode;
extern u_int16 cbut;
extern u_int16 auxChoice;

/*************/
/* FUNCTIONS */
/*************/


// Generic function to accept UART commands from the terminal
// for debugging and information purposes. Very helpful in software development.
// Once a solid driver is designed, it may be commented out to save CPU cycles,
// power, and memory space
u_int16 checkUART()
{	
	// Check for UART control
	if (ioctl(stdin,IOCTL_TEST,NULL) > 0) 
	{
		char c = fgetc(stdin);
		
	   if (c == 'h')
	   {
	       puts("'C' = Toggle capacitive button output to UART");
	       puts("'>' = Play next song");
	       puts("'<' = Play previous song");
			puts("'+' = Volume up");
			puts("'-' = Volume down");
			puts("'R' = Toggle repeat song mode");
			puts("'P' = Toggle Play/Pause");
			puts("'i' = Press top left screen button");
			puts("'k' = Press bottom left screen button");
			puts("'o' = Press top right screen button");
			puts("'l' = Press bottom right screen button");
			puts("'s' = Prints various variable statuses");
			puts("'A' = Toggle which analog pin is read for battery percentage");
			puts("'h' = Prints this help menu");
			puts("'0-9' = sets the song index to the chosen number");
			return 1;
	   }
	   
		if (c == 'Q') 
		{ // Q Quit
			audioDecoder->cs.cancel = 1;
			running = 0;
			return 1;
		}
		
		
	   if (c == 'b')
	   {
	   		monitorVoltage();
	   		return 4;
		}		
		
		if (c == 'C')
		{
			cbut = !cbut;
			return 5;
		}

		if (c == '>') 
		{ // > Next
			repeatSongStatus = 0;
			audioDecoder->cs.cancel = 1;
			return 2;
		}
		
		if (c == '<') 
		{ // < Previous
			repeatSongStatus = 0;
			audioDecoder->cs.cancel = 1;
			if (audioDecoder->cs.playTimeSeconds < 3)
			{
				fileNum -= 2;
			}
			else
			{
				fileNum -= 1;
			}
	
			if (fileNum < -1) 
			{
				fileNum = -1;
			}
				return 3;
		}
		
		if (c == '+') 
		{ // volume up
		
			if (volume > 0)
			{
				volume = volume - 6;
			}
		
			ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));
			printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, 30-volume/6);
			anyButtonPressed = 1;
			idleMode = 0;
			return 4;
		}
		if (c == '-') 
		{ // volume down
		
			if (volume < 180)
			{
				volume = volume + 6;
			}
		
			ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));
			printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, 30-volume/6);
			anyButtonPressed = 1;
			idleMode = 0;
			return 5;
		}
			
		if (c == 'R')
		{
			repeatSongStatus = !repeatSongStatus;
			if (repeatSongStatus == 0)
			{
				printf("Repeat : off");
			}
			else
			{
				printf("Repeat : on");
			}
			return 6;
		}
		
		if (c == 'P')
		{
			audioDecoder->pause = !audioDecoder->pause;
			if (audioDecoder->pause) 
			{
				printf("PAUSING \n");
			}
			else 
			{
				printf("PLAYING \n");
			}
			return 9;
		}
		

		if (c == 'i')
		{
			anyButtonPressed = 1;
			idleMode = 0;
			switch (currentMenu)
			{
				case MAIN_MENU :
					loadSongsMenu();
					break;
				case INFO_MENU :
					break;
				case ARTISTS_MENU :
					moveBoxSelection(UP);
					break;
				case SONGS_MENU :
					moveBoxSelection(UP);
					break;
				case PLAYLISTS_MENU :
					moveBoxSelection(UP);
					break;
				default :
					break;
			}
			return 14;
		}
		
		if (c == 'k')
		{
			anyButtonPressed = 1;
			idleMode = 0;
			switch (currentMenu)
			{
				case MAIN_MENU :
					loadPlaylistsMenu();
					break;
				case INFO_MENU :
					break;
				case ARTISTS_MENU :
					moveBoxSelection(DOWN);
					break;
				case SONGS_MENU :
					moveBoxSelection(DOWN);
					break;
				case PLAYLISTS_MENU :
					moveBoxSelection(DOWN);
					break;
				default :
					break;
			}
			return 13;
		}
		
		if (c == 'o')
		{
			anyButtonPressed = 1;
			idleMode = 0;
			switch (currentMenu)
			{
				case MAIN_MENU :
				
					loadArtistsMenu();
					break;
				case INFO_MENU :
					
					loadMainMenu();
					break;
				case ARTISTS_MENU :
					
					loadMainMenu();
					//fclose(filehandle);
					break;
				case SONGS_MENU :
					
					loadMainMenu();
					//fclose(filehandle);
					break;
				case PLAYLISTS_MENU :
					loadMainMenu();
					break;
				default :
					break;
			}
			return 12;
		}
		
		if (c == 'l')
		{
			anyButtonPressed = 1;
			idleMode = 0;
			switch (currentMenu)
			{
				case MAIN_MENU :
					loadInfoMenu();
					break;
				case SONGS_MENU :
					path = retrieveSongLocation(&index, 0);
					printf("%s - %i \n", path, index);
					break;
				default :
					break;
			}
			return 11;
		}
		
		if (c == 's')
		{
			printf("Charging:  %i \n", charging);
			printf("Volume:    %i \n", volume);
			printf("Shuffling: %i \n", shuffle);
			printf("Repeating: %i \n", repeatSongStatus);
			printf("Mute:      %i \n", mute);	
			return 12;		
		}
		
		if (isdigit(c)) 
		{ // number:select file
			fileNum = c-'0';
			audioDecoder->cs.cancel = 1;		
			return 10;	
		}
		
	   if (c == 'A')
	   {
	      auxChoice = !auxChoice;
	      if (auxChoice)
	      {
	      	puts("auxChoice = AUX2");
		   }
		   else
		   {
		    puts("auxchoice = AUX3");
		   }
		   return 11;
	   }
	}
	return 0;
}