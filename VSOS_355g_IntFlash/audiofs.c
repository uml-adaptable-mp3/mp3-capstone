/**
   \file audiofs.c Audio Filesystem driver for VsOS
   \author Henrik Herranen, VLSI Solution Oy
*/

#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <audio.h>
#include <timers.h>
#include "vsos.h"
#include "audiofs.h"
#include "devAudio.h"
#include "parseFileParam.h"
//#include "forbid_stdout.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

ioresult AudioFsCreate(register __i0 DEVICE *dev, const char *name,
		       u_int16 *fatBuffer) {
  //  AudioDeviceInfo *di=(AudioDeviceInfo*)dev->deviceInfo;
  return S_OK;
}




ioresult VoAudioCloseFile(register __i0 VO_FILE *f){ ///< Flush and close file
  devAudioHwInfo *hw = (devAudioHwInfo *)f->dev->hardwareInfo;	
  if (f->flags & __MASK_READABLE) {
    ioctl(f, IOCTL_AUDIO_CLOSE_INPUT, NULL);
  }
#if 0
  printf("VoAudioCloseFile(%p), rc %d, wc %d\n",
	 f, hw->readOpenCounter, hw->writeOpenCounter);
#endif
  return S_OK;
}


u_int16 VoAudioWriteFile(register __i0 VO_FILE *f, void *buf,
			  u_int16 sourceIndex, u_int16 bytes) {
  AudioFileInfo *fi = (AudioFileInfo *)f->fileInfo;
  u_int16 *d = buf;
  u_int16 left = bytes;
  if (!(f->flags & __MASK_WRITABLE)) {
    return 0;
  }

  d += sourceIndex >> 1;
  sourceIndex &= 1;

#if 0
  printf("VoAudioWriteFile(%p, %p, sI %d, b %d)\n",
	 f, buf, sourceIndex, bytes);
#endif

  if (fi->bytesInOCache || sourceIndex || (left & 3) || fi->oChannels != 2) {
    /* Cannot use fast copy. We'll do it the hard way. */
    while (left--) {
      int c;
      static const u_int16 __y cacheShiftO[4] = {8, 0, 24, 16};
      if (sourceIndex) {
	c = *d++ & 0xFF;
      } else {
	c = (*d >> 8);
      }
      sourceIndex = 1-sourceIndex;
      fi->oCache |= (u_int32)c << cacheShiftO[fi->bytesInOCache];
      fi->bytesInOCache = (fi->bytesInOCache+1) & 3;
      if (fi->oChannels == 1 && fi->bytesInOCache == 2) {
	fi->oCache += (u_int32)fi->oCache << 16;	// Copy LSB's to MSB's
	fi->bytesInOCache = 0;
      }
      if (!fi->bytesInOCache) {
	int t = f->dev->Write(f, &fi->oCache, 0, 4);
	if (t != 4) {
	  if (bytes >= t) {
	    bytes -= t;
	  } else {
	    bytes = 0;
	  }
	}
	fi->oCache = 0;
      }
    }
  } else {
    bytes = f->dev->Write(f, d, 0, left);
  }

  f->pos += bytes;

  return bytes;
}


