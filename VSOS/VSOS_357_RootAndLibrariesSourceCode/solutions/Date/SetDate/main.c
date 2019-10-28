/*
  NOTE!
  This VSOS Shell program is specific to VS1005g because it uses ROM symbols.
 */
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <stdlib.h>
#include <timers.h>
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
  int set = 0;
  int i;
  time_t tim = time(NULL);
  struct tm *tm = localtime(&tim);
  ioresult res = S_OK;

#if 0
  printf("%04d-%02d-%02d %02d:%02d:%02d\n",
	 tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
	 tm->tm_hour, tm->tm_min, tm->tm_sec);
#endif

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: SetDate [YYYY-MM-DD|YY-MM-DD|HH:MM:SS] [-h]\n"
	     "YYYY-MM-DD\tSet date, e.g. 2015-09-18\n"
	     "YY-MM-DD\tSet date, e.g. 15-09-18\n"
	     "HH:MM:SS\tSet time, e.g. 12:34:56\n"
	     "HH:MM\t\tSet time, e.g. 12:34\n"
	     "-h\t\tShow this help\n");
      return S_OK;
    } else {
      if (strlen(p) == 10 && p[4] == '-' && p[7] == '-')  {
	tm->tm_year = atoi(p+0)-1900;
	tm->tm_mon = atoi(p+5)-1;
	tm->tm_mday = atoi(p+8);
	set |= 1;
      } else if (strlen(p) == 8 && p[2] == '-' && p[5] == '-')  {
	tm->tm_year = atoi(p+0)+100;
	tm->tm_mon = atoi(p+3)-1;
	tm->tm_mday = atoi(p+6);
	set |= 1;
      } else if (strlen(p) == 8 && p[2] == ':' && p[5] == ':')  {
	tm->tm_hour = atoi(p+0);
	tm->tm_min = atoi(p+3);
	tm->tm_sec = atoi(p+6);
	set |= 2;
      } else if (strlen(p) == 5 && p[2] == ':')  {
	tm->tm_hour = atoi(p+0);
	tm->tm_min = atoi(p+3);
	tm->tm_sec = 0;
	set |= 2;
      } else {
	printf("E: Malformed parameter \"%s\"\n", p);
	res = S_ERROR;
	goto finally;
      }
    }
    p += strlen(p)+1;
  }


  if (set) {
    u_int32 t = (u_int32)mktime(tm);
    if (tim < 0 && set < 3) {
      printf("E: RTC not set. You need to set both date and time!\n");
      res = S_ERROR;
      goto finally;
    }
    if (SetRtc(t) != t) {
      printf("E: RTC not found. Check RTC battery!\n");
      res = S_ERROR;
      goto finally;
    }
  }

#if 0
  printf("%04d-%02d-%02d %02d:%02d:%02d\n",
	 tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
	 tm->tm_hour, tm->tm_min, tm->tm_sec);
#endif

 finally:
  return res;
}


#if 0
DLLENTRY(fini)
void fini(void) {
  // Add code here to force release of resources such as 
  // memory allocated with malloc, entry points, 
  // hardware locks or interrupt handler vectors.
}
#endif
