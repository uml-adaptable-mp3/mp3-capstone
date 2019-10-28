#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec.h>
#include "time.h"
#include "rtc.h"

/* Number of days in each month of the year. */
int __mem_y monLen[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};