u_int16 VoAudioReadFile(register __i0 VO_FILE *f, void *buf, u_int16 destIndex,
			u_int16 bytes) {
  AudioFileInfo *fi = (AudioFileInfo *)f->fileInfo;
  u_int16 *d = buf;
  u_int16 left = bytes;

#if 0
  printf("VoAudioReadFile(%p, %p, sI %d, b %d)\n",
	 f, buf, destIndex, bytes);
#endif

  if (!(f->flags & __MASK_READABLE)) {
    return 0;
  }

  d += destIndex >> 1;
  destIndex &= 1;

  bytes = 0;

  if (fi->bytesInICache || destIndex || (left & 3) || fi->iChannels != 2) {
#if 0
    printf(" VoR: %d %d %d %d\n",
	   fi->bytesInICache, destIndex, (left & 3), fi->iChannels);
#endif
    /* Cannot use fast copy. We'll do it the hard way. */
    while (left--) {
      int c;

      if (!fi->bytesInICache) {
	fi->bytesInICache = f->dev->Read(f, &fi->iCache, 0, 4);
	if (fi->bytesInICache != 4) {
#if 0
	  printf("****PANIC AT THE DISCO!\n");
	  while (1)
	    ;
#endif
	  return bytes-left-1;
	}
      }

      //      printf("bIC %d, ", fi->bytesInICache);
      c = (fi->iCache >> (--fi->bytesInICache * 8)) & 0xFF;
      //      printf("cac 0x%08lx, c 0x%02x\n", fi->iCache, c);
      if (destIndex) {
	*d = (*d & 0xFF00U) | c;
	d++;
      } else {
	*d = (*d & 0x00FFU) | (c<<8);
      }
      destIndex = 1-destIndex;
    }
  } else {
    bytes = f->dev->Read(f, d, 0, left);
  }

  f->pos += bytes;

  return bytes;
}





