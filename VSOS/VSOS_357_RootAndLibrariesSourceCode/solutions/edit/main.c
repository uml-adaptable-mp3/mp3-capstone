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
s_int16 editLine[ONE_LINE_LEN];
s_int16 killLine[ONE_LINE_LEN];
s_int16 **d = NULL;
s_int16 __mem_y lines = 0, spaceForLines = 0;
s_int16 __mem_y topLine = 0, leftColumn = 0, cursorLine = 0, cursorColumn = 0;
s_int16 rows = 24, columns = 80;
u_int16 __mem_y insertMode = 1;
char *fileName = NULL;



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





s_int16 NewCurrentLine(register s_int16 newLineNumber) {
  static s_int16 oldCurrent = -1;
  if (oldCurrent >= 0) {
    s_int16 *t = realloc(d[oldCurrent], strlen(editLine)+1);
    if (!t) {
      printf("E: Out of memory!\n");
      return -1;
    }
    strcpy(t, editLine);
    d[oldCurrent] = t;
  }
  if (newLineNumber >= 0) {
    strcpy(editLine, d[newLineNumber]);
  }
  oldCurrent = newLineNumber;
  return 0;
}



s_int16 RemoveLine(register s_int16 lineToRemove) {
  if (NewCurrentLine(-1)) {
    return -1;
  }
  if (lineToRemove < lines-1) {
    s_int16 **p = d+lineToRemove;
    if (*p) {
      free(*p);
    }
    memmove(p, p+1, (lines-lineToRemove-1)*sizeof(d[0]));
  }
  lines--;
  return NewCurrentLine(cursorLine);
}



s_int16 InsertLine(register s_int16 insertBefore, register const s_int16 *s) {
  s_int16 **p;

  if (NewCurrentLine(-1)) {
    return -1;
  }
  if (lines >= spaceForLines) {
    spaceForLines = spaceForLines*2+16;
    p = realloc(d, spaceForLines*sizeof(d[0]));
    if (!p) {
      printf("E: Out of memory!\n");
      return -1;
    }
    d = p;
  }
  p = d+insertBefore;
  memmove(p+1, p, (lines-insertBefore)*sizeof(d[0]));
  *p = malloc(strlen(s)+1);
  if (!*p) {
    printf("E: Out of memory!\n");
    return -1;
  }
  strcpy(*p, s);
  lines++;
  return NewCurrentLine(cursorLine);
}



u_int16 CursorPhysicalColumn(void) {
  int i, res = 0;
  for (i=0; i<cursorColumn; i++) {
    if (d[cursorLine][i] == 9) { /* Tabulator */
      res = (res+8)&~7;
    } else {
      res++;
    }
  }
  return res;
}



void UpdateMenu(register const char *s) {
  static char menuBuf[128];
  int i;
  printf("\033[%d;1H\033[7m", rows);
  if (!s) {
    sprintf(menuBuf,
	    "Ctrl-P help, line %d/%d, insert %s, %s",
	    cursorLine+1, lines, insertMode ? "on" : "off", fileName);
  } else {
    strncpy(menuBuf, s, 127);
  }

  if (columns < 128) {
    menuBuf[columns] = '\0';
  }
  printf("%s", menuBuf);
  for (i=strlen(menuBuf)+1; i<columns; i++) {
    printf(" ");
  }
  printf("\033[m"
	 "\033[%d;%dH", cursorLine-topLine+1,
	 CursorPhysicalColumn()-leftColumn+1);
}



