#include <vo_stdio.h>
#include <volink.h>                 // Linker directives like DLLENTRY
#include <apploader.h>              // RunLibraryFunction etc
#include <libaudiodec.h>
#include <vsostasks.h>
#include <consolestate.h>
#include <ctype.h>
#include <kernel.h>
#include <cyclic.h>
#include <uimessages.h>         // Volume control and reporting
#include <aucommon.h>
#include <lcd.h>
#include <vstypes.h>
#include <string.h>
#include <vsos.h>
#include <string.h>
#include <unistd.h>
#include <vo_fat.h>
#include <vo_fatdirops.h>
#include <sysmemory.h>

#define FILE_NAME_CHARS 128
char fileName[FILE_NAME_CHARS]="";

#include "taskandstack.h"
#include "buttons.h"
#include "UI.h"
#include "battery.h"


#define USE_UART (1)

#include "UART.h"
#include "ID3v2.h"

void PlayerThread(void);
ioresult PlayFile(FILE *fp);
u_int16 decoderTaskRunning();
u_int16 checkFlags();
u_int16 recursiveFindMP3(char *path, u_int16 root);
u_int16 randLFSR(u_int16 seed);
u_int16 reloop(char *text);
u_int16 scanDirectoryForMP3(u_int16 root);
char *scanDirectoryForDirectory(char *directory, u_int16 dirIndex);
void writeToFile(char *FileName, u_int16 fileIndex, char *mp3Location, char *songName);

FILE *fp = NULL;
FILE *openedFile = NULL;
void *decoderLibrary = NULL;
AUDIO_DECODER *audioDecoder = NULL;
char *errorString = "";
u_int16 eCode = 0;
u_int16 running = 0;
u_int16 mute = 0;
s_int16 volume = 90; // 30 - (90/6) = volume 15 to start
s_int16 arrowSelection = 0;
u_int16 currentMenu = 0;
u_int16 shuffle = 0;
u_int16 repeatSongStatus = 0;
u_int16 anyButtonPressed;
u_int16 idleMode;
u_int16 charging = 0;
struct TaskAndStack *voltageTask = NULL;
struct TaskAndStack *interfaceTask = NULL;
struct TaskAndStack *powerSavingTask = NULL;
struct CyclicNode myCyclicNode = {{0}, (void *) monitorVoltage}; // not used anywhere?
SongInfo *metadata;
u_int16 seed;
u_int16 numSongsDirectory = 10;
u_int16 newSongSelected = 0;
char *path;
u_int16 index;
char *extension = "*.mp3";
static char directory[64];
static char nextDirectory[64];	
static char tempDirectory[64];	
u_int16 voltageChecked;	
s_int16 offset;
u_int16 totalNumSongs;
u_int16 cbut = 1;
u_int16 auxChoice = 1;


#define DEBUG_MODE (0)

char *filespec = "*.mp3";
u_int16 fileNum = 0;

