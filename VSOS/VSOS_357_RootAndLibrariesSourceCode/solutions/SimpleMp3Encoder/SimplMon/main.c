/*

  Audio monitor for Simple Mp3/Ogg recorder for VS1205 / VS1005.
  VLSI Solution 2015-10-20 HH

  This VSOS application reads audio from VS1005g's default input
  (typically line input) and writes it to the default output.
  If the user selects "Record", the application loads S:SimplRec.ap3,
  which in turn will start actual recording.

*/
#include <vo_stdio.h>
#include <stdlib.h>
#include <sysmemory.h>
#include <timers.h>
#include <stdbuttons.h>
#include <aucommon.h>
#include <audio.h>
#include <lcd.h>
#include <rgb565.h>
#include <vsos.h>
#include <apploader.h>
#include <string.h>
#include <vo_gpio.h>
#include <kernel.h>


/* ----- USER ADJUSTABLE PARAMETERS START HERE!!! ----- */
#if 1
#  define USE_LCD_TOUCH
#endif
/* ----- USER ADJUSTABLE PARAMETERS END HERE!!! ----- */


enum butActions {
  baQuit = 1,
  baRecord,
  baPause
};

#ifndef MIN
#  define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#  define MAX(a,b) (((a)>(b))?(a):(b))
#endif

StdButton buttons[9+1] = {0};

ioresult main(void) {
  // Remember to never allocate buffers from stack space. So, if you
  // allocate the space inside your function, never forget "static"!
  int i;
  ioresult retVal = S_ERROR;
  u_int32 lastUITimeCount = ReadTimeCount();
  u_int32 lastSeconds = 0, sampleRate;
  StdButton *textArea, *vuButton, *timeButton, *bitRateButton, *fileNameButton;
  StdButton *audioMissingButton;
  StdButton *currentButton = buttons;
  void *fileBufLib = NULL, *encLib = NULL;
  char *fileName = "<none>";
  struct AutoBufferElement *abe = NULL;
  int fileNumber = 0;
  int nextFile = 1;
  int oldPauseMode;
  int quit = 0;

  /* Prepare screen */
  ioctl(&console, IOCTL_START_FRAME, "MP3/Ogg Vorbis Encoder Monitor");
  SetVirtualResolution(12,9);

  fileNameButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_TEXT|BTN_NO_BEVEL|BTN_DISABLED,  0,5, 4,1, fileName);
  timeButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_TEXT|BTN_NO_BEVEL|BTN_DISABLED,  4,5, 4,1, NULL);
  bitRateButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_TEXT|BTN_NO_BEVEL|BTN_DISABLED,  8,5, 4,1, NULL);
  audioMissingButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_TEXT|BTN_NO_BEVEL|BTN_DISABLED,  2,7, 8,2, NULL);
  vuButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_INVISIBLE,  0,6,12,1, NULL);
#ifdef USE_LCD_TOUCH
  CreateStdButton(currentButton++, baRecord,  BTN_NORMAL,    0,7, 3,2, "Record");
  //  CreateStdButton(currentButton++, baPause, BTN_CHECKABLE, 3,7, 3,2, "Pause");
  CreateStdButton(currentButton++, baQuit,  BTN_NORMAL,    9,7, 3,2, "Quit");
#endif
  textArea=currentButton;
  CreateStdButton(currentButton++, BTN_END, BTN_INVISIBLE|BTN_TEXT,
                  0,0, 12,5, "");

  SetClippingRectangleToButton(textArea);

  RenderStdButtons(buttons);

  printf("VSOS 3.25 Simple Encoder Monitor v1.04\n"
         "2014-2015 VLSI Solution\n\n");
#ifndef USE_LCD_TOUCH
  printf("Push S1 to record, S4 to quit\n\n");
#endif

  if (!stdaudioin || !stdaudioout) {
    printf("No audio in/out!\n");
    retVal = EXIT_FAILURE;
    goto finally;
  }


  ioctl(stdaudioin, IOCTL_AUDIO_GET_IRATE, (void *)(&sampleRate));
#ifdef BITRATE_KBPS
  printf("Monitoring at %ld Hz %s\n", sampleRate);
#endif

  ioctl(stdaudioout, IOCTL_AUDIO_SET_ORATE, &sampleRate);

  RenderStdButton(fileNameButton);

  while (!quit) {
    u_int32 tmpTimeCount = ReadTimeCount();
#define AUDIO_BUF_SIZE 128
    static u_int16 audioBuf[AUDIO_BUF_SIZE];

    fread(audioBuf, sizeof(audioBuf[0]), AUDIO_BUF_SIZE, stdaudioin);
    fwrite(audioBuf, sizeof(audioBuf[0]), AUDIO_BUF_SIZE, stdaudioout);
    

    /* If there is data to write, and less then 1/50 seconds since user
       interface was last handled... */
    if (tmpTimeCount-lastUITimeCount >= TICKS_PER_SEC/50) {
#ifndef USE_LCD_TOUCH
      static u_int16 lastButtonPress = 0;
#endif
      u_int16 buttonPress;


#ifdef USE_LCD_TOUCH
      buttonPress = GetStdButtonPress(buttons);
#else
      /* If no touch interface, read button S1 for Next, S4 for Quit. */
      buttonPress = 0;

      if (GpioReadPin(0x00)) {
	buttonPress = baRecord;
      } else if (GpioReadPin(0x01)) {
	buttonPress = baPause;
      } else if (GpioReadPin(0x03)) {
	buttonPress = baQuit;
      } else {
	buttonPress = 0;
      }
      if (buttonPress == lastButtonPress) {
	buttonPress = 0; /* If key (e.g. baRecord) is being held, don't react.*/
      } else {
	lastButtonPress = buttonPress;
      }
#endif

      /* Handle user interface */
      switch (buttonPress) {
      case baQuit:
	quit = -1;
	break;
      case baRecord:
	quit = 1;
	fclose(appFile);
	appFile = fopen("S:SIMPLREC.AP3", "rb");
	break;
      }
      lastUITimeCount = tmpTimeCount;
    }
  } /* while (!quit) */
  retVal = S_OK;


 finally:
  return retVal;
}
