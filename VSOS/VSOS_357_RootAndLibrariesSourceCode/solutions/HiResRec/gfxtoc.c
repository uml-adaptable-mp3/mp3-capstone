#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <leo_ppm.h>

void PrintHelp(const char *progName) {
    printf("Usage:\n"
	   "%s in.ppm iconname n w h x1 y1 [x2 y2 [...]]\n",
	   progName);
}

int main(int argc, char **argv) {
  struct PICTURE *pic = NULL;
  int retVal = EXIT_FAILURE;
  int i, n, w, h;
  FILE *fp = NULL;
  char *outFileName = NULL;
  int compression = 0;

  if (argc < 8) {
    PrintHelp(argv[0]);
    goto finally;
  }

  n = strtol(argv[3], NULL, 0);
  if (n == -1) {
    n = 1;
    compression = 1;
  }
  if (argc != 6+2*n) {
    PrintHelp(argv[0]);
    goto finally;
  }

  w = strtol(argv[4], NULL, 0);
  h = strtol(argv[5], NULL, 0);

  pic = ReadPic(argv[1]);
  if (!pic) {
    printf("%s: Couldn't read %s\n", argv[0], argv[1]);
  }

  if (pic->type != PIC_PPM) {
    printf("%s: %s is wrong type\n", argv[0], argv[1]);
    goto finally;
  }

  outFileName=malloc(strlen(argv[2]+3));
  strcpy(outFileName, argv[2]);
  strcat(outFileName, ".c");

  if (!(fp = fopen(outFileName, "w"))) {
    printf("%s: Could't open %s for writing\n", argv[0], argv[2]);
  }

  fprintf(fp, "#include <vo_stdio.h>\n\n");

  if (compression) {
    int xStart = strtol(argv[6+0*2], NULL, 0);
    int yStart = strtol(argv[7+0*2], NULL, 0);
    int xEnd = xStart+w;
    int yEnd = yStart+h;
    int rleRun = 0;
    int mem = -1;
    int y, x, n;
    int tIdx = 0;
    int tab[65536][2];
    char s[1024*1024] = "";

    for (y=yEnd-1; y>=yStart; y--) {
      for (x=xEnd-1; x>=xStart; x--) {
	int r = pic->data[0][y*pic->width+x] >> 3;
	int g = pic->data[1][y*pic->width+x] >> 2;
	int b = pic->data[2][y*pic->width+x] >> 3;
	int rgb;
#if 0
	if (!r && !g && !b) {
	  r = 0; g = 21; b = 7; /* Transparent. */
	}
#endif
	rgb = (r<<11) | (g<<5) | (b);
	if (mem < 0) {
	  rleRun = 1;
	} else if (rgb == mem) {
	  rleRun++;
	} else {
	  rleRun = 1;
	}
	tab[tIdx][0] = rgb;
	tab[tIdx][1] = rleRun;
	tIdx++;
	mem = rgb;
#if 0
	printf("%3d %3d %04x %3d\n", y, x, rgb, rleRun);
#endif
      }
    }

    i = tIdx-1;
    n = 0;
    while (i>-1) {
      int useRle = 0;
      if (tab[i][1] > 2) {
	useRle = 1;
      } else if (tab[i][1] == 2 && (i < 2 || tab[i-2][1] > 1)) {
	useRle = 1;
      }
      if (useRle) {
	sprintf(s+strlen(s),
		"    0x%04x, 0x%04x,\n", tab[i][1]|0x8000, tab[i][0]);
	i -= tab[i][1];
	n += 2;
      } else {
	int iStart = i;
	int column = 1;
	while (tab[iStart][1] < 3 && iStart >= 0) {
	  iStart--;
	}
	n += 1+i-iStart;
	sprintf(s+strlen(s), "    0x%04x,", i-iStart);
	while (i>iStart) {
	  if (column++ > 8) {
	    sprintf(s+strlen(s), "\n           ");
	    column = 2;
	  }
	  sprintf(s+strlen(s), " 0x%04x,", tab[i][0]);
	  i--;
	}
	  
	sprintf(s+strlen(s), "\n");
      }
    }
    fprintf(fp, "/* WxH = %dx%d = %d pixels, %d (%1.1f%%) after RLE "
	    "compression */\n",
	    w, h, w*h, n, 100.0*n/(w*h));
    fprintf(fp, "const u_int16 icons_%s[1][%d] = {\n", argv[2], n);
    fprintf(fp, "  {\n");
    fprintf(fp, "%s", s);
    fprintf(fp, "  }\n};\n");
  } else { /* !compression */
    fprintf(fp, "/* WxH = %dx%d */\n", w, h);
    fprintf(fp, "const u_int16 icons_%s[%d][%d] = {\n", argv[2], n, w*h);

    fprintf(fp, "  {\n");
    for (i=0; i<n; i++) {
      int xStart = strtol(argv[6+i*2], NULL, 0);
      int yStart = strtol(argv[7+i*2], NULL, 0);
      int xEnd = xStart+w;
      int yEnd = yStart+h;
      int y, x;
      for (y=yStart; y<yEnd; y++) {
	fprintf(fp, "    ");
	for (x=xStart; x<xEnd; x++) {
	  int r = pic->data[0][y*pic->width+x] >> 3;
	  int g = pic->data[1][y*pic->width+x] >> 2;
	  int b = pic->data[2][y*pic->width+x] >> 3;
	  if (!r && !g && !b) {
	    r = 31; g = 63; b = 30; /* Transparent. */
	  }
	  fprintf(fp, "0x%04x,", (r<<11) | (g<<5) | (b));
	}
	fprintf(fp, "\n");
      }
      fprintf(fp, "  }%s", (i<n-1)?",{\n":"\n");
    }
    fprintf(fp, "};\n");
  }

  retVal = EXIT_SUCCESS;

 finally:
  if (pic) {
    FreePic(pic);
    pic = NULL;
  }

  if (fp) {
    fclose(fp);
    fp = NULL;
  }

  return retVal;
}