int main(char *params) 
{
    static char fnumMode[10];
    int number_of_mp3;
    int number_of_dir;
    u_int16 counter = 0;
    char *nextDir;

    //get lots of kernel debug output to stderr (UART)
    //kernelDebugLevel = 99;

    // Powers and enables the SAR
    PERIP(ANA_CF1) |= ANA_CF1_SAR_ENA; 
    // Sets the volume to 15 - a reasonable volume to start at
    ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));
    
	LcdInit(0);
   
	// We need to check if the flash drive is actually plugged in and make sure
	// that the flash drive is formatted to the correct format (FAT)
	if (!(vo_pdevices[5] && vo_pdevices[5]->fs == vo_filesystems[0]))
	{
		loadNoDriveMenu();
		return S_ERROR;
	}
	
	 // Some time is going to be spent scanning, let the user know
    loadScanningMenu();
    printf("loaded scan \n");
    
    // We want to delete and remake the lookup directory each time the 
    // system is booted in case any artists were added or deleted
    // This isn't necessary for the master song list since it is re-written
    // every time either way
    //RunProgram("DEL", "S:lookup/artists/");
    //mkdir("S:lookup/artists");
    
    // For some reason, sizeof(struct SongInfo) causes an error. Instead,
    // we can just assign the exact space needed for each unsigned char within
    // the structure. So, 31+31+31+31+5+2=131. We also need to get the size of the
    // file pointer to be included too since it will be assigned to the struct.
    // But, some compilers like to keep the allocated space a multiple of 32 bits, 
    // so we round up to the next multiple of 4, ie. 132 bytes and take the modulus
    // of fp to determine if it needs rounding
    metadata = malloc((sizeof(unsigned char)*132)+(sizeof(FILE*)) + (sizeof(FILE*)%4));

	printf("alloc \n");
    // check if the memory was allocated for metadata storage. The entire system design relies
    // on this to work, so if it wasn't allocated, nothing can be done and we should exit
    if (!metadata)
    {
        // We want to inform the user of the problem, so lets display an error message screen
        // so that they are aware and can reset the system to try to fix the issue.
        printf("Metadata space not allocated! \n");
        loadCriticalErrorMenu();
        return S_ERROR;
    } 

	printf("error check \n");
    // open the file we will be storing the looked up music in
   openedFile = fopen("S:lookup/songs.txt", "w");
   if (!openedFile)
   {
		 printf("Master Song list not created! \n");
        loadCriticalErrorMenu();
        return S_ERROR;
	}
	
   printf("file \n");
    sprintf(directory, "F:");
    sprintf(currentDirectory, "F:");
    //sprintf(currentDirectory, "F:ISEEST~1/");
    // FIXME
    
    //recursiveFindMP3(currentDirectory, 1);
    
    

	// For some reason, the recursive searching function that we wrote isn't actually working
	// Instead, this function and the scanDirectoryForDirectory function were written as a
	// temporary workaround. This code is only capable of checking the root directory of the
	// flash drive and then one directory deeper, which may not be how a user sets up their
	// music on the flash drive. Fixing the other function would resolve this problem
	printf("before scan \n");
    scanDirectoryForMP3(1);
    
    do
    {
        nextDir = scanDirectoryForDirectory("F:", counter);
        //printf("[%s] nextDir \n", nextDir);
        if (strcmp(nextDir, "no"))
        {
            sprintf(directory, "F:%s", nextDir);
            //printf("[%s] directory \n", directory);
            scanDirectoryForMP3(0);
            counter++;
        }
    } while (strcmp(nextDir, "no")); 
    
	printf("end of scan \n");  
    
    // done adding to this file, close it
    fclose(openedFile);

   // Create a task to monitor the battery voltage
   voltageTask = CreateTaskAndStack(cyclicVoltage, "cyclicVoltage", 128, 5);
   // Create a task for the menus so that it operates independently of music playing
   // interfaceTask = CreateTaskAndStack(navigateMenus, "navigateMenus", 256, 5);
   powerSavingTask = CreateTaskAndStack(idleCheck, "idleCheck", 128, 5);
   

    decoderLibrary = LoadLibrary("audiodec");
    
    if (!decoderLibrary) 
    {
        loadCriticalErrorMenu();
        return S_ERROR;
    }

    fileNum = 0;
    
    loadMainMenu();
    // Read the initial voltage value
    monitorVoltage();

    // Once everything has been set up appropriately, we want to indefinitely run this loop
    // to make sure that the system is constantly functioning
    while (1)
    {
        // Power saving function that checks if idleMode is set and if it is, adds a delay
        // before executing other code. This delay will decrease CPU run time, essentially
        // saving power.
        powerSavingDelay();
    
        // Not currently playing a song, but we need to allow menu navigation still so that
        // a song can be chosen to be played
        navigateMenus();
        
         // Navigation of menus is also possible through the UART, so this can be enabled for
         // testing purposes
        #if USE_UART
            checkUART();
        #endif

        // Check to see if the system was set to run because of an entry being chosen
        if (running)
        {
            // Grab the entry info to find the song that was requested
            fileNum = index;
            sprintf(filespec, "%s*.mp3", path);
            sprintf(fnumMode, "rb#%d", fileNum);

            // Error checking before continuing
            if (appFlags & APP_FLAG_QUIT) 
            {
                loadCriticalErrorMenu();
                DropLibrary(decoderLibrary); 
                free(metadata);
                break;
            }

            // Now that we are in a running mode, we stay in a running mode until a STOP occurs
            while (running) 
            {               
                #if DEBUG_MODE
                    printf("Open file %d...\n",fileNum+1);
                #endif
        
                sprintf(fnumMode,"rb#%d",fileNum);
                fp = fopen(filespec,fnumMode); // e.g. fopen("N:MUSIC\*","rb#123");
                
                if (fp) 
                {   
                
                    #if DEBUG_MODE
                        // this Identify is good to be right after the fopen
                        printf("Playing '%s' ", fp->Identify(fp, NULL, 0));
                        printf("from device %s...\n",fp->dev->Identify(fp->dev,NULL,0));
                    #endif
                    
                    // Now assign the file pointer to the struct
                    metadata->fp = fp;
        
                    // Run the metadata driver to retrieve song information
                    RunLibraryFunction("MYMETA",ENTRY_1,(int) &metadata);
                    
                    #if DEBUG_MODE
                        printf("Title : %s \n", metadata->title);
                        printf("Artist : %s \n", metadata->artist);
                        printf("Album : %s \n", metadata->album);
                    #endif
                    
                    PlayFile(fp);
                    
                    // Once the song has started, loop through these checks until something causes an exit
                    while (decoderTaskRunning())
                    {
                        // We need the power delay feature here too since this is another while loop that may
                        // occur for a long time
                        powerSavingDelay();
                        
                        if      (checkFlags())                      {}
                        else if (newSongSelected)                   {audioDecoder->cs.cancel = 1;}
                        #if USE_UART
								else if (checkUART())                       {}
						   #endif
                        else if (navigateMenus())                   {}
                        else if (playPause(BUTTON1, ACTIVEHIGH))    {}
                        else if (nextSong(BUTTON2, ACTIVEHIGH))     {}
                        else if (previousSong(BUTTON3, ACTIVEHIGH)) {}
                        else if (volumeUp(BUTTON4, ACTIVEHIGH))     {}
                        else if (volumeDown(BUTTON5, ACTIVEHIGH))   {}
                        else if (volumeMute(BUTTON6, ACTIVEHIGH))   {}
                        else if (repeatSong(BUTTON7, ACTIVEHIGH))   {}
                        else if (stopSong(BUTTON8, ACTIVEHIGH))     {}
                        else if (shuffleSong(BUTTON9, ACTIVEHIGH))  {}
                        
                        // Capacitive buttons are active low and since there aren't enough features
                        // currently, they are on repeated functionalities just for demo purposes
                        #if 1
                            //else if (!GpioReadPin(CBUTTON1))       {printf("cap1 button pressed! \n");}
                            //else if (!GpioReadPin(CBUTTON2))    {printf("cap2 button pressed! \n");}
                            else if (!GpioReadPin(CBUTTON3) && cbut)        {printf("cap3 button pressed! \n"); Delay(1000);}
                        #endif
                    }
                }
                // This should only occur if repeat/shuffle mode were not set and 
                // a new song wasn't selected so the filenum was incremented but
                // the number was greater than the number of songs in the directory
                // So in this case we want to just reset back to the first song
                else 
                {
                    printf("File %d not found, finished playing.\n",fileNum+1);
                    // Restart from first file
                    fileNum = 0;
                    continue;
                }
                
                // Always clean up filehandles and decoder before playing a new song
                DeleteAudioDecoder(decoderLibrary, audioDecoder);
                fclose(fp);
                    
                // A song was selected while the system was running, set the request as this info
                // for the next loop
                if (newSongSelected)
                {
                	  offset = 0;
                    fileNum = index;
                    sprintf(filespec, "%s*.mp3", path);
                    sprintf(fnumMode, "rb#%d", fileNum);
                    newSongSelected = 0;
                }
                // if shuffle is on, we want to generate a random file index 
                // of the current song directory we're looking at
                // NOTE: If shuffle is on, it always has precedence over repeat mode
                else if (shuffle)
                {
                    // generating a random number might result in the same index being 
                    // chosen, but we don't want the same song to play, so until a new
                    // number is generated from the LFSR, keep looping.
                    // if the number of songs in the directory is only 1 though,
                    // it is impossible to generate a new number, so just play the same
                    // song again.
                    // FIXME : This is not correct, and should be modified to the commented
                    //         code below.
                    do
                    {
                        seed = randLFSR(seed);
                    } while (fileNum == seed % numSongsDirectory);
                    fileNum = seed % numSongsDirectory;


                    // FIXME : Shuffle needs additional work to generate a proper randomly
                    //         generated choice. The number of songs for the current source
                    //         needs to be referenced and then a number generated to pick
                    //         a random song from that source.
                    
					  /*
					  if (currentMenu == SONGS_MENU)
					  {
                      do
                      {
                         seed = randLFSR(seed);
                      } while (offset == seed % numSongsDirectory * (-1 * (offset >> 15)) & numSongsDirectory != 1);
                      // This loop makes sure that the same song isn't chosen from the random number generation
                      // If there is only one song to look at though, then it exits after the first run.
                      // Whatever offset that is produced
					  }
					  else if (currentMenu == ARTISTS_MENU)
					  {
					     do
                      {
                         seed = randLFSR(seed);
                      } while (offset == (seed % totalNumSongs) * (-1 * (offset >> 15)) && totalNumSongs != 1);
					  }
					  */

                    //offset = seed % numSongsDirectory;
                    //path = retrieveSongLocation(&index, offset);
                    //fileNum = index;
                    //sprintf(filespec, "%s*.mp3", path);
                    //sprintf(fnumMode, "rb#%d", fileNum);
                    //newSongSelected = 0;
                }
                // FIXME :
                // incrementing a song should reference the current menu being used, not just a 
                // file number. Incrementing a file number will change the song to a different song within
                // the currently set directory, but that may not be the next song on the list. If there is a 
                // single song in a list, then it will continuously play that song even though others may
                // show up within the songs menu. This is currently functional, yet not ideal or really
                // practical, so it should be changed in the future.
                else if (!repeatSongStatus)
                {
                	   //path = retrieveSongLocation(&index, offset++);
                    //fileNum = index;
                    //sprintf(filespec, "%s*.mp3", path);
                    //sprintf(fnumMode, "rb#%d", fileNum);
                    //newSongSelected = 0;
                    fileNum++;
                }
            }
        }
    }

    // We should never get here unless an error occured that caused us to break out
    // Clean up any allocated memory before returning
    DropLibrary(decoderLibrary); 
    free(metadata);
    return S_ERROR;
}

