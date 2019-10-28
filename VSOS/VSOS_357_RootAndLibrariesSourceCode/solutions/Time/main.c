/*

  Time runs a program and measures the time it took to run it.
  
*/
#include <vo_stdio.h>
#include <stdlib.h>
#include <apploader.h>
#include <consolestate.h>
#include <string.h>
#include <stdlib.h>
#include <timers.h>
#include <audio.h>
#include <kernel.h>



ioresult main(char *parameters) {
  ioresult retVal = S_OK;
  u_int32 startTime, endTime;
  char *p;

  if (!strcmp(parameters, "-h") || !*parameters) {
    printf("Usage: time [program [parameters]|-h]\n"
	   "-h\tShow this help\n");
    goto finally;
  }

  if ((p = strchr(parameters, ' '))) {
    /* At this point, variable parameters becomes the program name,
       and p becomes the pointer to the parameters to that program. */
    *p++ = '\0';
  } else {
    p = "";
  }

  startTime = ReadTimeCount();
  retVal = RunProgram(parameters, p);
  endTime = ReadTimeCount();

  printf("errCode %d, time: %6.3fs\n",
	 retVal, (endTime-startTime)*(1.0/TICKS_PER_SEC));

 finally:
  return retVal;
}
