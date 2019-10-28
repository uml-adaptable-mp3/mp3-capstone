#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/*** START leo_types.h ***/

#include <limits.h>

#define WORDMASK(b)		(((ULL)-1)>>(sizeof(ULL)*8-(b)))

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif



/*

   System-independent fixed-sized integers and floating point types.

 */
#if CHAR_MAX == 127
	typedef char		s_int8;
#else
	typedef signed char	s_int8;
#endif
#define FMT_S8 "%d"
#if CHAR_MAX == 255
	typedef char		u_int8;
#else
	typedef unsigned char	u_int8;
#endif
#define FMT_U8 "%d"
typedef short			s_int16;
#define FMT_S16 "%d"
typedef unsigned short		u_int16;
#define FMT_U16 "%d"
typedef int			s_int32;
#define FMT_S32 "%d"
typedef unsigned int		u_int32;
#define FMT_U32 "%u"
typedef long long		s_int64;
#define FMT_S64 "%lld"
typedef unsigned long long	u_int64;
#define FMT_U64 "%llu"
typedef float			flt32;
#define FMT_F32 "%f"
typedef double			flt64;
#define FMT_F64 "%lf"
/*
   Some more types that may come handy, but without a fixed size.
 */
typedef int			lbool;


/*

  Define PI. This could in fact be in its own file, but this is as good
  place as any.

 */
#ifndef PI
	#define PI 3.15169265358979
#endif

/*** END leo_type.h ***/



/*** START leo_ppm.h ***/

#ifndef PI
  #define PI 3.14159265358979323
#endif

enum PicType {
    PIC_PBM,
    PIC_PGM,
    PIC_PPM,
    PIC_INVALID
};

struct PICTURE {
    enum PicType type;
    s_int32 width, height, maxVal;
    u_int8 *data[3];
};

extern char *progName;

struct VECTOR {
    flt64 *data;
    s_int32 size, maxSize;
};

void FreePic(struct PICTURE *p);
struct PICTURE *AllocPic(long width, long height, long maxVal,
			 enum PicType typ);
struct PICTURE *ReadPic(const char *fileName);
int WritePic(const char *fileName, const struct PICTURE *pic);
void PbmToPgm(struct PICTURE *pic);

/*** END leo_ppm.h ***/



/*** START leo_ppm.c ***/

static s_int32 ReadUnsignedInt(FILE *in) {
    s_int32 val = 0;
    int ch = fgetc(in);

    do {
	while (isspace(ch))
	    ch = fgetc(in);
	if (ch == '#') {
	    while ((ch = fgetc(in)) != '\n' && ch != EOF)
		; /* Empty while */
	    if (ch == EOF)
		return -1;
	}
    } while (!isdigit(ch));

    while (isdigit(ch)) {
        val = val*10 + (s_int32)(ch-'0');
        ch = fgetc(in);
    }

    if (!isspace(ch))
	return -1;
    else {
	return val;
    }
}




struct PICTURE *AllocPic(long width, long height, long maxVal,
			 enum PicType typ) {
    static struct PICTURE *p;
    int i;

    if (width < 1 || height < 1 || maxVal < 1 || maxVal > 255) {
	fprintf(stderr, "Can't allocate picture with bad values.\n"
		"width (must be >= 1) is %ld, height (>= 1) is %ld,"
		"maxVal (1..255) is %ld\n", width, height, maxVal);
	return NULL;
    }

    if (!(p = calloc(1, sizeof(struct PICTURE)))) {
	fprintf(stderr, "Not any memory for picture data.\n");
	return NULL;
    }

    p->type = typ;
    p->width = width;
    p->height = height;
    p->maxVal = maxVal;

    for (i=0; p && i<((typ == PIC_PPM) ? 3 : 1); i++) {
	if (!(p->data[i] =
	      calloc(p->width*p->height,sizeof(*p->data[0])))) {
	    fprintf(stderr, "No memory for picture data.\n");
	    FreePic(p);
	}
    }

    return p;
}

void FreePic(struct PICTURE *p) {
    if (p) {
	int i;
	for (i=0; i<3; i++) {
	    if (p->data[i])
		free(p->data[i]);
	}
    }
    free(p);
}


struct PICTURE *ReadPic(const char *fileName) {
    int ch = 0;  /* This initialization removes an unneeded warning */
    struct PICTURE *pic = NULL;
    int ok = FALSE;
    int picCode;
    enum PicType typ;
    long w, h, maxVal;
    FILE *fp;

    if (fileName) {
	if (!(fp = fopen(fileName, "rb"))) {
	    fprintf(stderr, "Could not open file %s.\n", fileName);
	    goto cleanup;
	}
    } else {
	fp = stdin;
    }

    if (fgetc(fp) != 'P' || (picCode=fgetc(fp)) == EOF) {
        fprintf(stderr, "File is not a PBM, PGM, or PPM file.\n");
	goto cleanup;
    }

