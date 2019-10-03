#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec.h>
#include "time.h"
#include "rtc.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


/* Nonzero if `y' is a leap year, else zero. */
/* Valid between 1901 to 2099, which is enough for us */
#define leap(y) (((y) & 3) == 0)

/* Number of leap years from 1970 to `y' (not including `y' itself). */
/* Valid between 1901 to 2099, which is enough for us */
#define nleap(y) (((y) - 1997) / 4)



/* localtime() converts calendar time in *tp into local time.
   Is the counterpart to mktime(). */
void *VoGetTimeFromRtc(void) {
  time_t t = GetRtc(), nextT;

  memset(&currentTime, 0, sizeof(currentTime));
  if (t < 0) {
    /* Invalid clock; return 1999-31-12 12:00:00 */
    currentTime.tm_year = 99;
    currentTime.tm_mon = 12-1;
    currentTime.tm_mday = 31;
    currentTime.tm_hour = 12;
    return;
  }

  currentTime.tm_year=100;
  while ((nextT = t - (365+leap(currentTime.tm_year+1900))*86400) >= 0) {
    currentTime.tm_year++;
    t = nextT;
  }

  while ((nextT = t - monLen[leap(currentTime.tm_year+1900)][currentTime.tm_mon]*86400) >= 0) {
    currentTime.tm_mon++;
    t = nextT;
  }

  currentTime.tm_mday = (int)(t/86400);
  t -= currentTime.tm_mday*86400;

  currentTime.tm_hour = (int)(t/3600);
  t -= currentTime.tm_hour*3600;

  currentTime.tm_min = (int)t/60;
  currentTime.tm_sec = (int)t - currentTime.tm_min*60;

  currentTime.tm_mday++;

  return;
}