u_int16 randLFSR(u_int16 seed)
{
    // Get lsb (i.e., the output bit).
    u_int16 lsb = seed & 1;
    // Shift register
    seed >>= 1;             
    // Only apply toggle mask if output bit is 1.
    if (lsb == 1)            
    {
        // Apply toggle mask, value has 1 at bits corresponding to taps, 0 elsewhere
        // 1011 0100 0000 0000 = 0xB400 <- 16/14/13/11 bits are tapped for 16bit LFSR
        // http://courses.cse.tamu.edu/walker/csce680/lfsr_table.pdf
        seed ^= 0xB400u;        
    }
   return seed;     
}

void PlayerThread(void) 
{
    eCode = DecodeAudio(decoderLibrary, audioDecoder, &errorString);
}

// Generates the audioDecoder that will decode the file "fp" using the decoderLibrary,
// sending the decoded audio to stdaudiout and auto determining the format of the file specified
ioresult PlayFile(FILE *fp) 
{
    audioDecoder = CreateAudioDecoder(decoderLibrary, fp, stdaudioout, NULL, auDecFGuess);
    if (!audioDecoder) 
    {
        loadCriticalErrorMenu();
        printf("Couldn't create audio decoder\n");
        return S_ERROR;
    }

    // Start a task in the background called TASK_DECODER that runs the PlayerThread function
    StartTask(TASK_DECODER, PlayerThread);
    Delay(100); 
    
    return S_OK;
}

