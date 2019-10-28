/// \file browsefiles.c Small and efficient File browser that uses plFatIndex playlist functionality 
/// and StdButtons with a custom renderer. 
/// Browses FAT disks on a VSOS system and builds a playlist from all files with identified file
/// suffixes in a single folder. 

/// Currently uses the same playlist for browsing and playing to save memory - allocating them separately
/// would allow editing the playlist while songs are playing - perhaps in Classic Player next version

/// \author Panu-Kristian Poiksalo, VLSI Solution Oy


#include <vo_stdio.h>
#include "browsefiles.h"
#include <stdbuttons.h>
#include <uimessages.h>
#include <lcd.h>
#include <string.h>
#include "lcdstuff.h"
#include "plfatindex.h"
#include "screenlayout.h"
#include "usemodel.h"
#include <stdlib.h>


StdButton *renderButton;
u_int16 renderResult;
u_int16 pathSelected = 0;
u_int16 extraX = 500;

void ID3RenderCallBack(s_int16 index, u_int16 message, u_int32 value) {
	if ((message) == UIMSG_TEXT_SONG_NAME) {
		strncpy(currentFileName, (char*)value, sizeof(currentFileName)-1); 
		renderResult = 1;
	}
	if ((message > UIMSG_TEXT_SONG_NAME) && (message <= UIMSG_TEXT_ARTIST_NAME)) {
		extraX = LcdTextOutXY16(extraX, 218, PANEL_RIGHT-3, greyText, (char*)value);
		extraX = LcdTextOutXY16(extraX, 218, PANEL_RIGHT-3, greyText, "  ");
	}
}


void FileButtonRender (register struct stdButtonStruct *button, register u_int16 op, register u_int16 x, register u_int16 y) {
	DirEntryInfo *e = button->extraRenderInfo;
	FILE *f;
	static void *textColors[2] = {blackText,blueOnWhite}; 	
	
	if (button->flags & BTN_LOWERED) {
		extraX = PANEL_LEFT;
	} else if (button->result > 0) {
		LcdFilledRectangle(PANEL_LEFT+1, 218, PANEL_RIGHT-1, 218+16, NULL, COLOR_WINDOW);
		extraX = 500;	
	}
			
	if (e) {
		f = OpenFileByDirEntryInfo(path,e);
		if (f) {
			renderButton = button;
			model.DecodeID3(f, ID3RenderCallBack);
			fclose(f);
			Delay(1);
			StdRender(button,op,x,y);
			LcdTextOutXY16(renderButton->x1+3, renderButton->y1+2, PANEL_RIGHT-10, (renderButton->flags&BTN_LOWERED)?loweredText:textColors[button->flags>>15], currentFileName);
			return;
		} 
	}
	StdRender(button,op,x,y);
	extraX = 500;
	
}

#define BROWSER_Y1 (PANEL_CLIENT_Y1 + 10)
#define BTN_BROWSE_UP -1
#define BTN_BROWSE_DOWN -2 
#define BTN_SELECT_DISK -3
#define BTN_CANCEL -4


StdButton fileButtons[15] = {
	{BTN_NORMAL,BTN_BROWSE_UP, PANEL_RIGHT - 26,BROWSER_Y1 + 3,  PANEL_RIGHT,BROWSER_Y1 + 40, "\x8d",NULL,NULL},
	{BTN_NORMAL,BTN_BROWSE_DOWN, PANEL_RIGHT - 26,BROWSER_Y1 + 140,PANEL_RIGHT,BROWSER_Y1 + 177,"\x9d",NULL,NULL},
	{BTN_NO_FACE,BTN_SELECT_DISK,3,3,298,20,"",NULL,NULL},
	{BTN_DISABLED|BTN_INVISIBLE,BTN_CANCEL,300,3,316,20,"X",NULL,NULL}, //This cannot work now - playlist is lost when entering the file select dialog
};
void MakeFileButtons(u_int16 n) {
	u_int16 i;
	u_int16 *p = (u_int16*)(&fileButtons[4]);
		
	fileButtons[0].render = StdRender;
	fileButtons[1].render = StdRender;
	fileButtons[2].render = StdRender;
	fileButtons[3].render = StdRender;
	extraX = 500;


	if (n) { // "up" button - visible or not?
		fileButtons[0].flags = BTN_NORMAL;	
	} else {
		fileButtons[0].flags = (BTN_NO_BEVEL | BTN_NO_CAPTION | BTN_DISABLED);
	}

	// Populate the button structure with pointers to file information in extraInfo;
	for (i=0; i<8; i++) {
		u_int16 y1;
		/*flags*/ 	*p++ = (n<nFiles) ? (files[n].flags & DIR_FLAG_DIR ? BTN_HIGHLIGHTED|(1<<15) : BTN_NORMAL) : (BTN_NO_BEVEL | BTN_NO_CAPTION | BTN_DISABLED);
		/*result*/ 	*p++ = n+1;
		/*x1*/ 		*p++ = PANEL_LEFT;
		/*y1*/ 		*p++ = y1 = BROWSER_Y1 + 3 + i * 22;
		/*x2*/ 		*p++ = PANEL_RIGHT-30;
		/*y2*/ 		*p++ = y1+19;
		/*caption*/ *p++ = (u_int16)"";
		/*extraRI*/ *p++ = (n<nFiles) ? &files[n]: NULL; //put pointer to file information to button's extraRenderInfo
		/*render*/ 	*p++ = FileButtonRender;
		n++;
	}


	if (n>=nFiles) { // "down" button - visible or not?
		fileButtons[1].flags = (BTN_NO_BEVEL | BTN_NO_CAPTION | BTN_DISABLED);
	} else {
		fileButtons[1].flags = BTN_NORMAL;	
	}
	

}

