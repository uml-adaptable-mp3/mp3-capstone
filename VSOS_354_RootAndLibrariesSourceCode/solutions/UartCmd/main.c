/*

  Example for VSOS Uart Shell command

  The resulting .DL3 should be copied to the SYS/ folder of the system
  drive of the VS1005 VSOS3 device.
  
*/
#include <vo_stdio.h>
#include <string.h>
#include <apploader.h>
#include <consolestate.h>
#include <kernel.h>

ioresult main(char *parameters) {
  int nParam, i;
  char *p = parameters;
  ioresult ret = S_ERROR;

  /* Handle parameters */
  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: ProgramName [-h]\n"
	     "-h\tShow this help\n");
      ret = S_OK;
      goto finally;
    } else {
      printf("E: Unknown parameter \"%s\"\n", p);
      ret = S_ERROR;
      goto finally;
    }
    p += strlen(p)+1;
  }

  /* Action goes here */

 finally:
  /* Clean-up, like closing files comes here. */

  return ret;
}
