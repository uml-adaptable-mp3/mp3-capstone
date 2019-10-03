/*

  Sine plays sine waves through stdaudioout
  
*/
#include <vo_stdio.h>
#include <stdlib.h>
#include <apploader.h>
#include <consolestate.h>
#include <string.h>
#include <stdlib.h>
#include <audio.h>
#include <math.h>
#include <aucommon.h>
#include <ringbuf.h>
#include <timers.h>
#include <kernel.h>

#define MY_AUDIO_BUFFER_SIZE 128

s_int32 myAudioBuffer[MY_AUDIO_BUFFER_SIZE];

#define MAX_SINES 16
#define PARAMETER_STACK_SIZE (3*MAX_SINES)

double __mem_y parameterStack[PARAMETER_STACK_SIZE];
int parametersInStack = 0;

struct Sine {
  double phase, phaseAdd, leftLin, rightLin;
  double freq, left_dB, right_dB;
};

struct Sine __mem_y sine[MAX_SINES];
int sines=0;

#define MAX(a,b) (((a)>(b))?(a):(b))

ioresult main(char *parameters) {
  int i, nParam, verbose = 1;
  char *p = parameters;
  ioresult ret = S_ERROR;
  int only16BitsAvailable = 0;
  u_int32 sampleRate = 0;
  double totLeft = 0.0, totRight = 0.0;
  u_int32 oldTC = ReadTimeCount();
  u_int32 underflows = 0;

  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: Sine freq1 dbl1 dbr1 [freq2 dbl2 dbr2 [...]] [-rrate|-h]\n"
	     "freq dbl dbr\tSet frequency to freq Hz, left volume to dbl dB,\n"
	     "\t\tright volume to dbr dB. If dbl or dbr is greater than zero,\n"
	     "\t\tvolume is muted for that channel.\n"
	     "-rrate\tSet sample rate to rate Hz\n"
	     "-h\tShow this help\n"
	     "\nExamples:\n"
	     "  Sine 1000 0 0\n"
	     "  Sine -r24000 1000 -6 1 2000 1 -6 3000 -6.1 -6.1\n");
      goto finallyok;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strncmp(p, "-r", 2)) {
      sampleRate = strtol(p+2, NULL, 0);
    } else {
      parameterStack[parametersInStack++] = atof(p);
#if 0
    } else {
      printf("E: Unknown parameter \"%s\"\n", p);
      goto finally;
#endif
    }
    p += strlen(p)+1;
  }

  if (sampleRate) {
    sampleRate = -sampleRate; /* Negative means try making audio 32 bits */
    if (ioctl(stdaudioout, IOCTL_AUDIO_SET_RATE_AND_BITS, (void *)(&sampleRate))) {
      if (only16BitsAvailable =
	  ioctl(stdaudioout, IOCTL_AUDIO_SET_BITS, (void *)32)) {
	printf("Warning: Couldn't set 32-bit mode, using 16 bits.\n");
      }
      sampleRate = -sampleRate;
      if (ioctl(stdaudioout, IOCTL_AUDIO_SET_ORATE, (void *)(&sampleRate))) {
	printf("Warning: Couldn't set sample rate to %ld Hz.\n", sampleRate);
      }
    }
  } else {
    if (only16BitsAvailable =
	ioctl(stdaudioout, IOCTL_AUDIO_SET_BITS, (void *)32)) {
      printf("Warning: Couldn't set 32-bit mode, using 16 bits.\n");
    }
  }

  if (ioctl(stdaudioout, IOCTL_AUDIO_GET_ORATE, (void *)(&sampleRate))) {
    printf("Warning: Couldn't get sample rate, assuming 48 kHz\n");
    sampleRate = 48000;
  }

#if 0
  ioctl(stdaudioout, IOCTL_AUDIO_SET_BITS, (void *)16);
  only16BitsAvailable = 1;
