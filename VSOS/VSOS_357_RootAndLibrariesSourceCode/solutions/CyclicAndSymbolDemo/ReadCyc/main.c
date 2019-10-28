#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <apploader.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <clockspeed.h>
#include <audio.h>
#include <timers.h>
#include <dsplib.h>
#include <crc32.h>
#include <kernel.h>

DLLIMPORT(tenthsCounter)
extern u_int16 tenthsCounter;

ioresult main(char *parameters) {
  printf("Tenths counter %u\n", tenthsCounter);
  return S_OK;
}