    switch(picCode) {
    case '4':
	typ = PIC_PBM;
	break;
    case '5':
	typ = PIC_PGM;
	break;
    case '6':
	typ = PIC_PPM;
	break;
    default:
	fprintf(stderr, "File is not a binary PBM, PGM or PPM file.\n");
	goto cleanup;
    }

    w = ReadUnsignedInt(fp);
    h = ReadUnsignedInt(fp);
    if (typ != PIC_PBM)
	maxVal = ReadUnsignedInt(fp);
    else
	maxVal = 1;

    if (w < 1 || h < 1 || maxVal < 1) {
	fprintf(stderr, "File corrupted\n");
	goto cleanup;
    }

    pic = AllocPic(w, h, maxVal, typ);
    if (!pic)
	goto cleanup;

    if (pic->type == PIC_PBM) {
      unsigned char *p = (unsigned char *)pic->data[0];
	long x, y;
	for (y=0; y<pic->height; y++) {
	    for (x=0; x<pic->width; x++) {
		if (!(x&7)) {
		    if ((ch = getc(fp)) == EOF) {
			fprintf(stderr, "Not enough data in file.\n");
			goto cleanup;
		    }
		}
		*p++ = (ch & 128) ? 0 : 1;
		ch <<= 1;
	    }
	}
    } else {
	size_t size = pic->width*pic->height;
	if (pic->type == PIC_PGM) {
	    if (fread(pic->data[0], 1, size, fp) != size) {
		fprintf(stderr, "Not enough data in file.\n");
		goto cleanup;
	    }
	} else {
	    u_int32 loc, color, size = pic->width*pic->height;
	    for (loc = 0; loc < size; loc++) {
		for (color = 0; color < 3; color++) {
		    if ((ch = getc(fp)) == EOF) {
			fprintf(stderr, "Not enough data in file.\n");
			goto cleanup;
		    } else {
			pic->data[color][loc] = ch;
		    }
		}
	    }
	}
    }

    ok = TRUE;

 cleanup:
    if (fileName && fp)
	fclose(fp);
    if (!ok) {
	FreePic(pic);
	pic = NULL;
    }
    return pic;
}



int WritePic(const char *fileName, const struct PICTURE *pic) {
    int ok = FALSE;
    FILE *fp;
    char magic;

    if (fileName) {
	if (!(fp = fopen(fileName, "wb"))) {
	    goto cleanup;
	}
    } else {
	fp = stdout;
    }

    switch(pic->type) {
    case PIC_PBM:
	magic = '4';
	break;
    case PIC_PGM:
	magic = '5';
	break;
    case PIC_PPM:
	magic = '6';
	break;
    default:
	goto cleanup;
    }

    if (fprintf(fp, "P%c\n" FMT_S32 " " FMT_S32 "\n",
		magic, pic->width, pic->height) < 0)
	goto cleanup;

    switch(pic->type) {
    case PIC_PBM:
    {
	unsigned char *p = (unsigned char *)pic->data[0];
	long xx, x, y;
	for (y=0; y<pic->height; y++) {
	    x=0;
	    while (x<pic->width) {
		int ch = 0;
		xx = (x+7 >= pic->width) ? pic->width & 7 : 8;
		x += xx;
		while (xx-- > 0) {
		    ch = (ch << 1) + ((*p++) ? 0 : 1);
		}
		ch <<= ((8-(x&7))&7); /* Pad any leftback bits */

		if (putc(ch, fp) == EOF)
		    goto cleanup;
	    }
	}
	ok = TRUE;
	break;
    }
    case PIC_PGM:
    {
	size_t size = pic->width*pic->height;
	if (fprintf(fp, FMT_S32 "\n", pic->maxVal) < 0)
	    goto cleanup;
	if (fwrite(pic->data[0], 1, size, fp) != size)
	    goto cleanup;
	ok = TRUE;
	break;
    }
    case PIC_PPM:
    {
	long loc, color, size = pic->width*pic->height;
	if (fprintf(fp, FMT_S32 "\n", pic->maxVal) < 0)
	    goto cleanup;
	for (loc = 0; loc < size; loc++) {
	    for (color = 0; color < 3; color++) {
		if (putc(pic->data[color][loc], fp) == EOF)
		    goto cleanup;
	    }
	}
	ok = TRUE;
	break;
    }
    default:
	break;
    }

 cleanup:
    if (fileName && fp)
	fclose(fp);
    if (!ok)
	fprintf(stderr, "File couldn't be written\n");
    return ok;
}

void PbmToPgm(struct PICTURE *pic) {
    int i;
    if (pic->type == PIC_PBM) {
	unsigned char *p = (unsigned char *)pic->data[0];
	pic->type = PIC_PGM;
	pic->maxVal = 255;
	for(i=0; i<pic->width*pic->height; i++) {
	    *p = (*p) ? 255 : 0;
	    p++;
	}
    }
}

/*** END leo_ppm.c ***/








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