#endif

  if (!parametersInStack) {
    sines = 1;
    sine[0].freq = 1002.1371;
    sine[0].left_dB  = -12.0;
    sine[0].right_dB = -12.0;
  } else {
    double __mem_y *dp = parameterStack;
    for (i=0; i<parametersInStack/3; i++) {
      sine[i].freq = *dp++;
      sine[i].left_dB  = *dp++;
      sine[i].right_dB = *dp++;
    }
    sines = i;
  }

  for (i=0; i<sines; i++) {
    if (sine[i].left_dB <= 0.0) {
      sine[i].leftLin  = pow(10.0, sine[i].left_dB/20.0);
      totLeft += sine[i].leftLin;
      sine[i].leftLin *= 2147000000.0;
    } else {
      sine[i].left_dB = -999.99;
    }
    if (sine[i].right_dB <= 0.0) {
      sine[i].rightLin = pow(10.0, sine[i].right_dB/20.0);
      totRight += sine[i].rightLin;
      sine[i].rightLin *= 2147000000.0;
    } else {
      sine[i].right_dB = -999.99;
    }
    sine[i].phaseAdd = 2*M_PI*sine[i].freq/sampleRate;
  }

  {
    double tot = MAX(totLeft, totRight);
    if (tot > 1.0) {
      printf("Too large total sine energy, scaling down by %3.2f dB\n",
	     20.0*log(tot)/log(10.0));
      for (i=0; i<sines; i++) {
	sine[i].leftLin /= tot;
	sine[i].rightLin /= tot;
      }
    }
  }

  if (verbose) {
    printf("Sample rate %ld Hz, %d bits\n",
	   sampleRate, only16BitsAvailable ? 16 : 32);
    for (i=0; i<sines; i++) {
      printf("%d: Sine %10.4f Hz, left %7.2f dB, right %7.2f dB\n",
	     i, sine[i].freq, sine[i].left_dB, sine[i].right_dB);
#if 0
      printf("phaseadd %f, leftlin %f, rightlin %f\n",
	     phaseAdd, leftLin, rightLin);
#endif
    }
  }

  memset(myAudioBuffer, 0, sizeof(myAudioBuffer));

  while (!(appFlags & APP_FLAG_QUIT)) {
    int ch;
    u_int32 newTC = ReadTimeCount();

    if (newTC - oldTC >= TICKS_PER_SEC) {
      u_int32 newUnderflows;
      if (ioctl(stdaudioout, IOCTL_AUDIO_GET_UNDERFLOWS,
		(void *)(&newUnderflows)) >= 0) {
	if (underflows && newUnderflows != underflows) {
	  printf("E: %ld new audio underflows, cannot play!\n",
		 newUnderflows - underflows);
	  goto finally;
	}
	underflows = newUnderflows;
      }
      oldTC += TICKS_PER_SEC;
    }

    if (only16BitsAvailable) {
      ringcpy((u_int16 *)myAudioBuffer, 1,
	      (u_int16 *)myAudioBuffer+1, 2, MY_AUDIO_BUFFER_SIZE);
      fwrite(myAudioBuffer, sizeof(s_int16), MY_AUDIO_BUFFER_SIZE, stdaudioout);
    } else {
      fwrite(myAudioBuffer, sizeof(s_int32), MY_AUDIO_BUFFER_SIZE, stdaudioout);
    }

    memset(myAudioBuffer, 0, sizeof(myAudioBuffer));

    for (ch=0; ch<sines; ch++) {
      s_int32 *bp = myAudioBuffer;
      double phase = sine[ch].phase;
      double phaseAdd = sine[ch].phaseAdd;
      double leftLin = sine[ch].leftLin;
      double rightLin = sine[ch].rightLin;

      for (i=0; i<MY_AUDIO_BUFFER_SIZE; i+=2) {
	double tt = sin(phase);
	if ((phase += phaseAdd) >= M_PI) {
	  phase -= 2*M_PI;
	}
	*bp++ += (s_int32)(tt*leftLin);
	*bp++ += (s_int32)(tt*rightLin);
      }

      sine[ch].phase = phase;
      sine[ch].phaseAdd = phaseAdd;
      sine[ch].leftLin = leftLin;
      sine[ch].rightLin = rightLin;
    }
  }

  if (only16BitsAvailable) {
    /* Write a bufferful of zeroes. */
    memset(myAudioBuffer, 0, MY_AUDIO_BUFFER_SIZE);
    fwrite(myAudioBuffer, sizeof(s_int16), MY_AUDIO_BUFFER_SIZE, stdaudioout);
  } else {
    /* Fade out current buffer value for slightly less pop. */
    s_int32 *bp = myAudioBuffer;
    for (i=MY_AUDIO_BUFFER_SIZE-1; i>=0; i--) {
      *bp = ((*bp>>16)*i/MY_AUDIO_BUFFER_SIZE) << 16;
      bp++;
    }
    fwrite(myAudioBuffer, sizeof(s_int32), MY_AUDIO_BUFFER_SIZE, stdaudioout);
  }

 finallyok:
  ret = S_OK;
 finally:
  return ret;
}