// Used to check whether the decoder task created is still running properly
u_int16 decoderTaskRunning()
{
    // Syntax found within VLSI FI working examples (Playfiles solutions)
    if (pSysTasks[TASK_DECODER].task.tc_State && pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) 
    {
        Delay(100);     
        return 1;
    }
    
    return 0;
}

u_int16 checkFlags()
{
    if (appFlags & APP_FLAG_QUIT) 
    {
        audioDecoder->cs.cancel = 1;
        running = 0;
        Delay(100);
        DeleteAudioDecoder(decoderLibrary, audioDecoder);
        return 1;
    }   
    
    return 0;
}

u_int16 scanDirectoryForMP3(u_int16 root)
{
    FILE *dirFile = NULL;
    FILE *metaFile = NULL;
    char filestring[32];
    u_int16 songsInDirectory = 0;
    static char filenum[10];
    
    dirFile = fopen(directory, "s");

    if (dirFile) 
    {
        // Check for file within current directory. Since we're always setting the current
        // directory before this call, there is no reason to pass a path to the function
        // because the path is always an exact match of our directory
        if (FatFindFirst(dirFile,directory,fileName,FILE_NAME_CHARS) == S_OK) 
        {  // Note: there was a random tic mark here, not sure why
            do 
            {
                // Check to see if the extension for a file is MP3. If it is, we're interested
                // in its information
                if (!strcmp( (char *) &dirFile->extraInfo[13], "MP3"))
                {
                    sprintf(filenum,"rb#%d", songsInDirectory);

                    if (root)
                    {
                        sprintf(filestring, "%s*.mp3", directory);
                    }
                    else 
                    {
                        sprintf(filestring, "%s/%s", directory, extension);
                    }

						printf("fopen(%s, %s) \n", filestring, filenum);
                    metaFile = fopen(filestring, filenum);
                    
                    // make sure file successfully opened
                    if (metaFile)
                    {
                        //printf("opened metafile pointer! \n");
                        // Now assign the file pointer to the struct
                        metadata->fp = metaFile;
                        // Retrieve metadata 
                        RunLibraryFunction("MYMETA",ENTRY_1,(int) &metadata);
                        
							#if 0
								printf("Title : %s \n", metadata->title);
                            printf("Artist : %s \n", metadata->artist);
                            printf("Album : %s \n", metadata->album);
                        #endif

                        // We want to add this information to the master file list 
                        // so that the "songs" menu is correct
                        if (!strcmp((char *) metadata->title, "UNKNOWN"))
                        {
                            strncpy((char *) metadata->title, fileName, 30);
                            metadata->title[30] = '\0';
                        }

                        if (root)
                        {
                            fprintf(openedFile, "%s|%i|%s| \r\n", (char *) metadata->title, songsInDirectory++, directory);
                        }
                        else
                        {
                            fprintf(openedFile, "%s|%i|%s/| \r\n", (char *) metadata->title, songsInDirectory++, directory);
                        }
                        fclose(metaFile);
                    }
                    else
                    {
                        printf("failed to open metafile pointer \n");
                    } 
                }
            } while (S_OK == FatFindNext(dirFile,fileName,FILE_NAME_CHARS));
        } 
        else 
        {
            printf("Path not found\n");
        }
    
        fclose(dirFile);        
        return songsInDirectory;
    }
}

