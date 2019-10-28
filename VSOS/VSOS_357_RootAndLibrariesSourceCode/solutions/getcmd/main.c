#define HASH_DELAY 400


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
#include <timers.h>
#include <sysmemory.h>
#include <consolestate.h>
#include <imem.h>
#include <kernel.h>

void MonitorBreak(void);

char tabfn[128];
u_int16 tabstate = 0;

/* Returns a command from history */
s_int16 ReturnFromHistory(register char *d, register const char *history,
			  register s_int16 len, register s_int16 entry) {
  s_int16 *prev;
  s_int16 currEntry = 0;
  history += len-1;
  prev = history--;
  while (len) {
    u_int16 histChar = *history;
    if (!histChar) {
      return -1;
    }
    if (histChar == 0xFFFFU) {
      if (entry == currEntry) {
	int thisLen = prev-history-1;
	memcpy(d, history+1, thisLen);
	d[thisLen] = '\0';
	return entry;
      } else {
	prev = history;
	currEntry++;
      }
    }
    len--;
    history--;
  }
  return -2;
}

ioresult main(char *cmdLine) {
  char *p;
  int i=0;
  int cmdLineSize;
  char *oldCmd;
  int prevCmdCounter;
  int insertMode = 1;
  char *b2 = NULL, *b = NULL;

  memset(tabfn, 0, sizeof(tabfn));

  if (!cmdLine || !cmdLine[0]) {
    return S_OK;
  }

  cmdLineSize = cmdLine[0];

 do_it_again:
  p = cmdLine+1;
  i = 0;
  prevCmdCounter = -1;

  if (!(oldCmd = malloc((cmdLineSize-1)*sizeof(sizeof(oldCmd[0]))))) {
    goto finally;
  }
  cmdLine++;
  memcpy(oldCmd, cmdLine, cmdLineSize-1);
  // detect split string and unsplit.
  do {i++;} while (oldCmd[i] != 0); 
  if (oldCmd[i+1]) {
    oldCmd[i] = ' ';
  }
  memset(p, 0, cmdLineSize-1);
	
  appFlags &= ~(APP_FLAG_QUIT | APP_FLAG_RAWTTY);

  if (appFlags & APP_FLAG_ECHO) {
    printf("%s>", currentDirectory);
  } else {
    printf("#\n", appFlags);
  }

#if 0
  while(1) {
    char c = fgetc(vo_stdin);
    printf("%02x:",c);
  }
#else
  while(1) {
    char c = fgetc(vo_stdin);
    //if (c<' ') {
    //    	printf("[%x]",c);
    //}
    
	if (!(appFlags & APP_FLAG_RAWTTY) && (c == 239)) {
		PERIP(INT_ENABLE0_LP) = 0;
		PERIP(INT_ENABLE0_HP) = 0;
		PERIP(INT_ENABLE1_LP) = 0;
		PERIP(INT_ENABLE1_HP) = 0;
		WriteIMem(0x20,0x2a26988e); //restore original DAC interrupt vector
		WriteIMem(0x2d,0x2a2026ce); //restore original RX interrupt vector
		MonitorBreak(); //VS3EMU Connection /// \todo disable while parsing UTF-8
	}

    if (c==0x1b) {
      //printf("[Esc]");
      /* Note: 0x4f should not really be needed, but for some reason Microcom
	 seems to use it instead of 0x2b for cursor up/down/left/right */
      if ((c = fgetc(vo_stdin)) != 0x5b && c != 0x4f) continue;
      c = fgetc(vo_stdin);
      if (c=='A') {
	/* Up */
	if (ReturnFromHistory(cmdLine, oldCmd, cmdLineSize-1, prevCmdCounter+1)
	    == prevCmdCounter+1) {
	  prevCmdCounter++;
	  if (appFlags & APP_FLAG_ECHO) {
	    printf("\033[2K");
	    printf("\r%s>%s", currentDirectory, cmdLine);
	  }
	  p = cmdLine+strlen(cmdLine);
	}
      } else if (c=='B') {
	/* Down */
	if (ReturnFromHistory(cmdLine, oldCmd, cmdLineSize-1, prevCmdCounter-1)
	    == prevCmdCounter-1) {
	  prevCmdCounter--;
	  if (appFlags & APP_FLAG_ECHO) {
	    printf("\033[2K");
	    printf("\r%s>%s", currentDirectory, cmdLine);
	  }
	  p = cmdLine+strlen(cmdLine);
	}
      } else if (c=='C') {
	/* Cursor right */
	if (*p) {
	  if (appFlags & APP_FLAG_ECHO) {
	    printf("\033[1C");	
	  }
	  p++;
	}
      } else if (c=='D') {
	/* Cursor left */
	if (p>cmdLine) {
	  if (appFlags & APP_FLAG_ECHO) {
	    printf("\033[1D");	
	  }
	  p--;
	}
      } else if (c=='2') {
	/* Insert */
	fgetc(vo_stdin);
	insertMode = !insertMode;
      } else if (c=='3') {
	/* Delete */
	fgetc(vo_stdin);

	if (*p) {
	  strcpy(p, p+1);
	  if (appFlags & APP_FLAG_ECHO) {
	    printf("\033[0K");
	    if (*p) {
	      printf("%s", p);
	      printf("\033[%dD", strlen(p));
	    }
	  }
	}
      }
      continue;
    } else if (c==0x9) {
      //printf("#1"); Delay(HASH_DELAY); printf("\x8\x8");
      if (tabstate==0) {
	//printf("#2"); Delay(HASH_DELAY); printf("\x8\x8");
	b = p;
	while ((b[-1]!=' ') && (b[-1]!=':') && (b[-1]!='/') && (b[-1]!='\\') && (b > cmdLine)) {
	  b--;
	}
	b2 = b;
	b = p;
	while ((b[-1]!=' ') && (b > cmdLine)) {
	  b--;
	}		
	if ((b2==cmdLine) && (b==cmdLine)) {
	  sprintf(tabfn,"s:sys/%s*.dl3", b);					
	} else {
	  sprintf(tabfn, "%s*", b);
	}
      }
      {
	char m[8];
	FILE *f;
      tryagain:
	sprintf(m,"rb#%d",tabstate);
	tabstate++;
	f = fopen(tabfn, m);
	//printf("#3"); Delay(HASH_DELAY); printf("\x8\x8");
	//printf("(%s)(%s)",tabfn,m); Delay(1000);
	if (f) {
	  //printf("#4"); Delay(HASH_DELAY); printf("\x8\x8");
	  
	  strcpy(b2, f->Identify(f,NULL,0));
	  if ((b2==cmdLine) && (b==cmdLine)) {
	    char *c = cmdLine;
	    while (*c) {
	      if (*c=='.') {
		char *d = &cmdLine[cmdLineSize-2];
		while(d>=c) {
		  *d--=0;
		}
		break;
	      }
	      c++;
	    }
	  }
	  p = cmdLine+strlen(cmdLine);
	  if (appFlags & APP_FLAG_ECHO) {
	    printf("\r%s>%s\x1b[0K",currentDirectory,cmdLine);
	  }
	  fclose(f);
	} else {
	  //printf("#5"); Delay(HASH_DELAY); printf("\x8\x8");

	  if (tabstate!=1) {
	    //printf("#6"); Delay(HASH_DELAY); printf("\x8\x8");

	    tabstate=0;
	    goto tryagain;
	  }
	}
      }
    } else {
      tabstate = 0;
      if (c=='A'-0x40) {
	/* Start of line */
	if (p > cmdLine && (appFlags & APP_FLAG_ECHO)) {
	  printf("\033[%dD", p-cmdLine);
	}
	p = cmdLine;
      } else if (c=='E'-0x40) {
	/* End of line */
	if (*p && (appFlags & APP_FLAG_ECHO)) {
	  printf("\033[%dC", strlen(p));
	}
	p += strlen(p);
      } else if (c=='K'-0x40) {
	/* Erase to end of line */
	*p = '\0';
	if (appFlags & APP_FLAG_ECHO) {
	  printf("\033[0K");
	}
      } else if ((c==0xd) || (c==0xa)) {
	/* Enter / Line feed */
	goto finally;
      } else if (c==8 || c==0x7f) {
        /* Backspace */
	if (p>cmdLine) {
	  --p;
	  strcpy(p, p+1);
	  if (appFlags & APP_FLAG_ECHO) {
	    printf("\033[1D");
	    printf("\033[0K");
	    if (*p) {
	      printf("%s", p);
	      printf("\033[%dD", strlen(p));
	    }
	  }
	}
      } else if (c>0x1b) {
	/* Normal, printable character */
	if (insertMode) {
	  memmove(p+1, p, strlen(p));
	  cmdLine[cmdLineSize-2] = '\0';
	}
	if (p < cmdLine+cmdLineSize-2) {
	  if (!*p) { /* If last char on line, clear next char. */
	    p[1] = '\0';
	  }
	  *p++ = c;
	  if (appFlags & APP_FLAG_ECHO) {
	    printf("%c", c);
	  }
	}
	if (appFlags & APP_FLAG_ECHO) {
	  if (*p) {
	    printf("%s", p);
	    printf("\033[%dD", strlen(p));
	  }
	}
      } else {
	if (appFlags & APP_FLAG_ECHO) {
	  int i;
	  printf("<%04x>", c);
	  Delay(300);
	  printf("\033[6D");
	  printf("\033[0K");	
	  if (*p) {
	    printf("%s", p);
	    printf("\033[%dD", p, strlen(p));
	  }
	}
	while (c==0xFFFF) { /* If we lose file input, stay in infinite loop. */ 
	  Delay(TICKS_PER_SEC);
	}
      }
    }
  } /* while (1) */
#endif
	
 finally:
  appFlags &= ~APP_FLAG_QUIT;

  //  *p = '\0';
  /* If command line is of format "%-number", replace it with a command
     from history. */
  if (p-cmdLine >= 2 && cmdLine[0] == '!') {
    if (cmdLine[1] == '!') {
      ReturnFromHistory(cmdLine, oldCmd, cmdLineSize-1, 0);
    } else if (cmdLine[1] == '-') {
      ReturnFromHistory(cmdLine, oldCmd, cmdLineSize-1, atoi(cmdLine+2)-1);
    } else {
      int i = 0;
      while (ReturnFromHistory(oldCmd, oldCmd, cmdLineSize-1, i) == i) {
	if (!strncmp(cmdLine+1, oldCmd, strlen(cmdLine+1))) {
	  strcpy(cmdLine, oldCmd);
	  break;
	}
	i++;
      }
    }
  }


#if 0
  printf("cmdLineSize = %d, line \"%s\"\n", cmdLine[-1], cmdLine);
#endif

  /* If there was a command, store it in history. */
  if (cmdLine[0]) {
    u_int16 len = strlen(cmdLine);
    if (len < cmdLineSize/2) {
      oldCmd[cmdLineSize-2] = -1;
      memmove(oldCmd, oldCmd+len+1, cmdLineSize-2-len);
      strcpy(oldCmd+cmdLineSize-2-len, cmdLine);
    } else {
      memset(oldCmd, 0, cmdLineSize-1);
    }
  }
  strcpy(oldCmd, cmdLine);
  memcpy(cmdLine, oldCmd, cmdLineSize-1);

  /* If command line is of format "x:", replace it with "cd x:" */
  if (p-cmdLine == 2 && cmdLine[1] == ':') {
    sprintf(cmdLine, "cd %c:", cmdLine[0]);
  } else if (!strcmp(cmdLine, "..")) {
    strcpy(cmdLine, "cd ..");
  }

  if (oldCmd) {
    if (!strcmp(cmdLine, "history")) {
      int i = 0;
      if (appFlags & APP_FLAG_ECHO) {
	printf("\n");
      }
      while (ReturnFromHistory(oldCmd, cmdLine, cmdLineSize-1, i) == i) {
	printf("!-%d %s\n", ++i, oldCmd);
      }
      free(oldCmd);
      cmdLine--;
      goto do_it_again;
    }
    if (!strcmp(cmdLine, "help")) {
      printf("\nGetCmd functionality:\n"
	     "Up\tPrevious command\n"
	     "Down\tNext command\n"
	     "C-K\tKill rest of line\n"
	     "C-A\tGo to beginning of line\n"
	     "C-E\tGo to end of line\n"
	     "Ins\tToggle insert mode\n"
	     "history\tShow command history (length set and limited by shell)\n"
	     "!!\tRe-execute previous command\n"
	     "!-n\tRe-execute 'n'th command in history\n"
	     "!xx\tRe-execute last command that started with \"xx\"\n"
	     "help\tShow this help\n");
      free(oldCmd);
      cmdLine--;
      goto do_it_again;
    }
    free(oldCmd);
    oldCmd = NULL;
  }

  cmdLine--;

  if (!cmdLine[1]) {
    if (appFlags & APP_FLAG_ECHO) {
      printf("\n");
    }
    goto do_it_again;
  }

  if (appFlags & APP_FLAG_ECHO) {
    printf("\n");
  } else {
    printf(":\n");
  }

  return S_OK;
}
