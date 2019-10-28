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




const char * __mem_y wdayNames[7] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

const char * __mem_y monNames[12] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};

const char * __mem_y digFmt[5] = {
  "%d", "%1d", "%02d", "%03d", "%04d"
};




/* Helper function for strftime() */
char *StrfCondPrintS(register char *d, register size_t *smax,
		     register s_int16 n, register const char *s) {
  while (*smax && n && *s) {
    *d++ = *s++;
    (*smax)--;
    n--;
  }
  return d;
}

/* Helper function for strftime() */
char *StrfCondPrint(register char *d, register size_t *smax,
		   register s_int16 digits, register s_int16 val) {
  char s[5];
  sprintf(s, digFmt[digits], val);
  return StrfCondPrintS(d, smax, digits, s);
}





/* clock() returns time in clock_t. In VSOS CLOCKS_PER_SEC is 1 */
clock_t clock(void) {
  u_int32 oldS, newS;
  u_int16 fractions;

 /* To guarantee there are no long pauses while running these functions
    calls, we'll forbid multitasking for a short while (a few microseconds,
    tops). */
  Forbid();
  oldS = GetRtc();
  fractions = GetRtc128();
  newS = GetRtc();
  Permit();
  if (oldS == RTC_NOT_SET || oldS == RTC_NOT_FOUND) {
    return oldS;
  }
  if (fractions < 64) {
    oldS = newS;
  }
  return (oldS << 7) | fractions;
}

/* time() returns current calendar time or -1 if time not available. It tp is
   non-null, return value is also assigned to *tp. */
time_t time(register time_t *tp) {
  time_t t = GetRtc();
  if (tp) {
    *tp = t;
  }
  return t;
}

/* difftime() returns time2-time1 in seconds */
double difftime(register time_t time2, register time_t time1) {
  return (double)time2-(double)time1;
}

/* asctime() converts the time in *tp into a string of form
   "Sun Jan  3 15:14:13 1988\n" */
char *asctime(register const struct tm *tp) {
  static char asctimeStr[25];
  strftime(asctimeStr, 25, "%a %b %d %X %Y", tp);
  /* Remove '0' from day of month if necessary. */
  if (asctimeStr[8] == '0') {
    asctimeStr[8] = ' ';
  }
  return asctimeStr;
}

/* ctime() converts the time in *tp into a string of local time form
   "Sun Jan  3 15:14:13 1988\n" */
#if 0
/* Same implementation as for asctime */
char *ctime(register const struct tm *tp) {
  return NULL;
}
#endif

/* gmtime() converts calendar time in *tp into UTC */
#if 0
/* Same implementation as for localtime */
struct tm *gmtime(register const time_t *tp) {
  return NULL;
}
#endif

/* localtime() converts calendar time in *tp into local time.
   Is the counterpart to mktime(). */
struct tm *localtime(register const time_t *tp) {
  static struct tm tm;
  time_t t = *tp, nextT;

  /* Quick and dirty implementation. Shouldn't reinvent the wheel, but
     here we go. */
  memset(&tm, 0, sizeof(tm));
  if (t < 0 && t > -128) {
    /* Invalid clock; return 1999-31-12 12:00:00 */
    tm.tm_year = 99;
    tm.tm_mon = 12-1;
    tm.tm_mday = 31;
    tm.tm_yday = 364;
    tm.tm_hour = 12;
    goto finally;
  }
  tm.tm_wday = ((u_int16)(t/86400) + 4) % 7;

  tm.tm_year=100;
  while ((nextT = t - (365+leap(tm.tm_year+1900))*86400) >= 0) {
    //    printf("    year %d, nextT %ld, t %ld\n", tm.tm_year+1900, nextT, t);
    tm.tm_year++;
    t = nextT;
  }
  tm.tm_yday = (int)(t/86400);

  while ((nextT = t - monLen[leap(tm.tm_year+1900)][tm.tm_mon]*86400) >= 0) {
    //    printf(" mon %d, nextT %ld, t %ld\n", i, nextT, t);
    tm.tm_mon++;
    t = nextT;
  }

  tm.tm_mday = (int)(t/86400);
  t -= tm.tm_mday*86400;

  tm.tm_hour = (int)(t/3600);
  t -= tm.tm_hour*3600;

  tm.tm_min = (int)t/60;
  tm.tm_sec = (int)t - tm.tm_min*60;

  tm.tm_mday++;

 finally:
  return &tm;
}



