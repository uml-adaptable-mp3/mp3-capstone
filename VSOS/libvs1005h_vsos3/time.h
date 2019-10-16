#ifndef _TIME_H_
#define _TIME_H_

#include <vstypes.h>
#include <stdlib.h>

struct tm {
  int tm_sec;	/* Seconds after the minute, 0..61 (typ. 0..59) */
  int tm_min;	/* Minutes after the hour, 0..59 */
  int tm_hour;	/* Hours since midnight, 0..23 */
  int tm_mday;  /* day of the month, 1..31 */
  int tm_mon;	/* months since January, 0..11 */
  int tm_year;	/* years since 1900 */
  int tm_wday;	/* days since Sunday, 0..6 */
  int tm_yday;	/* days since January 1, 0..365 */
  int tm_isdst;	/* Daylight Saving Time flags, 0..1 */
};

typedef s_int32 clock_t;
typedef s_int32 time_t;

/* The accuracy of the RTC that is used as a base for clock() is 1/128s.
   Thus, CLOCKS_PER_SEC for clock_t is 128. Note that clock_t can and will
   run through its 32-bit range every 388 days. */
#define CLOCKS_PER_SEC 128
/* clock() returns time in clock_t, which runs at a speed of CLOCKS_PER_SEC. */
clock_t clock(void);

#define TIME_NOT_FOUND -2
#define TIME_NOT_SET -1
/* time() returns current calendar time or negative if time not available
   (possible error codes are TIME_NOT_FOUND and TIME_NOT_SET).
   It tp is non-null, return value is also assigned to *tp. */
time_t time(register time_t *tp);

/* difftime() returns time2-time1 in seconds */
double difftime(register time_t time2, register time_t time1);

/* mktime() converts local time in the structure *tp into calendar time.
   Is the counterpart to localtime(). */
time_t mktime(register struct tm *tp);

/* asctime() converts the time in *tp into a string of form
   "Sun Jan  3 15_:14:13 1988\n" */
char *asctime(register const struct tm *tp);

/* ctime() converts the time in *tp into a string of local time form
   "Sun Jan  3 15_:14:13 1988\n" */
#if 0
  char *ctime(register const struct tm *tp);
#else
  #define ctime(tp) (asctime(tp))
#endif

/* gmtime() converts calendar time in *tp into UTC */
#if 0
  struct tm *gmtime(register const time_t *tp);
#else
  #define gmtime(tp) (localtime(tp))
#endif

/* localtime() converts calendar time in *tp into local time.
   Is the counterpart to mktime(). */
struct tm *localtime(register const time_t *tp);

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
size_t strftime(register char *s, register size_t smax, register const char *fmt, register const struct tm *tp);

/* Compatible with vo_get_time_from_rtc_hook in vsos.h */
void *VoGetTimeFromRtc(void);


/* Internal definitions, not part of time.h standard */
extern int __mem_y monLen[2][12];
#endif /* !_TIME_H */
