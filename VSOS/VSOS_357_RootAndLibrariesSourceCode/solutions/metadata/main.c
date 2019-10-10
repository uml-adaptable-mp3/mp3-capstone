/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <volink.h>
#include <uimessages.h>
#include <string.h>
#include <kernel.h>

#if 1
#define USE_MOJIBAKE
#endif


// ENTRY_1
DLLENTRY(DecodeID3) // Call this to decode tags and handle the values in your own callback function
void DecodeID3(register FILE* fp, register UICallback callback);

// ENTRY_2
DLLENTRY(PrintID3f) // Call this to pass a file pointer and print the messages to stdout
ioresult PrintID3f(FILE *f);

ioresult main(char *parameters); // Call this to pass a file name string and print the messages to stdout



void ID3RenderCallBack(s_int16 index, u_int16 message, u_int32 value) {
  unsigned char *str = (unsigned char *)value;
  /* Album name, artist name, and song name may have characters outside the
     7-bit limited character set, while our characters can contain upto
     16-bit UCS-2 values. So we will print out data in UTF-8 format. */
  printf("~%04x'",message);
  while (*str) {
    if (*str >= 0x800) {
      /* Format: 1110xxxx 10xxxxxx 10xxxxxx (code points 0x800 to 0xffff) */
      putchar(0xe0 | (*str>>12));
      putchar(0x80 | ((*str>>6) & 0x3f));
      putchar(0x80 | (*str & 0x3f));
    } else if (*str >= 0x80) {
      /* Format: 110xxxxx 10xxxxxx (code points 0x80 to 0x7ff) */
      putchar(0xc0 | (*str>>6));
      putchar(0x80 | (*str & 0x3f));
    } else {
      /* Format: 0xxxxxxx (7-bit ASCII, code points 0x0 to 0x7f) */
      putchar(*str);
    }
    str++;
  }
  printf("\n");
}


#define MAX_TMP_STR_LEN 256 /* Must be at least 158 words, requirement by DecodeID3 */
static u_int16 currTmpStr[MAX_TMP_STR_LEN];

//
// ID3 parser routines
//

s_int32 GetID3Size7(FILE* fp) {
	int i;
	u_int32 res=0;
	for(i=0;i<4;i++) {
		res<<=7;
		res|=fgetc(fp)&0x7F;
	}
	return res;
}

s_int32 GetID3Size8(FILE* fp) {
	int i;
	u_int32 res=0;
	for(i=0;i<4;i++) {
		res<<=8;
		res|=fgetc(fp);
	}
	return res;
}

#define ID3F_UNSYNCHRONIZED	0x80
#define ID3F_EXTENDED		0x40
#define ID3F_EXPERIMENTAL	0x20

typedef struct ID3Tags {
	u_int16 uiMessage;
	char tagStr[4];
} ID3Tags;

#pragma msg 275 off

const ID3Tags id3Tags[]={
	{UIMSG_TEXT_SONG_NAME,"TIT2"},
	{UIMSG_TEXT_ALBUM_NAME,"TALB"},
	{UIMSG_TEXT_ARTIST_NAME,"TPE1"},
	{UIMSG_TEXT_YEAR,"TYER"},
	{UIMSG_TEXT_TRACK_NUMBER,"TRCK"},
	{UIMSG_VSOS_END_OF_LIST,""}
};

typedef void (*StringDecoder)(register u_int16*,register char);
#define GetStringDecoder(lib) (*(((StringDecoder*)(lib))+2+ENTRY_1))

static void TrimAndSend(register char *string,register UICallback callback,register u_int16 uiMsg) {
	// remove trailing blanks from ID3V1 tag
	char *c=currTmpStr+29;
	strncpy(currTmpStr, string, 30);
	currTmpStr[30] = '\0';
	while(*c==' ' && c > string) {
		*c-- = '\0';
	}
	callback(-1,uiMsg,(u_int32)currTmpStr);
}


