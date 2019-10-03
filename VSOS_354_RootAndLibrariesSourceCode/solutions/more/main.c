/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <strings.h>
#include <timers.h>
#include <sysmemory.h>
#include <consolestate.h>
#include <audio.h>
#include <ctype.h>
#include <kernel.h>



#define ONE_LINE_LEN 256
s_int16 oneLine[ONE_LINE_LEN];

s_int16 GetCoordinates(register s_int16 *x, register s_int16 *y) {
  u_int32 startTime;
  s_int16 *p = oneLine;

  memset(oneLine, 0, sizeof(oneLine));
  printf("\033[6n");
  startTime = ReadTimeCount();

  while (1) {
    if (ioctl(stdin, IOCTL_TEST, NULL) > 0) {
      if ((*p++ = fgetc(stdin)) == 'R') {
	break;
      }
      if (p >= oneLine+ONE_LINE_LEN) {
	return -1;
      }
    } else {
      /* Be multitasking friendly */ 
      Delay(1);
    }
    /* 100 ms timeout */
    if (ReadTimeCount() - startTime > TICKS_PER_SEC/10) {
      return -1;
    }
  } /* while(1) (with break) */

  if (oneLine[0] != '\033' || oneLine[1] != '[' || !(p=strchr(oneLine, ';'))) {
    return -1;
  }
  *y = atoi(oneLine+2);
  *x = atoi(p+1);

  return 0;
}




u_int16 hexBuf[16];

void PrintHexBuf(register u_int16 line, register u_int16 n) {
  int i;
  printf("%08lx  ", (s_int32)line*16);
  for (i=0; i<16; i++) {
    printf("%s", (i==8) ? "  " : " ");
    if (i<n) {
      printf("%02x", hexBuf[i]);
    } else {
      printf("  ");
    }
  }
  printf("  |");
  for (i=0; i<n; i++) {
    printf("%c", isprint(hexBuf[i]) ? hexBuf[i] : '.');
  }
  printf("|\n");
}




ioresult main(char *parameters) {
  FILE *fp = NULL;
  int c, i, rows, columns;
  char *p = parameters;
  int retVal = S_ERROR;
  int nextIsRows = 0, nextIsColumns = 0;
  int invertBS[3] = {0}; /* DEL = \[[3~, C-H = 0x08, Backspace = 0x7F */
  char *fileName = NULL;
  int x,y;
  int nParam;
  int line;
  int hexMode = 0;

  nParam = RunProgram("ParamSpl", parameters);

  for (i=0; i<nParam; i++) {
    if (nextIsRows > 0) {
      nextIsRows = -1;
      rows = atoi(p);
    } else if (nextIsColumns > 0) {
      nextIsColumns = -1;
      columns = atoi(p);
    } else if (!strcmp(p, "-h")) {
      printf("Usage: More [-r rows] [-c columns] [-x|+x] [-h] fileName\n"
	     "-x/+x\tHex mode on / off\n"
	     "-h\tShow this help\n");
      goto finally_ok;
    } else if (!strcmp(p, "-r")) {
      nextIsRows = 1;
    } else if (!strcmp(p, "-c")) {
      nextIsColumns = 1;
    } else if (!strcmp(p, "-x")) {
      hexMode = 1;
    } else if (!strcmp(p, "+x")) {
      hexMode = 0;
    } else if (!fileName) {
      fileName = p;
    } else {
      printf("E: Unknown parameter \"%s\"\n", p);
      goto finally;
    }
    p += strlen(p)+1;
  }

  if (!fileName) {
    printf("E: No file name provided!\n", fileName);
    goto finally;
  }

  fp = fopen(fileName, "r");
  if (!fp) {
    printf("E: Cannot open \"%s\"!\n", fileName);
    goto finally;
  }

  if (!nextIsRows && !nextIsColumns) {
    /* Determine terminal size */
    printf("\033[999;999H");
    GetCoordinates(&columns, &rows);
    printf("\033[999;1H");
  }

  if (columns < 20 || (hexMode && columns < 79)) {
    printf("E: Too narrow screen!\n");
    goto finally;
  }

  printf("\033[2J\033[f\033[1;1H");

#if 0
  GetCoordinates(&x, &y);
  printf("coord %d %d\n", x, y);
  printf("Size %dx%d\n", columns, rows);
#endif

  x=1; y=1;
  line = hexMode ? 0 : 1;

  while ((c = fgetc(fp)) != EOF) {
    if (hexMode) {
      hexBuf[x-1] = c;
      if (++x >= 17) {
	PrintHexBuf(line, 16);
	line++;
	x = 1;
	y++;
      }
      c = '\0';
    } else {
      putchar(c);
      if (isprint(c)) {
	if (++x > columns) {
	  y++;
	  x = 1;
	}
      } else {
	GetCoordinates(&x, &y);
      }
    }
    if (y >= rows) {
      int cc;
      printf("\033[7m--MORE--(%d)\033[27m", line);
      cc = fgetc(stdin);
      printf("\r                \r");
      if (cc == '\n') {
	printf("\n\033[1A");
      } else if (cc == 'q') {
	goto finally_ok;
      } else {
	printf("\033[2J\033[f\033[1;1H");
      }
      y = 1;
      x = 1;
    }
    if (c == '\n') {
      line++;
    }
  }
  if (hexMode && x > 1) {
    PrintHexBuf(line, x-1);
  }

 finally_ok:
  retVal = S_OK;
 finally:
  if (fp) {
    fclose(fp);
    fp = NULL;
  }

  return retVal;
}