s_int16 UpdateScreen(register u_int16 wholeScreen) {
  int i;
  int startI=0, endI=rows-1;

  if (NewCurrentLine(-1)) {
    return -1;
  }

  i = CursorPhysicalColumn();
  while (i < leftColumn) {
    leftColumn -= columns/2;
    if (leftColumn < 0) {
      leftColumn = 0;
    }
    wholeScreen = 1;
  }
  while (i > leftColumn+columns-1) {
    leftColumn += columns/2;
    wholeScreen = 1;
  }

  if (!wholeScreen) {
    startI = cursorLine-topLine;
    endI = startI+1;
  }

  for (i=startI; i<endI; i++) {
    printf("\033[%d;1H"
	   "\033[2K", i+1);
    if (topLine+i < lines) {
      int col = 0, j;
      s_int16 *p = d[topLine+i];
      s_int16 c;
      while ((c = *p++)) {
	u_int16 left = 1;
	if (c == 9) { /* Tabulator */
	  c = ' ';
	  left = ((col+8)&~7) - col;
	}
	do {
	  if (col >= leftColumn && col < leftColumn + columns) {
	    fputc(c, stdout);
	  }
	  col++;
	} while (--left);
      }
    }
  }

  UpdateMenu(NULL);
  return NewCurrentLine(cursorLine);
}



s_int16 AddChar(register u_int16 c, register s_int16 insertMode) {
  if (cursorColumn < ONE_LINE_LEN-1) {
    s_int16 *p = editLine+cursorColumn++;
    if (insertMode) {
      editLine[ONE_LINE_LEN-2] = '\0';
      memmove(p+1, p, strlen(p)+1);
    }
    if (!*p) {
      /* Was at the end of line, need to add new nul terminator character */
      p[1] = '\0';
    }
    *p++ = c;
    if (insertMode >= 0 && UpdateScreen(0)) {
      return -1;
    }
  }
  return 0;
}



s_int16 DelChar(register s_int16 left) {
  s_int16 *p;
  if (left) {
    if (!cursorColumn) {
      if (cursorLine) {
	if (NewCurrentLine(--cursorLine)) {
	  return -1;
	}
	cursorColumn = strlen(editLine);
	strncat(editLine, d[cursorLine+1], ONE_LINE_LEN-1-strlen(editLine));
	if (RemoveLine(cursorLine+1) || UpdateScreen(1)) {
	  return -1;
	}
      }
      return 0;
    } else {
      cursorColumn--;
      printf("\033[1D");
    }
  }
  p = editLine+cursorColumn;
  if (*p) {
    memmove(p, p+1, strlen(p));
    if (UpdateScreen(0)) {
      return -1;
    }
  } else if (!left && cursorLine < lines-1) {
    strcat(p, d[cursorLine+1]);
    if (RemoveLine(cursorLine+1) || UpdateScreen(1)) {
      return -1;
    }
  }
  return 0;
}