/*
  Reminder from Ansi C standard.
  "r"  = open text file for reading
  "w"  = create text file ; discard previous contents if any
  "a"  = append; open or create text file for writing at end of file
  "r+" = open text file for update (i.e., readingand writing)
  "w+" = open text file for update, discard previous contents if any
  "a+" = append; open or create text file for update, writing at end

  Because this is an audio stream device, no seek operations are allowed.
  Our take on correct modes can be seen from the fileModes table below.
*/
ioresult VoAudioOpenFile(register __i0 VO_FILE *f, const char *name,
			 char *mode) {
  AudioFileInfo *fi = (AudioFileInfo*)f->fileInfo;
  char *c;
  static const struct FileModes {
    const char *s;
    vo_flags flags;
  } __y fileModes[6] = {
    {"r",  __MASK_FILE | __MASK_READABLE                  },
    {"w",  __MASK_FILE                   | __MASK_WRITABLE},
    {"a",  __MASK_FILE                   | __MASK_WRITABLE},
    {"r+", __MASK_FILE | __MASK_READABLE | __MASK_WRITABLE},
    {"w+", __MASK_FILE | __MASK_READABLE | __MASK_WRITABLE},
    {"a+", __MASK_FILE | __MASK_READABLE | __MASK_WRITABLE},
  };
  int i;
  int len = strlen(mode);
  devAudioHwInfo *hw = (devAudioHwInfo *)f->dev->hardwareInfo;	

#if 0
  printf("## Joo\n");
#endif
#if 0
  printf("####1 %9ld\n", FindAudioParam(name, "hui", 12345678));
  printf("####1 %9ld\n", FindAudioParam(name, "fo", 12345678));
  printf("####1 %9ld\n", FindAudioParam(name, "ci", 12345678));
  printf("####1 %9ld\n", FindAudioParam(name, "co", 12345678));
#endif
#if 0
  {
    struct AudioFileParam afp;
    while (name = ParseAudioParam(&afp, name)) {
      printf("Now name = \"%s\"\n");
    }
    while (1)
      ;
  }
#endif


#if 0
#define DST_SIZE 16
  {
    s_int16 n = 0;
    const char *c;
    char s[DST_SIZE];
    c = FileParamSeek("if=12345/of=par1,par2,par3/out=1212", "i", &n);
    printf("n %2d, c %s\n", n, c ? c : "NULL");
    c = FileParamSeek("if=12345/of=par1,par2,par3/out=1212", "if", &n);
    printf("n %2d, c %s\n", n, c ? c : "NULL");
    c = FileParamSeek("if=12345/of=par1,par2,par3/out=1212", "of", &n);
    printf("n %2d, c %s\n", n, c ? c : "NULL");
    c = FileParamSeek("if=12345/of=par1,par2,par3/out=1212", "out", &n);
    printf("n %2d, c %s\n", n, c ? c : "NULL");
    n = FileParamInt("if=12345/of=par1,par2,par3/out=1212", "i", 444);
    printf("n %6d\n", n);
    n = FileParamInt("if=12345/of=par1,par2,par3/out=1212", "if", 444);
    printf("n %6d\n", n);
    n = FileParamInt("if=12345/of=par1,par2,par3/out=1212", "of", 444);
    printf("n %6d\n", n);

    n = FileParamStr("if=12345/of=par1,par2,par3/out=1212", "i", s, DST_SIZE);
    printf("n %2d, c %s\n", n, s);
    n = FileParamStr("if=12345/of=par1,par2,par3/out=1212", "if", s, DST_SIZE);
    printf("n %2d, c %s\n", n, s);
    n = FileParamStr("if=12345/of=par1,par2,par3/out=1212", "of", s, DST_SIZE);
    printf("n %2d, c %s\n", n, s);
    c = s;
    n = abs(n);
    while (n--) {
      printf("  \"%s\"\n", c);
      c += strlen(c)+1;
    }
    n = FileParamStr("if=12345/of=par1,par2,par3/out=1212", "out", s,DST_SIZE);
    printf("n %2d, c %s\n", n, s);

    while (1)
      ;
  }
#endif

  memset(fi, 0, sizeof(*fi));
#if 0
  fi->oChannels = fi->iChannels = 2;
#else
  fi->oChannels = FileParamInt(name, "oc", 2);
  if (fi->oChannels > 2 || fi->oChannels < 1) {
    return SysError("Incorrent number of output channels");
  }
  fi->iChannels = 2;
  fi->iChannel = -1;
  if (hw->oResources & AOR_DAC) {
    return SysError("Output already in use");
  }
  hw->oResources = AOR_DAC;

#endif

  {

    if (len && mode[len-1] == 'b') {
      len--;	// No separate binary file handling, ignore 'b'
    }
    i = -1;
    while (++i < 6 && strncmp(mode, fileModes[i].s, len))
      ;
    if (i < 6) {
      f->flags = fileModes[i].flags;
    } else {
      return SysError("Incorrent mode");
    }
#if 0
    printf("VoAudioOpenFile(%p, \"%s\", \"%s\")\n",
	   f, name, mode);
#endif
  }


#if 0
  {
    struct AudioFileParam afp;
    while (name = ParseAudioParam(&afp, name)) {
      printf("Now name = \"%s\"\n");
    }
    while (1)
      ;
  }
#else

  {
    s_int32 oRate = FileParamInt(name, "of", 0);
    if (oRate) {
#if 0
      printf("##1, f=%p, f->dev=%p, f->dev->Ioctl=%p, DAI=%p, flags=0x%x!\n",
	     f, f->dev, f->dev->Ioctl, DevAudioIoctl, f->flags);
#endif
      ioctl(f, IOCTL_AUDIO_SET_ORATE, &oRate);
    }
  }

  /* Open input and allocate resources if first. */
  if (f->flags & __MASK_READABLE) {
    u_int32 iRate;
#if 1
    u_int16 idPattern = AID_LINE1_1|AID_LINE1_3;
#elif 0
    u_int16 idPattern = AID_LINE1_1|AID_LINE1_3|AID_DEC6;
#elif 0
    u_int16 idPattern = AID_MIC1|AID_LINE1_3;
#else
    u_int16 idPattern = AID_LINE1_1|AID_MIC2;
#endif
    s_int16 bufSize = FileParamInt(name, "ibuf", 512);

    //    printf("idPattern %04p %04p\n", idPattern, AID_LINE1_1|AID_LINE1_3);
    if (ioctl(f, IOCTL_AUDIO_OPEN_INPUT, (void *)idPattern)) {
      return SysError("Can't open audio input");
    }

    if (ioctl(f, IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE, (void *)bufSize)) {
      return SysError("Can't set audio input buffer");
    }

    if ((iRate = FileParamInt(name, "if", 0))) {
#if 0
      printf("##2, f=%p, f->dev=%p, f->dev->Ioctl=%p, DAI=%p, flags=0x%x!\n",
	     f, f->dev, f->dev->Ioctl, DevAudioIoctl, f->flags);
#endif
      ioctl(f, IOCTL_AUDIO_SET_IRATE, &iRate);
    }
  }
#endif

#if 0
  printf("  -> rc %d, wc %d, flags 0x%x\n",
	 hw->readOpenCounter, hw->writeOpenCounter, f->flags);
#endif

  return S_OK;
  //return SysError("Cannot create file");
}

