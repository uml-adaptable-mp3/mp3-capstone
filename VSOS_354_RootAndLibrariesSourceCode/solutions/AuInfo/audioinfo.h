#ifndef AUDIO_INFO_H
#define AUDIO_INFO_H

struct AudioInfo {
  s_int32 fileBytes;
  s_int16 channels;
  s_int32 sampleRate;
  double seconds;
  char formatString[16];
};

#define GAIF_FAST 1

/* Gets audio info for given file. Returns S_OK if it recognized a file format,
   S_ERROR if file didn't exist or was of unknown format.
   If outFile is not NULL, send verbose output to that file. */
ioresult GetAudioInfo(register const char *fileName,
		      register struct AudioInfo *ai,
		      register FILE *outFile,
		      register u_int16 flags);

/* Gets audio info for given file pointer.
   Returns S_OK if it recognized a file format,
   S_ERROR if file didn't exist or was of unknown format.
   If outFile is not NULL, send verbose output to that file.
   Doesn't change location in file. */
ioresult GetAudioInfoFP(register FILE *fp,
			register struct AudioInfo *ai,
			register FILE *outFile,
			register u_int16 flags);
#endif