ioresult main(char *parameters) {
  FILE *fp = NULL;
  u_int16 fileSize;
  u_int16 c;
  int i;
  int nParam, i;
  char *p = parameters;
  int retVal = S_ERROR;
  int nextIsRows = 0, nextIsColumns = 0;
  int lastCharWasCr = 0;
  int invertBS[3] = {0}; /* DEL = \[[3~, C-H = 0x08, Backspace = 0x7F */
 
  nParam = RunProgram("ParamSpl", parameters);

  for (i=0; i<nParam; i++) {
    if (nextIsRows > 0) {
      nextIsRows = -1;
      rows = atoi(p);
    } else if (nextIsColumns > 0) {
      nextIsColumns = -1;
      columns = atoi(p);
    } else if (!strcmp(p, "-h")) {
      printf("Usage: Edit [-r rows] [-c columns] [-h] fileName\n"
	     "-h\tShow this help\n"
	     "\nNOTE: While in the editor, push Ctrl-P for help\n");
      retVal = S_OK;
      goto finally;
    } else if (!strcmp(p, "-r")) {
      nextIsRows = 1;
    } else if (!strcmp(p, "-c")) {
      nextIsColumns = 1;
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

  fp = fopen("S:SYS/EDIT.INI", "r");
  while (fp) {
    static u_int16 confErrors = 0;
    if (fgets(oneLine, ONE_LINE_LEN, fp)) {
      s_int16 *p = oneLine+strlen(oneLine)-1;
      while (p >= oneLine && isspace(*p)) {
	*p-- = '\0';
      }
      p = oneLine;
      while (*p && isspace(*p)) {
	p++;
      }
      if (*p == '\0' || *p == '#') {
	/* Empty line or comment line, ignore */
      } else if (!strcasecmp(p, "inv_del")) {
	invertBS[0] = 1;
      } else if (!strcasecmp(p, "inv_c-h")) {
	invertBS[1] = 1;
      } else if (!strcasecmp(p, "inv_backspace")) {
	invertBS[2] = 1;
      } else {
	printf("Malformed line \"%d\"\n", p);
	confErrors++;
      }
    } else {
      fclose(fp);
      fp = NULL;
      if (confErrors) {
	printf("W: There were errors in S:SYS/EDIT.INI\n");
	Delay(2*TICKS_PER_SEC);
      }
    }
  }

  fp = fopen(fileName, "r");
  if (!fp) {
    printf("New file %s!\n", fileName);
    InsertLine(0, "");
    Delay(TICKS_PER_SEC);
  } else {
    while (fgets(oneLine, ONE_LINE_LEN, fp)) {
      s_int16 *p = oneLine+strlen(oneLine)-1;
      while (p >= oneLine && (*p == '\n' || *p == '\r')) {
	*p-- = '\0';
      }
      if (InsertLine(lines, oneLine)) {
	goto finally;
      }
    }
    fclose(fp);
    fp = NULL;
  }

  if (!nextIsRows && !nextIsColumns) {
    /* Determine terminal size */
    printf("\033[2J\033[f\033[999;999H");
    GetCoordinates(&columns, &rows);
    columns--;
  }

  /* Clear screen */
  printf("\033[2J");
  if (UpdateScreen(1) || NewCurrentLine(0)) {
    goto finallycleargraphics;
  }

  while ((c = fgetc(stdin)) != 'C'-0x40) {
    /* Note: 0x4f should not really be needed, but for some reason Microcom
       seems to use it instead of 0x2b for cursor up/down/left/right */
    if (c == 0x1b && (((c = fgetc(stdin)) == 0x5b) || c == 0x4f)) {
      c = fgetc(stdin);
      if (c == 'A') {
	/* Up */
      up:
	if (cursorLine > 0) {
	  if (NewCurrentLine(--cursorLine)) {
	    goto finallycleargraphics;
	  }
	  if (cursorLine < topLine) {
	    topLine -= rows/2;
	    if (topLine < 0) {
	      topLine = 0;
	    }
	    if (UpdateScreen(1)) {
	      goto finallycleargraphics;
	    }
	  }
	  if (cursorColumn > strlen(d[cursorLine])) {
	    printf("\033[%dD", cursorColumn-strlen(d[cursorLine]));
	    cursorColumn = strlen(d[cursorLine]);
	  }
	  if (UpdateScreen(0)) {
	    goto finallycleargraphics;
	  }
	}
      } else if (c == 'B') {
	/* Down */
      down:
	if (cursorLine < lines-1) {
	  if (NewCurrentLine(++cursorLine)) {
	    goto finallycleargraphics;
	  }
	  if (cursorLine >= topLine+rows-1) {
	    topLine += rows/2;
	    if (UpdateScreen(1)) {
	      goto finallycleargraphics;
	    }
	  }
	  if (cursorColumn > strlen(d[cursorLine])) {
	    s_int16 t = strlen(d[cursorLine]);
	    printf("\033[%dD", cursorColumn-t);
	    cursorColumn = t;
	  }
	  if (UpdateScreen(0)) {
	    goto finallycleargraphics;
	  }
	}
      } else if (c == 'C') {
	/* Right */
	if (cursorColumn < strlen(d[cursorLine])) {
	  cursorColumn++;
	  if (UpdateScreen(0)) {
	    goto finallycleargraphics;
	  }
	} else if (cursorLine < lines-1) {
	  cursorColumn=0;
	  goto down;
	}
      } else if (c == 'D') {
	/* Left */
	if (cursorColumn) {
	  cursorColumn--;
	  if (UpdateScreen(0)) {
	    goto finallycleargraphics;
	  }
	} else if (cursorLine) {
	  cursorColumn = 32767;
	  goto up;
	}
      } else if (c == '2') {
	/* Insert */
	fgetc(vo_stdin);
	insertMode = !insertMode;
	UpdateMenu(NULL);
      } else if (c == '3') {
	/* Delete = \[[3~ */
	fgetc(vo_stdin);
	if (DelChar(invertBS[0])) {
	  goto finallycleargraphics;
	}
      } else if (c == 0x48) {
	/* Home */
	cursorLine = 0;
	cursorColumn = 0;
	topLine = 0;
	if (UpdateScreen(1)) {
	  goto finallycleargraphics;
	}
      } else if (c == 0x46) {
	/* End */
	cursorLine = lines-1;
	cursorColumn = strlen(d[lines-1]);
	topLine = cursorLine-rows+2;
	if (topLine < 0) {
	  topLine = 0;
	}
	if (UpdateScreen(1)) {
	  goto finallycleargraphics;
	}
      } else if (c == 0x35) {
	/* Page up */
	s_int16 t;
	fgetc(stdin);
	topLine -= rows;
	cursorLine -= rows;
	if (topLine < 0) {
	  topLine = 0;
	}
	if (cursorLine < 0) {
	  cursorLine = 0;
	}
	if (cursorColumn > (t = strlen(d[cursorLine]))) {
	  cursorColumn = t;
	}
	if (UpdateScreen(1)) {
	  goto finallycleargraphics;
	}
      } else if (c == 0x36) {
	/* Page down */
	s_int16 t;
	fgetc(stdin);
	topLine += rows;
	cursorLine += rows;
	if (topLine >= lines) {
	  topLine = lines-1;
	}
	if (cursorLine >= lines) {
	  cursorLine = lines-1;
	}
	if (cursorColumn > (t=strlen(d[cursorLine]))) {
	  cursorColumn = t;
	}
	if (UpdateScreen(1)) {
	  goto finallycleargraphics;
	}
      }
    } else if (c == 'A'-0x40) {
      /* Beginning of line */
      cursorColumn = 0;
      if (UpdateScreen(0)) {
	goto finallycleargraphics;
      }
    } else if (c == 'E'-0x40) {
      /* End of line */
      cursorColumn = strlen(editLine);
      if (UpdateScreen(0)) {
	goto finallycleargraphics;
      }
    } else if (c == 'P'-0x40) {
      printf("\033[2J\033[2;1H"
	     "    EDIT help:\n"
	     "\tCtrl-C\tClose editor\n"
	     "\tCtrl-W\tWrite file\n"
	     "\tCtrl-A\tGo to beginning of line\n"
	     "\tCtrl-E\tGo to end of line\n"
	     "\tCtrl-H\tDelete char\n"
	     "\tCtrl-K\tKill line\n"
	     "\tCtrl-Y\tYank killed line\n"
	     "\tCtrl-P\tShow this help\n"
	     "\n    Other keys:\n"
	     "\tInsert, Delete, Backspace, Home, End, Page Up, Page Down\n"
	     "\n    Press <Space> to exit help\n\n");
      while ((c = fgetc(stdin)) != ' ') {
	if (c == 'C'-0x40) {
	  retVal = S_OK;
	  goto finallycleargraphics;
	  break;
	}
      }
      if (UpdateScreen(1)) {
	goto finallycleargraphics;
      }
    } else if (c == 'K'-0x40) {
      /* Kill line */
      s_int16 *p = editLine+cursorColumn;
      if (*p) {
	strcpy(killLine, p);
	*p = '\0';
	if (UpdateScreen(0)) {
	  goto finallycleargraphics;
	}
      } else {
	if (DelChar(0)) {
	  goto finallycleargraphics;
	}
      }
    } else if (c == 'Y'-0x40) {
      s_int16 *p = killLine;
      while (*p) {
	if (AddChar(*p++, p[1] ? -1 : 1)) {
	  goto finallycleargraphics;
	}
      }
    } else if (c == 'W'-0x40) {
      /* Write */
      sprintf(oneLine, "%s %s", fileName, fileName);
      if ((p = strrchr(oneLine, '.'))) {
	strcpy(p, ".bak");
	sprintf(oneLine+ONE_LINE_LEN/2,
		"Making backup%s", strrchr(oneLine, ' '));
	UpdateMenu(oneLine+ONE_LINE_LEN/2);
	RunProgram("CopyF", oneLine);
      }
      sprintf(oneLine, "Writing %s", fileName);
      UpdateMenu(oneLine);
      if (NewCurrentLine(-1)) {
	goto finallycleargraphics;
      }
      printf("\033[1;1H");
      Delay(TICKS_PER_SEC/10);
      fp = fopen(fileName, "wb");
      if (!fp) {
	sprintf(oneLine, "ERROR! Couldn't open %s for writing!", fileName);
	UpdateMenu(oneLine);
	Delay(2*TICKS_PER_SEC);
      } else {
	for (i=0; i<lines; i++) {
	  fprintf(fp, "%s\r\n", d[i]); /* Write in DOS format: with CR+LF */
	}
	fclose(fp);
	fp = NULL;
      }
      UpdateMenu(NULL);
    } else if (c == 0x8) {
      /* C-H */
      if (DelChar(invertBS[1])) {
	goto finallycleargraphics;
      }
    } else if (c == 0x7f) {
      /* Backspace */
      if (DelChar(!invertBS[2])) {
	goto finallycleargraphics;
      }
    } else if ((c == 0xa && !lastCharWasCr) || c == 0xd) {
      /* Newline */
      s_int16 col = cursorColumn;
      if (c == 0xd) {
	lastCharWasCr = 2;
      }
      /* Make copy of the line being split. */
      strcpy(oneLine, editLine);
      /* Make the end of line. */
      if (InsertLine(cursorLine+1, oneLine+col)) {
	goto finallycleargraphics;
      }
      /* Make the beginning of the line */
      if (NewCurrentLine(cursorLine)) {
	goto finallycleargraphics;
      }
      editLine[col] = '\0';
      /* Set cursor and redraw. */
      if (++cursorLine > topLine+rows-2) {
	topLine += rows/2;
      }
      cursorColumn = 0;
      if (UpdateScreen(1)) {
	goto finallycleargraphics;
      }
    } else if (c >= 0x20 || c == 9) {
      /* Normal, printable char, or tabulator */
      if (AddChar(c, insertMode)) {
	goto finallycleargraphics;
      }
    }
    if (lastCharWasCr) {
      lastCharWasCr--;
    }
  } /* while (!c-c) */

  retVal = S_OK;

 finallycleargraphics:
  printf("\033[%d;1H\n", rows);
  if (retVal) {
    printf("ERROR, ABORTING\n");
  }

 finally:
  if (fp) {
    fclose(fp);
    fp = NULL;
  }
  if (d) {
    s_int16 **p = d;
    for (i=0; i<lines; i++) {
      s_int16 *t = *p++;
      if (t) {
	free(t);
      }
    }
    free(d);
    d = NULL;
  }

#if 0
  while (1) {
    c = fgetc(stdin);
    if (isprint(c)) {
      printf("%c", c);
    } else {
      printf("<%02x>", c);
    }
  }
#endif

  return retVal;
}


void fini(void) {
}