const char *IdentifyAudio(register __i0 void *fs, char *buf, u_int16 bufsize){
  return "Audio FS";
}



#if 1
IOCTL_RESULT VoAudioIoctl(register __i0 VO_FILE *f, s_int16 request, IOCTL_ARGUMENT argp) {
  u_int32 *p32 = (u_int32 *)argp;
  AudioFileInfo *fi = (AudioFileInfo*)f->fileInfo;
#if 0
  printf("VoAudioIoctl f=%p req=%d p=%p, p32=%ld\n", f, request,
	 argp, *p32);
#endif

  switch(request) {
  case IOCTL_RESTART:
    fi->bytesInOCache = 0;
    fi->oCache = 0;
    return f->dev->Ioctl(f, request, argp); // Restart also device
    break;
  case IOCTL_AUDIO_GET_OCHANNELS:
    return fi->oChannels;
    break;
  case IOCTL_AUDIO_SET_OCHANNELS:
    if ((int)argp < 1 || (int)argp > 2) {
      return S_ERROR;
    }
    fi->oChannels = (int)argp;
#if 0
    printf("  New oChannels %d\n", fi->oChannels);
#endif
    fi->bytesInOCache = 0;
    fi->oCache = 0;
    break;
  case IOCTL_AUDIO_GET_ICHANNELS:
    return fi->iChannels;
    break;
  case IOCTL_AUDIO_SET_ICHANNELS:
    if ((int)argp < 1 || (int)argp > 2) {
      return S_ERROR;
    }
    fi->iChannels = (int)argp;
#if 0
    printf("  New channels %d\n", fi->oChannels);
#endif
    fi->bytesInICache = 0;
    fi->iCache = 0;
    break;
  case IOCTL_AUDIO_GET_ORATE:
    *p32 = hwSampleRate;
    break;
  default:
    /* We cannot handle the message: forward it to the device */
    return f->dev->Ioctl(f, request, argp);
    break;
  }
  return S_OK;
}
#endif


#pragma msg 30 off //CommonErrorResultFunction triggers warning
const FILEOPS AudioFileOps = {
	VoAudioOpenFile, //ioresult (*Open) (register __i0 VO_FILE *f, const char *name, const char *mode); ///< Find and open a file
	VoAudioCloseFile, //ioresult (*Close)(register __i0 VO_FILE *f); ///< Flush and close file
	VoAudioIoctl,	//ioresult (*Ioctl)(register __i0 VO_FILE *f, s_int16 request, char *argp); ///< For extensions, not used.	
	VoAudioReadFile, //u_int16  (*Read) (register __i0 VO_FILE *f, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	VoAudioWriteFile, //u_int16  (*Write)(register __i0 VO_FILE *f, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
};
#pragma msg 30 on



const FILESYSTEM AudioFileSystem = {
	__MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE, //u_int16  flags; //< present, initialized,...
	IdentifyAudio, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);
	AudioFsCreate, // ioresult (*Create)(DEVICE *dev, char *name);
	0, //ioresult (*Delete)(DEVICE *dev);
	VoAudioIoctl, //ioresult (*Ioctl) (DEVICE *dev);
	&AudioFileOps, //FILEOPS  *op;	// Opened file inherits this set of file operations.
};
