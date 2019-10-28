/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <consolestate.h>
#include <string.h>
#include <strings.h>
#include <vo_fat.h>
#include <ctype.h>
#include <kernel.h>

char s[256];
VO_FILE *f;
char oldDir[256];

void ListDevices(void) {
  int i;
  static char s[80];
  printf("Currently installed system devices are:\n");
  for (i=0; i<26; i++) {
    if (vo_pdevices[i]) {
      DiskGeometry g = {0, 0};
      printf("- %c: ", 'A'+i);
      if (ioctl(vo_pdevices[i], IOCTL_GET_GEOMETRY, (void *)&g) != S_ERROR) {
	g.totalSectors >>= 1; /* Now in KiB */
	if (g.totalSectors < 10000) {
	  printf("%4ldK", g.totalSectors);
	} else if ((g.totalSectors /= 1024) < 10000) {
	  printf("%4ldM", g.totalSectors);
	} else {
	  printf("%4ldG", g.totalSectors/1024);
	}
      } else {
	printf("      ");
      }
      printf(" %s", vo_pdevices[i]->Identify(vo_pdevices[i], 0, 0));
      if (vo_pdevices[i]->fs) {
	printf(", handled by %s",
	       vo_pdevices[i]->fs->Identify(vo_pdevices[i]->fs, 0, 0));
      }
      printf(".\n");
    }
  }
}


ioresult SetDirectory(register char *p, register u_int16 verbose) {
  ioresult retVal = S_OK;
  int currentDirectoryLen = strlen(currentDirectory);

#if 0
  printf("SetDirectory(%s, %d)\n", p, verbose);
#endif

  if (!strcmp(p, "..") && (currentDirectoryLen > 2)) {
    char *p2 = &currentDirectory[currentDirectoryLen-1];
    *p2 = '\0';
    if (p2 = strrchr(currentDirectory, '/')) {
      p2[1] = '\0';
    } else {
      currentDirectory[2] = '\0';
    }
  } else if (strlen(p)==2 && p[1]==':') {
    DEVICE *newDisk;

    p[0] = toupper(p[0]);
    if (!isalpha(p[0])) {
      if (verbose) ListDevices();
    } else {
      newDisk = vo_pdevices[p[0]-'A']; 
      if (!newDisk) {
	if (verbose) {
	  printf("Disk %s not found. ", p);
	  ListDevices();
	}
	return S_ERROR;
      }
      if (newDisk->Identify && (appFlags & APP_FLAG_ECHO) && verbose) {
	printf("%s %s\n", p, newDisk->Identify(newDisk, NULL, 0)); 
      }
      if (newDisk->fs != vo_filesystems[0]) {
	if (verbose) {
	  printf("Filesystem of device %s is not FAT.\n");
	}
	return S_ERROR;
      }
      strcpy(currentDirectory,p);
    }
  } else {
    f = fopen(currentDirectory, "s");
    if (!f || FatFindFirst(f, currentDirectory+2, s, sizeof(s)-1) != S_OK) {
      if (verbose) {
	printf("The current directory is not valid.");
      }
      retVal = S_ERROR;
    } else {
      do {
	if (f->ungetc_buffer & 0x0010) {
	  int i;
	  if (!strcasecmp(p, s) || !strcasecmp(p, f->extraInfo)) {
	    /* By copying extraInfo instead of s, we get the short name */
	    sprintf(currentDirectory+currentDirectoryLen, "%s/", f->extraInfo);
	    goto end_search;
	  }
	}
      } while (FatFindNext(f, s, sizeof(s)-1) == S_OK);
      if (verbose) {
	printf("Path not found\n");
      }
      retVal = S_ERROR;
  end_search:
      fclose(f);
    }
  }

  return retVal;
}



ioresult main(char *parameters) {
  u_int16 i;
  char *p = parameters;
  int nParam = RunProgram("ParamSpl", parameters);
  ioresult errCode = S_ERROR;
  s_int16 verbose = 1;

  strcpy(oldDir, currentDirectory);

  if (!nParam) {
    puts(currentDirectory);
    errCode = S_OK;
    goto finally;
  }

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: cd [-v|+v] [-h] dir\n"
	     "-v|+v\tTurn verbose on/off\n"
	     "-h\tShow this help\n"
	     "Note: cd without parameters shows the current directory\n"
	     "      cd :: lists all available devices\n");
      errCode = S_OK;
      goto finally;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else {
      char *nextDP = NULL;

      /* If starts with drive definition */
      if (p[1] == ':') {
	char drive[3];
	drive[0] = toupper(p[0]);
	drive[1] = ':';
	drive[2] = '\0';
	p += 2;
	if (SetDirectory(drive, verbose)) {
	  goto finally;
	}
      }

      /* Go through each directory in the directory path. */
      do {
	if ((nextDP = strchr(p,'/')) || (nextDP = strchr(p,'\\'))) {
	  *nextDP = NULL;
	  nextDP++;
	}
	if (!*p || !strcmp(p, ".")) {
	  /* No-op */
	} else if (SetDirectory(p, verbose)) {
	  goto finally;
	}
	p = nextDP;
      } while (nextDP);
    }
    p += strlen(p)+1;
  }

  errCode = S_OK;
 finally:
  if (errCode) {
    strcpy(currentDirectory, oldDir);
  }

  return errCode;
}