void DecodeID3(register FILE* fp, register UICallback callback) {
	u_int32 oldPos = ftell(fp);

	fseek(fp, 0, SEEK_SET);
	if(fgetc(fp)=='I' && fgetc(fp)=='D' && fgetc(fp)=='3' && 
		((fgetc(fp)+1)>>1)==2 /* (accept v2.3 and 2.4) */ && fgetc(fp)==0x00) {
		/* found ID3v2 tag */
		#ifdef USE_MOJIBAKE
		void* stringDecoderLib=LoadLibrary("MOJIBAKE");
		#endif
		u_int16 flags=fgetc(fp);
		s_int32 id3Left=GetID3Size7(fp);
		if(flags&ID3F_EXTENDED) {
			u_int16 extSize=(u_int16)GetID3Size8(fp);
			fseek(fp,extSize,SEEK_CUR);
			id3Left-=extSize;
		}
		while(id3Left>0) {
			char frameId[4];
			s_int32 frameSize;
			u_int16 frameFlags;
			u_int16 thereIsFrameId=0;
			s_int16 i;
			for(i=0;i<4 && id3Left>0;i++) {
				thereIsFrameId|=(frameId[i]=fgetc(fp));
				id3Left--;
			}
			if(thereIsFrameId && id3Left>=6) {
				ID3Tags* cTag;
				frameSize=GetID3Size8(fp);
				frameFlags=(u_int16)fgetc(fp)<<8;
				frameFlags|=fgetc(fp);
				id3Left-=6;
				if(id3Left<frameSize) {
					frameSize=id3Left;
				}
				id3Left-=frameSize;
				i=0;
				cTag=id3Tags;
				while(cTag->uiMessage!=UIMSG_VSOS_END_OF_LIST) {
					if(!memcmp(cTag->tagStr,frameId,4)) {
						u_int16 s;
						u_int16* write=currTmpStr;
						char coding=fgetc(fp);
						--frameSize;
						s=(u_int16)frameSize;
						if(s>MAX_TMP_STR_LEN-2) {
							s=MAX_TMP_STR_LEN-2;
						}
						while(s-->0) {
							write[0]=fgetc(fp);
							++write;
							--frameSize;
						}
						write[0]=0;
						#ifdef USE_MOJIBAKE
						write[1]=0; // (double termination)
						if(stringDecoderLib) {
							StringDecoder Decode=GetStringDecoder(stringDecoderLib);
							Decode(currTmpStr,coding);
						}
						#endif
						callback(-1,cTag->uiMessage,(u_int32)currTmpStr);
					}
					++cTag;
				}
				fseek(fp,frameSize,SEEK_CUR);
			} else {
				fseek(fp,id3Left,SEEK_CUR);
				id3Left=0;
			}
		}
		#ifdef USE_MOJIBAKE
		if(stringDecoderLib) {
			DropLibrary(stringDecoderLib);
		}
		#endif
	} else { /* Look for ID3v1 tag */
		s_int16 i;
		char* c=(char*)currTmpStr;
		fseek(fp,-128,SEEK_END);
		for(i=0;i<128;i++) {
			*c++=fgetc(fp);
		}
		c=(char*)currTmpStr;
		if(!strncmp(c,"TAG",3)) { /* ID3v1 tag found */
			TrimAndSend(c+3,callback,UIMSG_TEXT_SONG_NAME);
			TrimAndSend(c+33,callback,UIMSG_TEXT_ARTIST_NAME);
			TrimAndSend(c+63,callback,UIMSG_TEXT_ALBUM_NAME);
			c+=93;
			c[4]=0;
			callback(-1,UIMSG_TEXT_YEAR,(u_int32)c);
		}
	}
	fseek(fp, oldPos, SEEK_SET);
}

ioresult PrintID3f(FILE *fp) {
  DecodeID3(fp, ID3RenderCallBack);
  return S_OK;
}

// Startup code for each instance of the library
// If CONFIG.TXT has several instance of the same driver, this is called for each line.
ioresult main(char *parameters) {
	FILE *f = fopen(parameters,"rb");
	if (!f) {
		printf("File not found.\n");
		return S_ERROR;
	}
	PrintID3f(f);
	fclose(f);
}