char *scanDirectoryForDirectory(char *directory, u_int16 dirIndex)
{
    FILE *dirFile = NULL;
    char *currentDir;
    u_int16 counter = 0;
    
    dirFile = fopen(directory, "s");

    if (dirFile) 
    {
        // Check for file within current directory. Since we're always setting the current
        // directory before this call, there is no reason to pass a path to the function
        // because the path is always an exact match of our directory
        if (FatFindFirst(dirFile,directory,fileName,FILE_NAME_CHARS) == S_OK) 
        {
            do 
            {   
                // Check to see if the specified path is a file or a directory
                // If it is a directory, we want to recursively enter it to determine if there
                // are any songs there that can be played
                if (dirFile->ungetc_buffer & __ATTR_DIRECTORY) 
                {   
                    // There are some directory paths that are reported that are generic
                    // and should not actually be checked. So, if they occur, we just skip them
                    if (strcmp(dirFile->extraInfo, "SYSTEM~1"))
                    {
                        if (counter == dirIndex)
                        {
                            fclose(dirFile);
                            return dirFile->extraInfo;
                        }
                        counter++;
                    }
                }
            } while (S_OK == FatFindNext(dirFile,fileName,FILE_NAME_CHARS));
        } 
        else 
        {
            printf("Path not found\n");
        }
    
        fclose(dirFile);
        
        return "no";
    }
    else
    {
        printf("failed to open dirFile for some reason \n");
    }
}

