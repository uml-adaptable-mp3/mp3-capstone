/*
  NOTE!
  This driver is specific to VS1005g because it uses ROM symbols
  clockSpeed, AudioBufFree, and hwSampleRate.
 */
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <stdlib.h>
#include <timers.h>
#include <audio.h>
#include "time.h"
#include "rtc.h"

#if 0
u_int16 audioMutex;
#endif
FILE *audio_orig = NULL;

//void (*RomStart)(void) = (void (*)(void)) 0x8000;


#if 0
DLLENTRY(init)
ioresult init(char *parameters) {
}
#endif

char *wday[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void PrintTime(time_t tt) {
  struct tm *tm = localtime(&tt);
  printf("  %04d-%02d-%02d at %02d:%02d:%02d, d=%s, yd=%d, dst=%d\n",
	 tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
	 tm->tm_hour, tm->tm_min, tm->tm_sec,
	 wday[tm->tm_wday], tm->tm_yday, tm->tm_isdst);
}

DLLENTRY(main)
ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int autoPrint = 1;
  int i;
  static char s[256];
  time_t tim = time(NULL);

  if (tim == TIME_NOT_FOUND) {
    printf("ERROR! Could not find RTC!\n"
	   "Please check that you have put an LR44 battery to the\n"
	   "BATT1 connector on the VS1005g developer board, or\n"
	   "otherwise powered it up!\n");
    return EXIT_FAILURE;
  } else if (tim == TIME_NOT_SET) {
    printf("ERROR! RTC not set. Please use VSOS Shell program SetDate\n"
	   "or C function SetRtc() (e.g. SetRtc(mktime(tm))) to set RTC!\n");
    return EXIT_FAILURE;
  }

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: Date [formatString] [-t|-q|-h]\n"
	     "-t\tPrint uptime timer\n"
	     "-q\tQuiet mode\n"
	     "-h\tShow this help\n\n"
	     "FormatString:\n"
	     "%%a wdayName   %%A weekdayName   %%b monName    %%b monthName\n"
	     "%%c date&time  %%d dayOfMonth    %%H hour-24    %%h hour-12\n"
	     "%%j dayOfYear  %%m month         %%M minute     %%p AM/PM\n"
	     "%%S second     %%w weekDay,S=0   %%x date       %%X time\n"
	     "%%y yr         %%Y year          %%Z UTC        %%%% %%\n");
      return S_OK;
    } else if (!strcmp(p, "-q")) {
      goto finally;
    } else if (!strcmp(p, "-t")) {
      if (!autoPrint) {
	putchar(' ');
      }
      printf("%5.3fs", ReadTimeCount()*0.001);
      autoPrint = 0;
    } else {
      if (!autoPrint) {
	putchar(' ');
      }
      strftime(s, 256, p, localtime(&tim));
      fputs(s, stdout);
      autoPrint = 0;
    }
    p += strlen(p)+1;
  }

  if (autoPrint) {
    strftime(s, 256, "%c", localtime(&tim));
    puts(s);
  } else {
    putchar('\n');
  }

#if 0
  {
    u_int32 seconds = GetRtc();
    printf("rtc_time %ld = %lx, rtc_frac_time %d\n",
	     seconds, seconds, GetRtc128());
  }

  {
    struct tm tm = {
      0, 0, 0,		/* sec (0), min (0), hr (0) */
      1, 1-1,		/* mday (1), mon (0) */
      2000-1900,	/* y-1900 */
      0, 0, 0		/* wday (S=0), yday (0), isdst */
    };
    time_t t2 = mktime(&tm);
    printf("Time #1 = %ld = %lx\n", t2, t2);
    PrintTime(t2);
  }

  {
    struct tm tm = {
      56, 34, 12,		/* sec (0), min (0), hr (0) */
      17, 9-1,		/* mday (1), mon (0) */
      2015-1900,	/* y-1900 */
      0, 0, 0		/* wday (S=0), yday (0), isdst */
    };
    static char s[256];
    time_t t2 = mktime(&tm);
    printf("Time #2 = %ld = %lx\n", t2, t2);
    PrintTime(t2);
    printf("Strtime returns: %d\n", strftime(s, 256, "1: %a %A %b %B \"%c\"\n2: %d %H %h %j %m %M %p %S\n3: %U %w %W %x %X %y %Y %Z %%", &tm));
    printf("strlen = %d\n", strlen(s));
    printf("%s\n", asctime(&tm));
    puts(s);
  }

  for (i=1; i<=12; i+=4) {
    struct tm tm = {
      0, 0, 0,		/* sec (0), min (0), hr (0) */
      1, 0,		/* mday (1), mon (0) */
      2000-1900,	/* y-1900 */
      0, 0, 0		/* wday (S=0), yday (0), isdst */
    };
    time_t t2;
    tm.tm_mon = i-1;
    t2 = mktime(&tm);
    printf("Time for month %d = %ld = %lx\n", i, t2, t2);
    PrintTime(t2);
  }

  for (i=2000; i<=2015; i+=5) {
    struct tm tm = {
      0, 0, 0,		/* sec (0), min (0), hr (0) */
      1, 0,		/* mday (1), mon (0) */
      2000-1900,	/* y-1900 */
      0, 0, 0		/* wday (S=0), yday (0), isdst */
    };
    time_t t2;
    tm.tm_year = i-1900;
    t2 = mktime(&tm);
    printf("Time for year %d = %ld = %lx\n", i, t2, t2);
    PrintTime(t2);
  }

  {
    u_int32 clocks[2];
    clocks[0] = clock();
    Delay(1000);
    clocks[1] = clock();
    printf("Cl = %ld %ld, diff %ld\n",
	   clocks[1], clocks[0], clocks[1]-clocks[0]);
  }
#endif

 finally:
  return S_OK;
}


#if 0
DLLENTRY(fini)
void fini(void) {
  // Add code here to force release of resources such as 
  // memory allocated with malloc, entry points, 
  // hardware locks or interrupt handler vectors.
}
#endif
