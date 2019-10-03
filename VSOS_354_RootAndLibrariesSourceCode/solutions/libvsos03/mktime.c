/* free mktime function
   Copyright 1988, 1989 by David MacKenzie <djm@ai.mit.edu>
   and Michael Haertel <mike@ai.mit.edu>
   Unlimited distribution permitted provided this copyright notice is
   retained and any functional modifications are prominently identified.  */

/* Note: This version of mktime is ignorant of the tzfile; it does not
   return correct results during the few hours around when daylight savings
   time goes in to or out of effect.  It also does not allow or adjust
   for invalid values in any of the fields, contrary to the ANSI C
   specification. */

/* Note 2015-09-17: Code changed to compile for the VLSI Solution VSOS
   environment; mainly from K&R C to ANSI C. Also made epoch 1.1.2000 instead
   of 1.1.1970. */

#include <vo_stdio.h>
#include "time.h"

time_t mkgmtime (register struct tm *tm);

/* Return the equivalent in seconds past 12:00:00 a.m. Jan 1, 1970 GMT
   of the local time and date in the exploded time structure `tm',
   and set `tm->tm_yday', `tm->tm_wday', and `tm->tm_isdst'.
   Return -1 if any of the other fields in `tm' has an invalid value. */

time_t mktime (register struct tm *tm) {
  struct tm save_tm;            /* Copy of contents of `*tm'. */
  struct tm *ltm;               /* Local time. */
  time_t then;                  /* The time to return. */

  then = mkgmtime (tm);
  if (then == -1)
    return -1;

#if 0
  /* In case `tm' points to the static area used by localtime,
     save its contents and restore them later. */
  save_tm = *tm;
  /* Correct for the timezone and any daylight savings time.
     If a change to or from daylight savings time occurs between when
     it is the time in `tm' locally and when it is that time in Greenwich,
     the change to or from dst is ignored, but that is a rare case. */
  then += then - mkgmtime (localtime (&then));

  ltm = localtime (&then);
  save_tm.tm_yday = ltm->tm_yday;
  save_tm.tm_wday = ltm->tm_wday;
  save_tm.tm_isdst = ltm->tm_isdst;
  *tm = save_tm;
#endif

  return then;
}

/* Nonzero if `y' is a leap year, else zero. */
/* Valid between 1901 to 2099, which is enough for us */
#define leap(y) (((y) & 3) == 0)

/* Number of leap years from 1970 to `y' (not including `y' itself). */
/* Valid between 1901 to 2099, which is enough for us */
#define nleap(y) (((y) - 1997) / 4)


/* Number of days in each month of the year. */
static char __mem_y monlens[] =
{
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* Return the equivalent in seconds past 12:00:00 a.m. Jan 1, 2000 GMT
   of the Greenwich Mean time and date in the exploded time structure `tm',
   and set `tm->tm_yday', `tm->tm_wday', and `tm->tm_isdst'.
   Return -1 if any of the other fields in `tm' has an invalid value. */

time_t mkgmtime (register struct tm *tm) {
  int years, months;
  volatile int days;
  time_t res;

  years = tm->tm_year + 1900;   /* year - 1900 -> year */
  months = tm->tm_mon;          /* 0..11 */
  days = tm->tm_mday - 1;       /* 1..31 -> 0..30 */

  if (years < 2000
      || months < 0 || months > 11
      || days < 0
      || days > monlens[months] + (months == 1 && leap (years)) - 1
      || tm->tm_hour < 0 || tm->tm_hour > 23
      || tm->tm_min < 0 || tm->tm_min > 59
      || tm->tm_sec < 0 || tm->tm_sec > 61) {
    return -1;
  }
  
  res = 3600 * (s_int32)tm->tm_hour + (60 * tm->tm_min + tm->tm_sec);

  /* Set `days' to the number of days into the year. */
  if (months > 1 && leap (years))
    ++days;
  while (months-- > 0)
    days += monlens[months];
  tm->tm_yday = days;

  /* Now set `days' to the number of days since Jan 1, 2000. */
  days += 365 * (years - 2000) + nleap (years);
#if 0
  printf("#DAYS%d#HR%d#MIN%d#SEC%d#", days, tm->tm_hour, tm->tm_min, tm->tm_sec);
#endif
  tm->tm_wday = (days + 6) % 7; /* Jan 1, 2000 was Saturday. */
  tm->tm_isdst = 0;

  res +=  (s_int32)days * 86400;

  return res;
}