u_int16 recursiveFindMP3(char *path, u_int16 root)
{
    FILE *dirFile = NULL;
    FILE *metaFile = NULL;
    char filestring[128];
    u_int16 totalNumSongs = 0;
    u_int16 songsInDirectory = 0;
    static char filenum[10];
    u_int16 previousDirLength;
    
	/*
    if (root)
    {
    	 strcpy(nextDirectory, "F:");
    }
    else
    {
    	 previousDirLength = strlen(nextDirectory);
    	 strcpy(tempDirectory, ("%s/%s", nextDirectory, directory));
    	 strcpy(nextDirectory, tempDirectory);
    }

    printf("nextDirectory - %s \n", nextDirectory); */

    dirFile = fopen(path, "s");

    if (dirFile) 
    {
    	 // Remove the drive letter from the path
        // This was copied from the "ls" driver
    	 path += 2;
    	 
        // Check for file within current directory. Since we're always setting the current
        // directory before this call, there is no reason to pass a path to the function
        // because the path is always an exact match of our directory
        if (FatFindFirst(dirFile,path,fileName,FILE_NAME_CHARS) == S_OK) 
        {
            do 
            {
                // Check to see if the specified path is a file or a directory
                // If it is a directory, we want to recursively enter it to determine if there
                // are any songs there that can be played
                if (dirFile->ungetc_buffer & __ATTR_DIRECTORY) 
                {
                    printf("[%s] [%s] \n", dirFile->extraInfo, fileName);
                    
                    // There are some directory paths that are reported that are generic
                    // and should not actually be checked. So, if they occur, we just skip them
                    if (!strcmp(dirFile->extraInfo, "SYSTEM~1") || !strcmp(dirFile->extraInfo, ".") || !strcmp(dirFile->extraInfo, ".."))
                    {
                        printf("We want to skip [%s] \n", dirFile->extraInfo);
                    }
                    else
                    {
                    		RunProgram("cd", dirFile->extraInfo);
                        printf("currentDirectory - %s \n", currentDirectory);
                        totalNumSongs += recursiveFindMP3(currentDirectory, 0);
                    }
                }
                // Check to see if the extension for a file is MP3. If it is, we're interested
                // in its information
                if (!strcmp( (char *) &dirFile->extraInfo[13], "MP3"))
                {
                    sprintf(filenum,"rb#%d", songsInDirectory);
                    if (root)
                    {
                        sprintf(filestring, "F:*.mp3");
                    }
                    else
                    {
	                     sprintf(filestring, "F:*.mp3");
                        //sprintf(filestring, "%s%s", currentDirectory, filespec);
                    }
                    printf("fopen(%s, %s) \n", filestring, filenum);
                    metaFile = fopen(filestring, filenum);
                    
                    // make sure file successfully opened
                    if (metaFile)
                    {
                        //printf("opened metafile pointer! \n");
                        // Now assign the file pointer to the struct
                        metadata->fp = metaFile;
                        // Retrieve metadata 
                        RunLibraryFunction("MYMETA",ENTRY_1,(int) &metadata);
                        //printf("Title : %s \n", metadata->title);
                        //printf("Artist : %s \n", metadata->artist);
                        //printf("Album : %s \n", metadata->album);

                        // We want to add this information to the master file list 
                        // so that the "songs" menu is correct
                        if (!strcmp((char *) metadata->title, "UNKNOWN"))
                        {
                            //printf("title meta not stored, using file name : %s \n", fileName);
                            //memcpy(metadata->title, fileName, 30);
                            strncpy((char *) metadata->title, fileName, 30);
                            metadata->title[30] = '\0';
                            printf("%s \n", metadata->title);
                            //fprintf(openedFile, "%s|%i|%s| \r\n", fileName, songsInDirectory++, currentDirectory);
                        }
                        
                        if (root)
                        {
                            fprintf(openedFile, "%s|%i|%s| \r\n", (char *) metadata->title, songsInDirectory, nextDirectory);
                        }
                        else
                        {
                            fprintf(openedFile, "%s|%i|%s/| \r\n", (char *) metadata->title, songsInDirectory, nextDirectory);
                        }
                        
                        // We also want to add it to an artist file so songs can
                        // be sorted by artist as well
                        // FIXME : currently append mode does not work properly, so calling on the writeToFile function
                        //         won't produce the proper results.
                        sprintf(filestring, "S:lookup/artists/%s.txt", (char *) metadata->artist);
                        writeToFile(filestring, songsInDirectory++, currentDirectory, (char *) metadata->title);
                        
                        fclose(metaFile);
                    }
                    else
                    {
                        printf("failed to open metafile pointer\n");
                    } 
                }
            } while (S_OK == FatFindNext(dirFile,fileName,FILE_NAME_CHARS));
        } 
        else 
        {
            printf("Path not found\n");
        }
    
        fclose(dirFile);
        
        // If we're not at the root directory, we always need to navigate back to the parent directory
        // for the parent recursive call to continue navigating properly
        if (!root)
        {
        	RunProgram("cd", "..");
        	//strcpy(tempDirectory, nextDirectory);
        	//strncpy(nextDirectory, tempDirectory, previousDirLength);
        }
        
        return (totalNumSongs + songsInDirectory);
    }
}