void SelectNextFatDisk() {
	int i = path[0]-'A';
	do { //Advance to the next FAT disk
		i++;
		if (i>25) i=0;
	} while (!vo_pdevices[i] || vo_pdevices[i]->fs != vo_filesystems[0]); //this checks that fs of device is FAT
	sprintf(path,"%c:",'A'+i);
}

void *SelectSong() {
	u_int16 selectedSong;
	u_int16 n = 0;
	u_int16 r = 0;
	while (1) {
		MakeFileButtons(n);
		RenderStdButtons(fileButtons);
		while(!(r = GetStdButtonPress(fileButtons))) {
			Delay(10);
		}
		if (r == BTN_BROWSE_UP) {
			n-=8;
		} else if (r == BTN_BROWSE_DOWN) {
			n += 8;
		} else if (r == BTN_SELECT_DISK) {
			SelectNextFatDisk();
			return 0;
		} else {
			n=2;
			currentSongNumber = r;
			while(1) { //File or directory selected
				if (fileButtons[n].result == r) {
					FILE *f = OpenFileByDirEntryInfo(path,(DirEntryInfo *)fileButtons[n].extraRenderInfo);
					r = f->flags & __MASK_READABLE; //File selected? (OpenFileByDirEntryInfo clear READABLE for dirs)
					fclose(f);
					if (r) return 1; //File selected	
					n = strlen(path);
					sprintf(&path[n],"%s/",currentFileName);
					return 0; //Directory selected
				}
				n++;
			}
		}
	}
}

#define attr ungetc_buffer
#define shortname extraInfo

int CompareDirEntry(const DirEntryInfo *a, const DirEntryInfo *b) {
	s_int16 d = (b->flags & DIR_FLAG_DIR) - (a->flags & DIR_FLAG_DIR);
	if (d) return d;
	if (a->order == b->order) return 0;
	if (a->order < b->order) return -1;
	return 1;
}


void SelectFileFromFolder() {
	// Read directory	
	char filename[NAMELENGTH];
	s_int16 i;
	volatile FILE *f = NULL;
	if (path[0] == '@') SelectNextFatDisk();

	restart:
	if (__F_OPEN(f)) fclose(f);
	f = fopen(path,"s");
	if (!f) return;
	
	DrawBevel(2,PANEL_CLIENT_Y1,317,237,lowered); //Client Area

	LcdFilledRectangle(5,3,297,PANEL_CLIENT_Y1-1,NULL,COLOR_WINDOW);
	i = LcdTextOutXY16(5,3,297,blackText,vo_pdevices[path[0]-'A']->Identify(vo_pdevices[path[0]-'A'],NULL,0));
	LcdTextOutXY16(i+10,3,297,blackText,path);

	
	// Populate the playlist with files and directories from the path
	plFatIndex.flags |= PF_SIZE_CHANGED; 
	nFiles = 0;
	if (FatFindFirst(f,&path[2],filename,NAMELENGTH) == S_OK) {
		do {
			DirEntryInfo *e = &files[nFiles];
			e->dirSector = f->currentSector; //Disk sector where the directory entry is
			e->flags = (f->pos-32)&511; //Location of directory entry within sector;
			e->order = filename[0]<<8 | filename[1]&0xff; //order by first 2 letters
			if (f->attr & __ATTR_DIRECTORY) {
				e->flags |= DIR_FLAG_DIR;				
				//continue;
			} else if (!strstr("MP3,MP2,WMA,AAC,OGG,WAV,FLA,ASF,M4A,MP4",&f->extraInfo[13])) {
				continue; //skip file with other extensions
			}
			nFiles++;
		} while (S_OK == FatFindNext(f,filename,NAMELENGTH));
	}
	fclose(f);
	
	//Sort directory
	qsort(files, nFiles, sizeof(files[0]), CompareDirEntry);

	if (!SelectSong()) {
		goto restart;
	}
	plFatIndex.flags |= PF_SIZE_CHANGED;

	//remove directory entries from the playlist
	while (nFiles && files[0].flags & DIR_FLAG_DIR) {
		memcpy(&files[0],&files[1],sizeof(files));
		nFiles--;
		currentSongNumber--;
	}
	
	//now we have proper playlist and currentSongNumber. We can safely return.
}