/* strftime() formats date and time information from *tp into s according to
   fmt, analugous to printf() format. Each %c is replaced as described below.
   %a abbreviated weekday name
   %A full weekday name
   %b abbreviated month name
   %B full month name
   %c local date and time representation
   %d day of the month, 01..31
   %H hour, 00..23
   %h hour, 01..12
   %j day of the year, 001..366
   %m month, 01..12
   %M minute, 00..59
   %p AM / PM
   %S second, 00..61
   %U week number of the year, Sunday as first day of week, 00..53 (not supported)
   %w weekday, 0..6, Sunday = 0
   %W week number of the year, Monday as first day of week, 00..53 (not supported)
   %x local date representation
   %X local time representation
   %y year withour century, 00..99
   %Y year with century
   %Z Time zone name, if any (not supported, always displays "UTC")
   %% %
*/
size_t strftime(register char *d, register size_t smax,
		register const char *fmt, register const struct tm *tp) {
  int esc=0;
  int c;
  size_t origSMax = --smax;
  if (smax < 0) {
    return 0;
  }
  while (smax > 0 && (c = *fmt++)) {
    size_t smaxCopy = smax;
    if (esc) {
      switch(c) {
      case 'a':
	d = StrfCondPrintS(d, &smaxCopy, 3, wdayNames[tp->tm_wday]);
	break;
      case 'A':
	d = StrfCondPrintS(d, &smaxCopy, 255, wdayNames[tp->tm_wday]);
	break;
      case 'b':
	d = StrfCondPrintS(d, &smaxCopy, 3, monNames[tp->tm_mon]);
	break;
      case 'B':
	d = StrfCondPrintS(d, &smaxCopy, 255, monNames[tp->tm_mon]);
	break;
      case 'c':
	{
	  char t[20];
	  sprintf(t, "%04d-%02d-%02d %02d:%02d:%02d",
		  tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday,
		  tp->tm_hour, tp->tm_min, tp->tm_sec);
	  d = StrfCondPrintS(d, &smaxCopy, 19, t);
	}
	break;
      case 'd':
	d = StrfCondPrint(d, &smaxCopy, 2, tp->tm_mday);
	break;
      case 'H':
	d = StrfCondPrint(d, &smaxCopy, 2, tp->tm_hour);
	break;
      case 'h':
	{
	  int hour = tp->tm_hour % 12;
	  d = StrfCondPrint(d, &smaxCopy, 2, hour ? hour : 12);
	}
	break;
      case 'j':
	d = StrfCondPrint(d, &smaxCopy, 3, tp->tm_yday+1);
	break;
      case 'm':
	d = StrfCondPrint(d, &smaxCopy, 2, tp->tm_mon+1);
	break;
      case 'M':
	d = StrfCondPrint(d, &smaxCopy, 2, tp->tm_min);
	break;
      case 'p':
	d = StrfCondPrintS(d, &smaxCopy, 2, (tp->tm_hour < 12) ? "AM" : "PM");
	break;
      case 'S':
	d = StrfCondPrint(d, &smaxCopy, 2, tp->tm_sec);
	break;
      case 'U':
      case 'W':
	d = StrfCondPrintS(d, &smaxCopy, 2, "-1");
	break;
      case 'w':
	d = StrfCondPrint(d, &smaxCopy, 2, tp->tm_wday);
	break;
      case 'x':
	{
	  char t[11];
	  sprintf(t, "%04d-%02d-%02d",
		  tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday);
	  d = StrfCondPrintS(d, &smaxCopy, 10, t);
	}
	break;
      case 'X':
	{
	  char t[9];
	  sprintf(t, "%02d:%02d:%02d",
		  tp->tm_hour, tp->tm_min, tp->tm_sec);
	  d = StrfCondPrintS(d, &smaxCopy, 8, t);
	}
	break;
      case 'y':
	d = StrfCondPrint(d, &smaxCopy, 2, tp->tm_year % 100);
	break;
      case 'Y':
	d = StrfCondPrint(d, &smaxCopy, 4, tp->tm_year+1900);
	break;
      case 'Z':
	d = StrfCondPrintS(d, &smaxCopy, 3, "UTC");
	break;
      default:
	*d++ = c;
	smax--;
	break;
      }
      esc = 0;
    } else if (c == '%') {
      esc = 1;
    } else {
      *d++ = c;
      smax--;
    }
    smax = MIN(smax, smaxCopy);
  }
  *d++ = '\0';
  return origSMax - smax;
}