// Checks if a file exists and appends metadata and location information to it if so.
// If it doesn't exist, it creates the file
// FIXME : currently append mode does not work properly, so calling on the writeToFile function
//         won't produce the proper results. This function could be rewritten to open a file in
//         read mode and then copy the contents to a new file and then write the new line
//         Or, if you are feeling confident, it is possible to open the file in read or append mode
//         and try to fseek your way around to the end of the file to add your new line.
void writeToFile(char *FileName, u_int16 fileIndex, char *mp3Location, char *songName) 
{
    FILE *openedFile;
    // open file in read mode to check if it exists. current directory should be set at this point,
    // so a path is not needed, just the filename
    openedFile = fopen(FileName, "rb");

     // check if it exists
    if (openedFile)
    {
        printf("file exists! \n");
        // it exists, so we want to re-open in append mode
        fclose(openedFile);
        openedFile = fopen(("%s", FileName), "a");
        if (openedFile)
        {
            printf("opened in append mode correctly");
        }
        else
        {
            printf("did not open in append mode");
            return;
        }
    }
    // it doesn't exist
    else
    {
        // opening in write mode creates it
        openedFile = fopen(("%s", FileName), "w");
        // No reason it should fail to open, but just in case
        if (!openedFile)
        {
            printf("could not find path to create : %s \n", FileName);
            return;
        }
    }

    // add index, filename, and songname to row
    //printf("%lu size \n", sizeof((char) fileIndex));
    fprintf(openedFile, "%s|%i|%s|\n", songName, fileIndex, mp3Location);
    // close file
    fclose(openedFile);
}