#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#include <coff.h>
#include <decode.h> /* for AddRelocation() */
#include <unasm.h>

#define WITH_COMPRESSED
#define WITH_CRYPT

#define FLG_ERROR   (1<<15)
#define FLG_NOCOMPRESSION (1<<0)
#define FLG_NOMERGE       (1<<1)
#define FLG_IGNORESIZE    (1<<2) /* allows larger than 8192-byte images */
#define FLG_CRYPTED       (1<<3)
#define FLG_VERBOSE       (1<<4)

#define FLG_OVERLAY       (1<<6) /* multiple binaries */
#define FLG_MEMSTATIC     (1<<7)
#define FLG_MEMOVERLAY    (1<<8)
#define FLG_OVLCONSTANTS  (1<<9) /* overlay constants */

int flags = 0;

long totalOut = 0, totalIn = 0;
long execAddr = 0;
static long errs[3] = {0,0,0};
void Note(int severity, const char *fmt, ...) {
    va_list ap;

    if (!severity && !(flags & FLG_VERBOSE))
	return;

    va_start(ap, fmt);
    switch (severity) {
    case 2:
	errs[2]++;
	fprintf(stderr, "ERROR: ");
	break;
    case 1:
	errs[1]++;
	fprintf(stderr, "Warning: ");
	break;
    case 0:
    default:
	errs[0]++;
	fprintf(stderr, "Note: ");
	break;
    }
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}



#ifdef WITH_COMPRESSED
static int escBits = 2, escMask = 0xc0;

#ifndef min
#define min(a,b) ((a<b)?(a):(b))
#endif

#define LRANGE    ((256-3)*256)
#define MAXLZLEN  256


#define OUT_SIZE 65536   /* output size--24kB should fit into 64kB */
static unsigned char outBuffer[OUT_SIZE];
static int outPointer = 0, flushNeeded = 0;
static int bitMask = 0x01;

static void PutBit(int bit) {
    if (bit && outPointer < OUT_SIZE)
	outBuffer[outPointer] |= bitMask;
    bitMask <<= 1;
    if (bitMask == 0x100) {
	bitMask = 0x01;
	outPointer++;
    }
}
static void FlushBits(void) {
    flushNeeded = 0;
    if (bitMask != 0x01)
	printf("fl ");
    if (bitMask != 0x01) {
	flushNeeded = 1;
	bitMask = 0x01;
	outPointer++;
    }
}


void PutNBits(int byte, int bits) {
    while (bits--)
	PutBit((byte & (1<<bits)));
}

void PutValue(int value) {
    int bits = 0, count = 0;

    while (value>1) {
	bits = (bits<<1) | (value & 1);	/* is reversed compared to value */
	value >>= 1;
	count++;
	PutBit(1);
    }
    if (count<7)
	PutBit(0);
    while (count--) {
	PutBit((bits & 1));	/* is reversed again */
	bits >>= 1;
    }
}

int LenValue(int value) {
    int count = 0;

    if (value<2)
	count = 0;
    else if (value<4)
	count = 1;
    else if (value<8)
	count = 2;
    else if (value<16)
	count = 3;
    else if (value<32)
	count = 4;
    else if (value<64)
	count = 5;
    else if (value<128)
	count = 6;
    else if (value<256)
	count = 7;
    if (count<7)
	return 2*count + 1;
    return 2*count;
}
static int lenValue[256];
void InitValueLen(void) {
    int i;
    for (i=1; i<256; i++)
	lenValue[i] = LenValue(i);
}

int LenLz(int lzlen, int lzpos) {
    if (lzlen==2) {
	return (lzpos <= 256) ? escBits + 2 + 8 : 100000;
    }
    return escBits + 8 + lenValue[((lzpos-1)>>8)+1] + lenValue[lzlen-1];
}


void OutputNormal(int *esc, unsigned char *data, int newesc) {
    if ((data[0] & escMask) == *esc) {
	PutNBits((*esc>>(8-escBits)), escBits);
	PutBit(0);
	PutBit(1);

	*esc = newesc;
	PutNBits((*esc>>(8-escBits)), escBits);
	PutNBits(data[0], 8-escBits);
    } else {
	PutNBits(data[0], 8);
    }
}

void OutputEof(int *esc) {
    /* EOF marker */
    PutNBits((*esc>>(8-escBits)), escBits);
    PutValue(3);
    PutValue(256-1);

    FlushBits();
}

void OutputLz(int *esc, int lzlen, int lzpos, char *data, int curpos) {
    PutNBits((*esc>>(8-escBits)), escBits);
    if (lzlen==2) {
	PutValue(lzlen-1);
	PutBit(0);
    } else {
	PutValue(lzlen-1);
	PutValue( ((lzpos-1) >> 8) +1);
    }
    PutNBits(((lzpos-1) & 0xff), 8);
}


static unsigned short *rle, *elr, *lzlen, *lzpos;
static int *length, inlen;
static unsigned char *indata, *mode, *newesc;
unsigned short *backSkip;

enum MODE {
    LITERAL = 0,
    LZ77 = 1,
    MMARK = 4
};

int OptimizeLength(int optimize) {
    int i;

    length[inlen] = 0; /* the "length" array length must be inlen+1 */
    for (i=inlen-1; i>=0; i--) {
	length[i] = 8 + length[i+1];
	mode[i] = LITERAL;
	if (lzlen[i]) {
	    int res = LenLz(lzlen[i], lzpos[i]) + length[i + lzlen[i]];

	    if (optimize && lzlen[i]>2) {
		int ii, mini = lzlen[i], minv = res;
		int topLen = LenLz(lzlen[i], lzpos[i]) - lenValue[lzlen[i]-1];

		/* Check only the original length and all shorter
		   lengths that are power of two.

		   Makes the assumption that the Elias Gamma Code is
		   used, i.e. values of the form 2^n are 'optimal' */
		ii = 4;
		while (lzlen[i] > ii) {
		    int v = topLen + lenValue[ii-1] + length[i + ii];
		    if (v < minv) {
			minv = v;
			mini = ii;
		    }
		    ii <<= 1;
		}
		if (backSkip[i] && backSkip[i] <= 256) {
		    int v = LenLz(2, (int)backSkip[i]) + length[i + 2];
		    if (v < minv) {
			minv = v;
			mini = 2;
			lzlen[i] = mini;
			res = minv;
			lzpos[i] = (int)backSkip[i];
		    }
		}
		if (minv != res && minv < length[i]) {
		    lzlen[i] = mini;
		    res = minv;
		}
	    }
	    if (res <= length[i]) {
		length[i] = res;
		mode[i] = LZ77;
	    }
	}
    }
    return length[0];
}


int OptimizeEscape(int *startEscape, int *nonNormal) {
    int i, j, states = (1<<escBits);
    long minp = 0, minv = 0, other = 0;
    long a[256], b[256]; /* needs int/long */
    int esc8 = 8-escBits;

    for (i=0; i<256; i++)
	b[i] = a[i] = -1;

    /* Mark bytes that are actually outputted */
    for (i=0; i<inlen; ) {
	if (mode[i] == LZ77) {
	    other++;
	    i += lzlen[i];
	} else {
	    mode[i++] = MMARK; /* mark it used */
	}
    }
    for (i=inlen-1; i>=0; i--) {
	if (mode[i] == MMARK) {
	    int k = (indata[i] >> esc8);

	    mode[i] = LITERAL;
	    /*
		k are the matching bytes,
		minv is the minimum value,
		minp is the minimum index
	     */
	    newesc[i] = (minp << esc8);
	    a[k] = minv + 1;
	    b[k] = b[minp] + 1;
	    if (k==minp) {
		minv++;
		for (k=states-1; k>=0; k--) {
		    if (a[k] < minv) {
			minv = a[k];
			minp = k;
			break;
		    }
		}
	    }
	}
    }
    /* Select the best initial escape */
    if (startEscape) {
	i = inlen;
	for (j=states-1; j>=0; j--) {
	    if (a[j] <= i) {
		*startEscape = (j << esc8);
		i = a[j];
	    }
	}
    }
    if (nonNormal)
	*nonNormal = other;
    return b[startEscape ? (*startEscape>>esc8) : 0];
}

int PackLz77(int len, unsigned char *data, int *startEscape) {
    int i, j, p;
    int escape;
    unsigned int *lastPair;

    inlen = len;
    indata = data;

    outPointer = 0;
    bitMask = 0x01;
    memset(outBuffer, 0, OUT_SIZE);

    length = (int *)calloc(sizeof(int), inlen + 1);
    mode = (unsigned char *)calloc(sizeof(unsigned char), inlen);
    rle = (unsigned short *)calloc(sizeof(unsigned short), inlen);
    elr = (unsigned short *)calloc(sizeof(unsigned short), inlen);
    lzlen = (unsigned short *)calloc(sizeof(unsigned short), inlen);
    lzpos = (unsigned short *)calloc(sizeof(unsigned short), inlen);
    newesc = (unsigned char *)calloc(sizeof(unsigned char), inlen);
    backSkip = (unsigned short *)calloc(sizeof(unsigned short), inlen);
    lastPair = (unsigned int *)calloc(sizeof(unsigned int), 256*256);

    if (!length || !mode || !rle || !elr || !lzlen || !lzpos || !newesc ||
	!lastPair || !backSkip) {
	fprintf(stderr, "Memory allocation failed!\n");
	goto errorexit;
    }

    for (p=0; p<inlen; p++) {
	/* check run-length code, LZ77 search needs it! */
	if (rle[p] <= 0) {
	    unsigned char *a = indata + p;
	    int val = *a++;
	    int top = inlen - p;
	    int rlelen = 1;

	    while (rlelen<top && *a++ == (unsigned char)val && rlelen < 65535)
		rlelen++;
	    if (rlelen>=2) {
		for (i=rlelen-1; i>=0; i--) {
		    rle[p+i] = rlelen-i;
		    elr[p+i] = i;	/* RLE backward skipping */
		}
	    }
	}
	/* check LZ77 code */
	if (p+rle[p]+1<inlen) {
	    int bot = p - LRANGE, maxval, maxpos, rlep = rle[p];
	    unsigned char valueCompare = indata[p+2];

	    if (rlep <= 0)
		rlep = 1;
	    if (bot < 0)
		bot = 0;
	    bot += (rlep-1);

	    i = (int)lastPair[ (indata[p]<<8) | indata[p+1] ] -1;
	    if (i>=0 && i>=bot) {
		maxval = 2;
		maxpos = p-i;

		i = (int)lastPair[(indata[p+(rlep-1)]<<8) | indata[p+rlep]] -1;
		while (i>=bot /* && i>=rlep-1 */) {
		    /* Equal number of A's ? */
		    if (!(rlep-1) || rle[i-(rlep-1)]==rlep) {
			if (indata[i+maxval-rlep+1] == valueCompare) {
			    unsigned char *a = indata + i+2;	/* match  */
			    unsigned char *b = indata + p+rlep-1+2;/* curpos */
			    int topindex = inlen-(p+rlep-1);

			    j = 2;
			    while (j < topindex && *a++==*b++)
				j++;
			    if (j + rlep-1 > maxval) {
				int tmplen = j+rlep-1, tmppos = p-i+rlep-1;

				if (tmplen > MAXLZLEN)
				    tmplen = MAXLZLEN;
				if (tmplen*8 - LenLz(tmplen, tmppos) >
				    maxval*8 - LenLz(maxval, maxpos)) {
				    maxval = tmplen;
				    maxpos = tmppos;
				    valueCompare = indata[p+maxval];
				}
				if (maxval == MAXLZLEN)
				    break;
			    }
			}
		    }
		    if (!backSkip[i])
			break;
		    i -= (int)backSkip[i];
		}
		if (p && rle[p-1] > maxval) {
		    maxval = rle[p-1] - 1;
		    maxpos = 1;
		}
		if (maxval < MAXLZLEN && rlep > maxval) {
		    bot = p - LRANGE;
		    if (bot < 0)
			bot = 0;
		    i = (int)lastPair[indata[p]*257] -1;
		    while (i>=bot) {
			if (elr[i] + 2 > maxval) {
			    maxval = min(elr[i] + 2, rlep);
			    maxpos = p - i + (maxval-2);
			    if(maxval == rlep)
				break;
			}
			i -= elr[i];
			if (!backSkip[i])
			    break;
			i -= (int)backSkip[i];
		    }
		}
		if (maxpos<=256 || maxval > 2) {
		    lzlen[p] = (maxval<MAXLZLEN)?maxval:MAXLZLEN;
		    lzpos[p] = maxpos;
		}
	    }
	}
	if (p+1<inlen) {
	    int index = (indata[p]<<8) | indata[p+1];
	    int ptr = p - (lastPair[index]-1);

	    if (ptr > p || ptr > 0xffff)
		ptr = 0;
	    backSkip[p] = ptr;
	    lastPair[index] = p+1;
	}
    }
    for (p=1; p<inlen; p++) {
	if (rle[p-1]-1 > lzlen[p]) {
	    lzlen[p] = (rle[p]<MAXLZLEN)?rle[p]:MAXLZLEN;
	    lzpos[p] = 1;
	}
    }
    for (p=0; p<inlen; p++) {
	rle[p] = 0;
    }

    {
	int mb = 0, mv = 8*OUT_SIZE;
	for (escBits=1; escBits<9; escBits++) {
	    int escaped, other = 0, c;

	    escMask = (0xff00>>escBits) & 0xff;
	    OptimizeLength(0);
	    escaped = OptimizeEscape(&escape, &other);
	    c = (escBits+3)*escaped + other*escBits;
	    if (c < mv) {
		mb = escBits;
		mv = c;
	    } else {
		break;
	    }
	}
	if (mb==1) {
	    int escaped;

	    escBits = 0;
	    escMask = 0;
	    OptimizeLength(0);
	    escaped = OptimizeEscape(&escape, NULL);
	    if (3*escaped < mv) {
		mb = 0;
	    }
	}
	escBits = mb;
	escMask = (0xff00>>escBits) & 0xff;
    }
    OptimizeLength(1);
    OptimizeEscape(&escape, NULL);
    if (startEscape)
	*startEscape = escape;

    for (p=0; p<inlen; ) {
	if (mode[p] == LZ77) {
	    OutputLz(&escape, lzlen[p], lzpos[p], indata+p-lzpos[p], p);
	    p += lzlen[p];
	} else {
	    OutputNormal(&escape, indata+p, newesc[p]);
	    p++;
	}
    }
    OutputEof(&escape);

errorexit:
    if (rle)
	free(rle);
    if (elr)
	free(elr);
    if (lzlen)
	free(lzlen);
    if (lzpos)
	free(lzpos);
    if (length)
	free(length);
    if (mode)
	free(mode);
    if (newesc)
	free(newesc);
    if (lastPair)
	free(lastPair);
    if (backSkip)
	free(backSkip);
    return 0;
}
#endif

#ifdef WITH_CRYPT
/*
  The minimum code for ARC4, not optimized for VSDSP.
  ARC4 creates a cipher stream that is xor:ed with the data stream to
  both encrypt and decrypt.

  Security is enhanced if the first cipher stream values are discarded
  and a random "crib" is included at the beginning of the data stream
  (and discarded when decrypted).

  If you encrypt one byte at a time, there will be no byte-order issues.
 */

unsigned char rc4key[256];
int rc4keylen = 0;

struct ARC4 {
    unsigned char S[256];
    unsigned int i, j;
} arc4;

/* KSA */
void arc4_init(unsigned char *key, unsigned int key_length) {
    int i, j;
    for (i = 0; i < 256; i++)
        arc4.S[i] = i;
    for (i = j = 0; i < 256; i++) {
        unsigned char temp;
        j = (j + key[i % key_length] + arc4.S[i]) & 255;
        temp = arc4.S[i];
        arc4.S[i] = arc4.S[j];
        arc4.S[j] = temp;
    }
    arc4.i = arc4.j = 0;
}
 
/* PRGA */
unsigned char arc4_output(void) {
    unsigned char temp;
 
    arc4.i = (arc4.i + 1) & 255;
    arc4.j = (arc4.j + arc4.S[arc4.i]) & 255;
 
    temp = arc4.S[arc4.j];
    arc4.S[arc4.j] = arc4.S[arc4.i];
    arc4.S[arc4.i] = temp;
 
    return arc4.S[(temp + arc4.S[arc4.j]) & 255];
}

/* encrypt / decrypt */
void arc4_crypt(unsigned char *data, unsigned int data_length) {
    int i;
    for (i=0;i<data_length;i++) {
	*data++ ^= arc4_output();
    }
}

int ParseKeyOrMask(unsigned char *km, unsigned char *p) {
    int len = 0;
    while (p[-1]) {
	char t[3];
	char *q;
	t[0] = p[-2];
	t[1] = p[-1];
	t[2] = '\0';
	if (p[-2]) { /* two */
	    *km++ = strtol(t, &q, 16);
	    p -= 2;
	} else {
	    *km++ = strtol(t+1, &q, 16);
	    p--;
	}
	if (!q || *q) {
	    fprintf(stderr, "error in key: '%s' unexpected\n", p);
	    break;
	}
	len++;
	if (len >= 8)
	    break;
    }
    return len;
}
#endif/*WITH_CRYPT*/


enum IMGTYPE {
    vs1000_nand = 0,
    vs1000_raw,
    vs1000_spi,
    vs1000_ramdisk,
    vs1005_nand,
    vs1005_raw,
    vs1005f_spi,
    vs1005_spi,
    vs1005g_sd,
    vs10xx_spi,
    vs10xx_plugin,
    vs10xx_cmd,
} imgType = vs1000_nand;

int nandType = 2; /* 1 = LARGEPAGE 2 = ADDRBYTES3 4 = ADDRBYTES4 */
int nandWaitNs = 90;
int blockSizeBits = 5; /* 32 */
int flashSizeBits = 15; /* 16M */

int PutHeader(FILE *fp, int addr) {
    if (!fp)
	return 0;

    if (imgType == vs10xx_plugin) {
	static const char pluginHeader[] =
	    "/* User application code loading tables for VS10xx */\n"
	    "#if 0\n"
	    "void LoadUserCode(void) {\n"
	    "  int i = 0;\n"
	    "\n"
	    "  while (i<sizeof(plugin)/sizeof(plugin[0])) {\n"
	    "    unsigned short addr, n, val;\n"
	    "    addr = plugin[i++];\n"
	    "    n = plugin[i++];\n"
	    "    if (n & 0x8000U) { /* RLE run, replicate n samples */\n"
	    "      n &= 0x7FFF;\n"
	    "      val = plugin[i++];\n"
	    "      while (n--) {\n"
	    "        WriteVS10xxRegister(addr, val);\n"
	    "      }\n"
	    "    } else {           /* Copy run, copy n samples */\n"
	    "      while (n--) {\n"
	    "        val = plugin[i++];\n"
	    "        WriteVS10xxRegister(addr, val);\n"
	    "      }\n"
	    "    }\n"
	    "  }\n"
	    "}\n"
	    "#endif\n"
	    "\n"
	    "#ifndef SKIP_PLUGIN_VARNAME\n"
	    "const unsigned short plugin[] = { /* Compressed plugin */\n"
	    "#endif\n"
	    ;
	fprintf(fp, "%s", pluginHeader);
    }
    if (imgType == vs10xx_cmd) {
	fprintf(fp, "#Legacy command file\n#W 2 <reg> <data>\n");
    }

    if (imgType == vs1000_spi || imgType == vs1005_spi || imgType == vs1005f_spi) {
	unsigned char head[16];
	int cnt = 0;

	head[cnt++] = 'V';
	head[cnt++] = 'L';
	head[cnt++] = 'S';
	head[cnt++] = (imgType == vs1005_spi) ? '5' : 'I';
	fwrite(head, 1, cnt, fp);
	return cnt;
    }

    if (imgType == vs10xx_spi) {
	unsigned char head[16];
	int cnt = 0;

	head[cnt++] = 'P';
	head[cnt++] = '&';
	head[cnt++] = 'H';
	fwrite(head, 1, cnt, fp);
	return cnt;
    }

    if (imgType == vs1000_nand || imgType == vs1005_nand) {
	unsigned char head[32];
	int cnt = 0;

	head[cnt++] = 'V'; /* exec */
	head[cnt++] = 'L'; /* exec */
	head[cnt++] = (imgType == vs1005_nand) ? 'N' : 'S';
	head[cnt++] = (imgType == vs1005_nand) ? '5' : 'I';
	head[cnt++] = nandType>>8; /*nandType hi */
	head[cnt++] = nandType;    /*nandType lo */
	head[cnt++] = blockSizeBits;
	head[cnt++] = flashSizeBits;
	head[cnt++] = nandWaitNs/256;
	head[cnt++] = nandWaitNs;
	head[cnt++] = 0xff; /* number of boot sectors hi */
	head[cnt++] = 0xff; /* number of boot sectors lo */

	if (imgType == vs1005_nand) {
	    /* word 6: number of extra words before boot record */
	    head[cnt++] = 0;
	    head[cnt++] = 1;
	    /* word 7: if exists (numExtra > 0), the erase block number
	       of the start of mapper area (including bad erase blocks
	       or not including bad erase blocks?) */
	    head[cnt++] = 0;
	    head[cnt++] = 2;
	}

	printf("NandType: %d %s %s addr, %dkB blocks, %dMB flash\n", nandType,
	       (nandType & 1) ? "Large-Page" : "Small-Page",
	       (nandType & 2) ?
	       ((nandType & 4) ? "6-byte" : "5-byte") : "4-byte",
	       (1<<blockSizeBits)/2,
	       (1<<flashSizeBits)/2048);

	if (addr != 0xffff) {
	    head[cnt++] = 'B';
	    head[cnt++] = 'o';
	    head[cnt++] = 'O';
	    head[cnt++] = 't';
	}

	fwrite(head, 1, cnt, fp);
	return cnt;
    }
    return 0;
}


int pluginWords = 0;
long imgStartByte = 0;
void FixHeader(FILE *fp) {
    if (!fp)
	return;
    if (imgType == vs1000_nand || imgType == vs1005_nand) {
	long pos = ftell(fp);
	long size = (pos-imgStartByte+511) / 512;
printf("size %ld blocks\n", size);
	fseek(fp, imgStartByte+10, SEEK_SET);
	fputc(size>>8, fp);
	fputc(size, fp);
	fseek(fp, pos, SEEK_SET);
	return;
    }
    if (imgType == vs10xx_plugin) {
	fprintf(fp,"#define PLUGIN_SIZE %d\n#ifndef SKIP_PLUGIN_VARNAME\n};\n#endif\n",
		pluginWords);
    }
}

unsigned long encryptionKey = //0xaaaaaaaa ^
#if 1
0
#endif
;

/*
FUNCTION: SavePack()
DESCRIPTION: Writes a data block to a file.
  0, 1, and 2 are used for uncompressed I, X, and Y blocks.
INPUTS:
RESULT:
 */
int SavePack(FILE *fp, int page, unsigned char *origData, int origSize,
	     int addr) {
    if (!fp)
	return 0;

    if (imgType == vs1005g_sd || 
	imgType == vs1000_ramdisk) {
	unsigned char blk[512];
	int cnt = 0;
	/* output 512-byte sectors */
	if (origData == NULL) {
	    /* exec */
	    if (addr != 0xffff) {
		memset(blk, 0, sizeof(blk));
		blk[cnt++] = 0x80; /* exec */
		blk[cnt++] = 0x03; /* exec */
		blk[cnt++] = 0;
		blk[cnt++] = 0;
		blk[cnt++] = (addr>>8);
		blk[cnt++] = (addr & 255);
		cnt = 512;

		fwrite(blk, 1, cnt, fp);
	    }
	    return cnt;

	} else {
	    while (origSize) {
		/* 126 leaves 1 word for end marker */
		int words = 2*116, idx = 0;
		if (page == 2) {
		    words -= 1;
		}
		if (words > origSize/2) {
		    words = origSize/2;
		}
		memset(blk, 0, sizeof(blk));
		blk[idx++] = 0x80;
		blk[idx++] = page;
		blk[idx++] = (words - 1) >> 8;
		blk[idx++] = (words - 1) & 255;
		blk[idx++] = (addr>>8);
		blk[idx++] = (addr & 255);

		memcpy(blk + idx, origData, words*2);
		idx += words*2;
		if (imgType == vs1000_ramdisk) {
		    /* BootFromX() has a small bug in Y write in VS1000BC,
		       add a dummy word. */
		    if (page == 2) {
			idx += 2;
		    }
		    /* Force ok return from rammapper write of sector
		       0x12345678
		       j 0x833d
		       ldc 1,a0

		       80 00 00 03 00 10   28 20 cf 40 00 00 00 40
		       80 03 00 00 00 10
		    */
		    memcpy(blk+idx,
			   "\x80\x00\x00\x03\x00\x10"
			   "\x28\x20\xcf\x40\x00\x00\x00\x40"
			   "\x80\x03\x00\x00\x00\x10", 20); 
		}		    
		fwrite(blk, 1, 512, fp);

		origSize -= words*2;
		origData += words*2;
		if (page == 0) {
		    addr += words/2;
		} else {
		    addr += words;
		}
		cnt += 512;
	    }
	    return cnt;

	}
	return 0;
    }
    if (imgType == vs10xx_spi) {
	unsigned char head[16];
	int cnt = 0;

	if (origData == NULL) {
	    if (addr != 0xffff) {
		head[cnt++] = 3; /* exec */
		head[cnt++] = 0;
		head[cnt++] = 0;
		head[cnt++] = (addr>>8);
		head[cnt++] = (addr & 255);

		fwrite(head, 1, cnt, fp);
	    }
	    fputc(0xff, fp); /* one extra for word-align */
	    return cnt + 1;
	} else {
	    head[cnt++] = page;
	    head[cnt++] = (origSize>>8);
	    head[cnt++] = (origSize & 255);
	    head[cnt++] = (addr>>8);
	    head[cnt++] = (addr & 255);

	    fwrite(head, 1, cnt, fp);

	    if (page == 0 && addr >= 0x30 && encryptionKey) {
		int i;
		unsigned long key = encryptionKey;
		printf("Key %08lx\n", encryptionKey);
		for (i=0;i<origSize/4;i++) {
		    unsigned long code =
			((unsigned long)origData[4*i+0]<<24) |
			((unsigned long)origData[4*i+1]<<16) |
			((unsigned long)origData[4*i+2]<<8) |
			((unsigned long)origData[4*i+3]<<0);
//printf("0x%08lx 0x%08lx\n", code, key);
		    code ^= key;
//printf("0x%08lx\n", code);
		    if (key & 0x80000000UL) {
			key = (key<<1) | 1;
		    } else {
			key <<= 1;
		    }
		    origData[4*i+0] = code>>24;
		    origData[4*i+1] = code>>16;
		    origData[4*i+2] = code>>8;
		    origData[4*i+3] = code>>0;
		}
	    }
	    fwrite(origData, origSize, 1, fp);
	    return cnt + origSize;
	}
    }

    /* TODO: convert to compressed records if possible. */

    if (imgType == vs1000_spi || imgType == vs1005_spi ||
	imgType == vs1005f_spi) {
	unsigned char head[16];
	int cnt = 0;

	if (origData == NULL) {
	    if (addr != 0xffff) {
		head[cnt++] = 3; /* exec */
		head[cnt++] = 0;
		head[cnt++] = 0;
		if (imgType == vs1000_spi) {
		    head[cnt++] = (addr & 255);
		    head[cnt++] = (addr>>8);
		} else {
		    head[cnt++] = (addr>>8);
		    head[cnt++] = (addr & 255);
		}

		fwrite(head, 1, cnt, fp);
	    }
	    fputc(0xff, fp); /* one extra for word-align */
	    return cnt;
	} else {
#ifdef WITH_COMPRESSED
	    if (!(flags & FLG_NOCOMPRESSION) &&
		(imgType == vs1005_spi ||
		 (imgType == vs1005f_spi &&
		  addr < 0x4000 && addr + origSize <= 0x4000))) {
		int startEscape, n = 1;
#ifdef WITH_CRYPT
		if (page != 7)
#endif
		    n = PackLz77(origSize, origData, &startEscape);
		if (!n) {
		    if (outPointer > OUT_SIZE) {
			printf("%d > buffer %d ", outPointer, OUT_SIZE);
			//SavePack(fp, page, data, len, outBuffer, len+1000, addr,
			//startEscape, escBits);
		    } else if (
#ifdef WITH_CRYPT
			!(flags & FLG_CRYPTED) &&
#endif
			outPointer >= origSize) {
			printf("%5d >=%5d ", outPointer, origSize);
		    } else {
			if (outPointer < origSize) {
			    printf("%5d < %5d " , outPointer, origSize);
			} else {
			    printf("%5d >=%5d ", outPointer, origSize);
			}
			//printf("(%02x %d) ", startEscape, escBits);

			head[cnt++] = 4 | page;
			head[cnt++] = ((outPointer+2)>>8);
			head[cnt++] = ((outPointer+2) & 255);
			head[cnt++] = (addr>>8);
			head[cnt++] = (addr & 255);

			head[cnt++] = (startEscape>>(8-escBits));
			head[cnt++] = escBits;
#ifdef WITH_CRYPT
			if (flags & FLG_CRYPTED) {
			    arc4_crypt(&head[cnt-2], 2);
			    arc4_crypt(outBuffer, outPointer);
			}
#endif
			fwrite(head, 1, cnt, fp);
			fwrite(outBuffer, outPointer, 1, fp);
			return cnt + outPointer;
		    }
		}
	    }
#endif

	    head[cnt++] = page;
	    if (imgType == vs1000_spi) {
		head[cnt++] = (origSize & 255);
		head[cnt++] = (origSize>>8);
		head[cnt++] = (addr & 255);
		head[cnt++] = (addr>>8);
	    } else {
		head[cnt++] = (origSize>>8);
		head[cnt++] = (origSize & 255);
		head[cnt++] = (addr>>8);
		head[cnt++] = (addr & 255);
	    }

	    fwrite(head, 1, cnt, fp);
	    fwrite(origData, origSize, 1, fp);
	    return cnt + origSize;
	}
    }

    if (imgType == vs1000_nand || imgType == vs1000_raw ||
	imgType == vs1005_nand || imgType == vs1005_raw) {
	unsigned char head[16];
	int cnt = 0;

	if (origData == NULL) {
	    if (addr != 0xffff) {
		head[cnt++] = 0x80; /* exec */
		head[cnt++] = 0x03; /* exec */
		head[cnt++] = 0;
		head[cnt++] = 0;
		head[cnt++] = (addr>>8);
		head[cnt++] = (addr & 255);

		fwrite(head, 1, cnt, fp);
	    }
	    return cnt;
	} else {
#ifdef WITH_COMPRESSED
	    if ((imgType == vs1005_nand || imgType == vs1005_raw)
		&& !(flags & FLG_NOCOMPRESSION)) {
		int startEscape, n = 1;
#ifdef WITH_CRYPT
		if (page != 7)
#endif
		    n = PackLz77(origSize, origData, &startEscape);
		if (!n) {
		    if (outPointer > OUT_SIZE) {
			printf("%d > buffer %d ", outPointer, OUT_SIZE);
			//SavePack(fp, page, data, len, outBuffer, len+1000, addr,
			//startEscape, escBits);
		    } else if (
#ifdef WITH_CRYPT
			!(flags & FLG_CRYPTED) &&
#endif
			outPointer >= origSize) {
			printf("%5d >=%5d ", outPointer, origSize);
		    } else {
			if (outPointer < origSize) {
			    printf("%5d < %5d " , outPointer, origSize);
			} else {
			    printf("%5d >=%5d ", outPointer, origSize);
			}
			if (outPointer & 1) {
			    /* gets word-aligned, but only the actual
			       bytes encrypted */
			    printf("*");
			}

			cnt = 0;
			head[cnt++] = 0x80;
			head[cnt++] = 4 | page;
			head[cnt++] = ((outPointer+1)/2 /*+1 -1*/) >> 8;
			head[cnt++] = ((outPointer+1)/2 /*+1 -1*/) & 255;
			head[cnt++] = (addr>>8);
			head[cnt++] = (addr & 255);

			/* these are actually part of the data,
			   and account for the +1 in the size field. */
			head[cnt++] = (startEscape>>(8-escBits));
			head[cnt++] = escBits;
#ifdef WITH_CRYPT
			if (flags & FLG_CRYPTED) {
			    arc4_crypt(&head[cnt-2], 2);
			    arc4_crypt(outBuffer, outPointer);
			}
#endif
			//printf("%02x %d ", startEscape, escBits);

			fwrite(head, 1, cnt, fp);
			fwrite(outBuffer, 2, (outPointer+1)/2, fp);
			return cnt + outPointer;
		    }
		}
	    }
#endif
	    head[cnt++] = 0x80;
	    head[cnt++] = page;
	    head[cnt++] = (origSize/2 - 1) >> 8;
	    head[cnt++] = (origSize/2 - 1) & 255;
	    head[cnt++] = (addr>>8);
	    head[cnt++] = (addr & 255);

	    fwrite(head, 1, cnt, fp);
	    fwrite(origData, origSize, 1, fp);
	    /* BootFromX() still has a small bug in Y write in VS1000BC,
	       add a dummy word. */
	    if (page == 2 && (imgType == vs1000_nand || imgType == vs1000_raw)) {
		cnt += 2;
		fwrite("\0\0", 2, 1, fp);
	    }
	    return cnt + origSize;
	}
    }
    return 0;
}

#ifndef min
#define min(a,b) ((a<b)?(a):(b))
#endif


/******************************** COFF ********************************/

#ifndef COFF_MAGIC16
#define COFF_MAGIC16  0xb5b4
#define COFF_MAGIC    0xb5b3
#define OPTMAGIC      0xb502
#define CF_EXEC     0x002
#define STYP_TEXT   0x020
#define STYP_DATA   0x040
#define STYP_BSS    0x080
#define STYP_DATAX  0x200
#define STYP_DATAY  0x400
#endif

unsigned short GetShort(FILE *fp, char swap) {
    unsigned short c1, c2;
    c1 = fgetc(fp);
    c2 = fgetc(fp);
    return swap ? ((c2<<8) | c1) : ((c1<<8) | c2);
}

unsigned long GetLong(FILE *fp, char swap) {
    unsigned long c1, c2, c3, c4;
    c1 = fgetc(fp);
    c2 = fgetc(fp);
    c3 = fgetc(fp);
    c4 = fgetc(fp);
    return swap ? ((c4<<24) | (c3<<16) | (c2<<8) | c1) :
	((c1<<24) | (c2<<16) | (c3<<8) | c4);
}

/******************************** COFF ********************************/

FILE *outFp = NULL;
//int totalIn = 0, totalOut = 0;
//int execAddr = 0;

#define MAX_SIZE 262144
struct AddrDat {
  unsigned int addr;
  unsigned int data;
};
int nRle = 0, nCopy = 0;
int nWork = 0;
struct AddrDat work[MAX_SIZE];
int outCols = 0;

void PluginDump(FILE *fp, int end) {
  int i = end - nCopy - nRle;
  if ((nRle < 3 && nCopy && work[i].addr == work[i+nCopy].addr) || 
       (!nCopy && nRle < 2)) {
    nCopy += nRle;
    nRle = 0;
  }
  if (nCopy) {
      fprintf(fp, "\t0x%04x,0x%04x, /*copy %d*/\n", work[i].addr, nCopy, nCopy);
      outCols = 0;
      pluginWords += 2;
    while (nCopy--) {
	pluginWords++;
	fprintf(fp, "%s0x%04x,", outCols? "": "\t", work[i].data);
	if (++outCols >= 8) {
	    outCols = 0;
	    fprintf(fp, "\n");
	}
	i++;
    }
    if (outCols) {
	fprintf(fp, "\n");
    }
  }
  if (nRle) {
      fprintf(fp, "\t0x%04x, 0x%04x, 0x%04x, /*Rle(%d)*/\n",
	      work[i].addr, nRle|0x8000U, work[i].data, nRle);
      outCols = 0;
      pluginWords += 3;
  }
  nRle = 1;
  nCopy = 0;
}

void SendFlashBlock(FILE *fp, int page, unsigned char *data, short len,
		    long addr) {
    int out;
    if (imgType == vs10xx_cmd) {
	int i;
	printf("\n");
	if (len == 0 || data == NULL) { /*execute record*/
	    unsigned char tmp[2];
	    if (addr == 0) {
		fprintf(stderr, "No execute address is set!\n");
		return; /* has not been set */
	    }
	    //fprintf(stderr, "addr is %ld!\n", addr);
	    tmp[0] = addr>>8;
	    tmp[1] = addr;
	    addr = 0xc00a;
	    len = 2;
	    data = tmp;
	} else if (page == 0) {
	    addr |= 0x8000;
	} else if (page == 1) {
	    if (addr >= 0x0000 && addr <= 0x3fff) {
		/* XRAM */
	    } else if (addr >= 0xc000 && addr <= 0xdfff) {
		/* XIO */
	    } else {
		/* not supported! */
		fprintf(stdout, "X address 0x%04lx not supported!\n", addr);
		exit(10);
	    }
	} else if (page == 2) {
	    if (addr >= 0x0000 && addr <= 0x3fff) {
		/* YRAM */
		addr |= 0x4000;
	    } else if (addr >= 0xe000 && addr <= 0xffff) {
		/* YEXTRA */
	    } else {
		/* not supported! */
		fprintf(stdout, "Y address 0x%04lx not supported!\n", addr);
		exit(10);
	    }
	}
	if (addr >= 0xc000 && addr + len/2 <= 0xc00f) {
	    /* direct writes to SCI registers */
	    i = 0;
	    while (i < len/2) {
		int t = (data[2*i+0] << 8) | data[2*i+1];
		fprintf(fp, "W 2 %x %04x\n", (short)addr & 15, t);
		addr++;
		i++;
	    }
	} else {
	    /* through WRAM/WRAMADDR */
	    fprintf(fp, "W 2 7 %04x\n", (int)addr);

	    i = 0;
	    while (i < len/2) {
		int t = (data[2*i+0] << 8) | data[2*i+1];
		fprintf(fp, "W 2 6 %04x\n", t);
		i++;
	    }
	}
	return;
    }
    if (imgType == vs10xx_plugin) {
	int i;
	long myaddr = addr;
	printf("\n");
	if (len == 0 || data == NULL) { /*execute record*/
	    unsigned char tmp[2];
	    data = tmp;
	    tmp[0] = addr >> 8;
	    tmp[1] = addr;
	    len = 2;
	    myaddr = 0xc00a; /*AIADDR*/
	    if (addr == 0)
		return; /* has not been set */
	} else if (page == 0) {
	    myaddr |= 0x8000;
	} else if (page == 1) {
	    if (addr >= 0x0000 && addr <= 0x3fff) {
		/* XRAM */
	    } else if (addr >= 0xc000 && addr <= 0xdfff) {
		/* XIO */
	    } else {
		/* not supported! */
		fprintf(stdout, "X address 0x%04lx not supported!\n", addr);
		exit(10);
	    }
	} else if (page == 2) {
	    myaddr |= 0x4000;
	    if (addr >= 0x0000 && addr <= 0x3fff) {
		/* YRAM */
	    } else if (addr >= 0xe000 && addr <= 0xffff) {
		/* YEXTRA */
	    } else {
		/* not supported! */
		fprintf(stdout, "Y address 0x%04lx not supported!\n", addr);
		exit(10);
	    }
	}
	if (myaddr >= 0xc000 && myaddr + len/2 <= 0xc00f) {
	    /* direct writes to SCI registers */
	    i = 0;
	    nWork = 0;
	    while (i < len/2) {
		int t = (data[2*i+0] << 8) | data[2*i+1];
		work[nWork].addr = myaddr & 15;
		work[nWork].data = t;
		nWork++;
		myaddr++;
		i++;
	    }
	} else {
	    /* through WRAM/WRAMADDR */
	    i = 0;
	    nWork = 0;
	    work[nWork].addr = 7;
	    work[nWork].data = myaddr;
	    nWork++;
	    while (i < len/2) {
		int t = (data[2*i+0] << 8) | data[2*i+1];
		work[nWork].addr = 6;
		work[nWork].data = t;
		nWork++;
		i++;
	    }
	}
	/* just a direct copy from cmdtoplg.c */
	nRle = 1;
	nCopy = 0;
	for (i=1; i<nWork; i++) {
	    if (work[i].addr != work[i-1].addr) {
		/* Always causes dump because address changes */
		/*NEVER in this prog!*/
//fprintf(fp, "Dump addr %04x != %04x\n", work[i].addr, work[i-1].addr);
		PluginDump(fp, i);
	    } else if (work[i].data != work[i-1].data) {
		if (nRle < 6) {
		    nCopy += nRle;
		    nRle = 1;
		} else {
//fprintf(fp, "Dump rle\n");
		    PluginDump(fp, i);
		}
	    } else {
		nRle++;
	    }
	}
//fprintf(fp, "last Dump\n");
	PluginDump(fp, i);
	return;
    }


    if (len) {
	out = SavePack(fp, page, data, len, addr);
	totalIn += len;
	totalOut += out;
	printf("In:%5d, out:%5d\n", len,  out);
    } else {
	out = SavePack(fp, 0, NULL, 0, addr);
	totalOut += out;
	printf("In: %ld, out: %ld\n", totalIn, totalOut);

	if (imgType == vs1000_nand || imgType == vs1005_nand) {
	    if (totalOut > 8192) {
		if (flags & FLG_IGNORESIZE) {
		    fprintf(stderr, "maximum out size 8192 ignored!\n");
		} else {
		    fprintf(stderr, "maximum out size is 8192!\n");
		    exit(10);
		}
	    }
	}
    }
}

struct MYSECT {
    int page;
    long addr;
    long size; /* size in words */
    long len;  /* size in bytes */
    unsigned char *data;
};
#define MAX_SECT 4000
struct MYSECT block[MAX_SECT];
int blockNum = 0;

/* Note: you have to set blockNum to 0 before calling this if you want
         to add only the file from fname. */
int SendFlashData(int putHeader, int addr, const char *fname, const char *fileOut) {
    FILE *fp;
    unsigned short c1, c2;
    unsigned short magic1, magic2;
    unsigned short f_nscns;
    unsigned short f_opthdr;
    unsigned short f_flags;
    char swap = 0;
    int sectNum;

    //blockNum = 0;
    fp = fopen(fname, "rb");
    if (!fp) {
	printf("could not open %s\n", fname);
	return 10;
    }
    imgStartByte = ftell(fp);
    c1 = fgetc(fp);
    c2 = fgetc(fp);
    magic1 = (c1<<8) | c2;
    magic2 = (c2<<8) | c1;
    if (magic1 == COFF_MAGIC) {
	swap = 0;
    } else if (magic2 == COFF_MAGIC) {
	swap = 1;
    } else if (magic1 == COFF_MAGIC16) {
	swap = 0;
    } else if (magic2 == COFF_MAGIC16) {
	swap = 1;
    } else {
	printf("not a COFF file: %s\n", fname);
	fclose(fp);
	return 10;
    }
    f_nscns = GetShort(fp, swap);
    GetLong(fp, swap);
    GetLong(fp, swap);
    GetLong(fp, swap);
    f_opthdr = GetShort(fp, swap);
    f_flags = GetShort(fp, swap);
    if (!(f_flags & CF_EXEC)) {
	fclose(fp);
	printf("%s not executable\n", fname);
	return 10;
    }
    if (f_opthdr) {
	fseek(fp, f_opthdr, SEEK_CUR);
    }
    sectNum = 0;
    while (sectNum < f_nscns) {
	long s_paddr, s_size, s_scnptr, s_flags;

	sectNum++;
	for (c1=0; c1<8; c1++) {
	    fgetc(fp);
	}
	s_paddr = GetLong(fp, swap);
	fseek(fp, 4, SEEK_CUR);
	s_size = GetLong(fp, swap);
	s_scnptr = GetLong(fp, swap);
	fseek(fp, 4+4+2+2, SEEK_CUR);
	s_flags = GetLong(fp, swap);

	if (!s_size || (s_flags & STYP_BSS))
	    continue; /* Skip BSS */

	if (blockNum < MAX_SECT) {
	    int page = 0, ss = 0;
	    unsigned char *data = NULL;
	    long filePos;

	    s_size /= 4; /* size in words */
	    if (s_flags & STYP_TEXT) {
		page = 0;
	    } else if ((s_flags & STYP_DATAX)) {
		page = 1;
	    } else if ((s_flags & STYP_DATAY)) {
		page = 2;
	    }
	    /* Check merging possibilities --
	       increases the compression ratio and reduce overhead */
	    if (flags & FLG_NOMERGE) {
	    } else
	    for (ss = 0; ss < blockNum; ss++) {
		if (page == block[ss].page &&
		    s_paddr == block[ss].addr + block[ss].size
#if 1 /*limit the size because compressed records can only be 32kB uncompr.*/
		    && block[ss].size + s_size < 0x2000
#endif
		    ) {
		    /* Add to the tail */
		    unsigned char *tmp = realloc(block[ss].data,
						 block[ss].len +
						 s_size * ((page == 0)?4:2));
		    if (tmp) {
			block[ss].data = tmp;
			data = tmp + block[ss].len; /* point to tail */
			block[ss].size += s_size; /* size in words */
			block[ss].len += s_size * ((page == 0) ? 4 : 2);
			ss = -1;
			break;
		    }
		}
		if (page == block[ss].page &&
		    s_paddr + s_size == block[ss].addr
#if 1 /*limit the size because compressed records can only be 32kB uncompr.*/
		    && block[ss].size + s_size < 0x2000
#endif
		    ) {
		    /* Add to the head -- although quite rare, mainly
		       data memory (init/const) */
		    unsigned char *tmp = malloc(block[ss].len +
						s_size * ((page == 0)? 4 : 2));
		    if (tmp) {
			/* copy old data to the tail */
			memcpy(tmp + s_size * ((page == 0)?4:2),
			       block[ss].data, block[ss].len);
			free(block[ss].data);
			block[ss].data = tmp;
			data = tmp; /* point to head -- new data here */
			block[ss].size += s_size; /* size in words */
			block[ss].len += s_size * ((page == 0) ? 4 : 2);
			block[ss].addr = s_paddr;
			ss = -1;
			break;
		    }
		} else {
#if 0
		    printf("page %d block[ss].page %d, %04x+%04x=%04x %04x\n",
			   page, block[ss].page,
			   s_paddr, s_size, s_paddr+s_size, block[ss].addr);
#endif
		}
	    }
	    if (ss != -1) {
		/* create new header */
		block[blockNum].page = page;
		block[blockNum].addr = s_paddr; /* physical addr! */
		block[blockNum].size = s_size;
		block[blockNum].len = s_size * ((page == 0) ? 4 : 2);
		data = calloc(s_size, (page == 0) ? 4 : 2);
		block[blockNum].data = data;
		blockNum++;
	    }
	    /* Add data */
	    filePos = ftell(fp); /* remember current location */
	    fseek(fp, s_scnptr, SEEK_SET); /* jump to start of data */
	    if (page == 0) {
		while (s_size--) {
		    unsigned long b = GetLong(fp, swap);
		    if (imgType == vs1000_spi) {
			*data++ = b>>0;
			*data++ = b>>8;
			*data++ = b>>16;
			*data++ = b>>24;
		    } else {
			*data++ = b>>24;
			*data++ = b>>16;
			*data++ = b>>8;
			*data++ = b>>0;
		    }
		}
	    } else {
		while (s_size--) {
		    unsigned long b = GetLong(fp, swap);
		    if (imgType == vs1000_spi) {
			*data++ = b>>0;
			*data++ = b>>8;
		    } else {
			*data++ = b>>8;
			*data++ = b>>0;
		    }
		}
	    }
	    fseek(fp, filePos, SEEK_SET); /* restore location */
	} else {
	    puts("too many sections");
	    return 10;
	}
    }

    outFp = fopen(fileOut, "wb");
    if (!outFp) {
	printf("could not open %s for writing\n", fileOut);
	return 10;
    }
    if (putHeader)
	totalOut += PutHeader(outFp, addr);
    sectNum = 0;
#ifdef WITH_CRYPT
    if (imgType != vs1005_spi && imgType != vs1005_nand && imgType != vs1005_raw) {
	flags &= ~FLG_CRYPTED;
    }
    if (flags & FLG_CRYPTED) {
	int i;
	// Add random crib
	// This prevents any record going over 64kB in the internal flash
	// to be of any use even when the device ID and bitmask is known.
#ifdef WIN32
	srand(time(NULL));
	for (i=0;i<8;i++) {
	    rc4key[16+i] = rand();
	}
	rc4keylen += 8;
#else
	srandom(time(NULL));
	for (i=0;i<8;i++) {
	    rc4key[16+i] = random();
	}
	rc4keylen += 8;
#endif
	printf("7: decrypt key %d bytes\n", rc4keylen);
	totalOut = SavePack(outFp, 7, rc4key+8, rc4keylen, 0);
	/* Mask key with ID */
	for (i=0;i<8;i++) {
	    rc4key[8+i] &= rc4key[i];
	}
	/* Initialize key */
#if 0
	for (i=0;i<rc4keylen;i++)
	    printf("%02x\n", rc4key[8+i]);
#endif
	arc4_init(rc4key+8, rc4keylen);
	printf("\n");
    }
#endif
    while (sectNum < blockNum) {
	static const char pstr[] = "IXY";
	printf("%c: 0x%04lx-0x%04lx ",
	       pstr[block[sectNum].page], block[sectNum].addr,
	       block[sectNum].addr + block[sectNum].size-1);
	SendFlashBlock(outFp,
		       block[sectNum].page, block[sectNum].data,
		       block[sectNum].len, block[sectNum].addr);
	sectNum++;
    }
	if (addr != 0xffff) {
      printf("Exec: 0x%04x\n", addr);
	} else {
      printf("No Exec\n");
	}
    SendFlashBlock(outFp, 0, "", 0, addr);
    if (putHeader)
	FixHeader(outFp);
    fclose(outFp);
    fclose(fp);
    return 0;
}



/***************************************************************************/
const char *RegName(int regnum) {
    static const char *regs[] = {
	"A0", "A1", "B0", "B1", "C0", "C1", "D0", "D1",
	"LR0", "LR1", "MR0", "MR1", "NULL", "LC", "LS", "LE",
	"I0", "I1", "I2", "I3", "I4", "I5", "I6", "I7",
	"I8", "I9", "I10", "I11", "I12", "I13", "I14", "I15",
	"A2", "B2", "C2", "D2", "NOP"
    };
    if (version >= 2 && regnum == REG_IPR0)
	return "IPR0";
    if (version >= 2 && regnum == REG_IPR1)
	return "IPR1";
    if (regnum >= 0 && regnum <= REG_NOP)
	return regs[regnum];
    return "<R_RES>";
}
int CustomUnAsm(struct instruction *i, char *out)
{
    return 1;
}



#define MAXIMGSIZE 1048576
int ALIGNMENT = 512; //alignment for overlay start addresses in nand image
#define MASK_I  1
#define MASK_X  2
#define MASK_Y  4
#define MASK_E  8
#define MASK_C  16
#define MASK_HEADER 128

unsigned char *img = NULL;//[MAXIMGSIZE];
int imgp = 0;
unsigned short ovlChecksum = 0;
unsigned char checksumHi = 0;
int isVS1000 = 0;
FILE *logfile = NULL;

void putimg(unsigned char b) {
    img[imgp] = b;
    if (imgp & 1) {
	ovlChecksum += checksumHi * 256 + b;
    }
    checksumHi = b;
    imgp++;
}

#if 0
void basename(char *s) {
    char *p = NULL;
    while (*s) {
	if (*s == '.') {
	    p = s;
	}
	s++;
    }
    if (p) {
	*p = '\0';
    }
}
#endif

const char *spaces(int n) {
    static const char *sp =
	"                                                                             ";
    return sp + strlen(sp) -22-12*n;
}

void OvlInsertBootImage(const char *fileName, unsigned short mask) {
    FILE *fi;
    unsigned char b0,b1,b2,b3,opcode, recData[65536];
    int recSize, recAddr, endAddr;
    int i;

    if (imgType == vs1000_nand) {
    } else if (imgType == vs1000_spi || imgType == vs10xx_spi) {
    } else {
	Note(1, "Unknown output image type %d. Using vs1000_nand.", imgType);
    }

    Note(0, "OvlInsertBootImage: Add image %s with mask %02xh at location %08xh.",
	 fileName, mask, imgp);

    ovlChecksum = 0;
    fprintf(logfile, "%s:\n", fileName);
    fprintf(logfile, "Add image %s with mask %02xh at location %08xh.\n",
	    fileName, mask, imgp);

    if ((fi = fopen(fileName, "rb"))) {
	b0 = fgetc(fi);
	b1 = fgetc(fi);
	b2 = fgetc(fi);
	if (b0 == 'P' && b1 == '&' && b2 == 'H') {
	    /* VS1053 etc. */
	    if ((mask & MASK_HEADER)) {
		putimg(b0);
		putimg(b1);
		putimg(b2);
	    }
	    do {
		opcode = fgetc(fi);
		b0 = fgetc(fi);
		b1 = fgetc(fi);
		recSize = b0*256+b1;
		if (imgType == vs1000_spi) {
		    recSize = b1*256+b0;
		}
		b0 = fgetc(fi);
		b1 = fgetc(fi);
		recAddr = b0*256+b1;
		if (imgType == vs1000_spi) {
		    recAddr = b1*256+b0;
		}
		Note(0, "  Opcode: %d Size: %d Addr: %d",
		     opcode, recSize, recAddr);
		fread(recData, 1, recSize, fi);
		endAddr = recAddr;
		if (opcode == 0) {
		    endAddr += recSize / 4;
		} else {
		    endAddr += recSize / 2;
		}
		if ((mask & (1<<opcode))) {
		    Note(0, "Inserting record type %d with %d data bytes into image position %d", opcode, recSize, imgp);
		    fprintf(logfile, "%s%04x-%04x\n", spaces(opcode), recAddr, endAddr);
		    fprintf(logfile, "%s%d>%d\n", spaces(opcode), recAddr, endAddr);
		    // Output NAND type bootrecords converted from SPI type bootrecords
		    putimg(opcode);
		    if (imgType == vs1000_spi) {
			putimg(recSize);
			putimg(recSize >> 8);
			putimg(recAddr);
			putimg(recAddr >> 8);
		    } else {
			putimg(recSize>>8);
			putimg(recSize);
			putimg(recAddr>>8);
			putimg(recAddr);
		    }
		    for (i = 0; i < recSize; i++) {
			putimg(recData[i]);
		    }
		} else {
		    //writeln ('Not inserting record type '+inttostr(opcode));
		}
	    } while (3 != opcode && !feof(fi));

	} else {
	    isVS1000 = 1;
	    b3 = fgetc(fi);
	    if (b0 != 'V' || b1 != 'L' || b2 != 'S') {
		Note(2, "%s is not VS1053 or VS1000 boot record format."
		     "%c %c %c %c",
		     fileName, b0,b1,b2,b3);
		exit(10);
	    }
	    if ((mask & MASK_HEADER)) {
#if 0
		Note(0, "Inserting NAND header (fixed parameters for 2K page, 256 Megabytes)\n");
		// insert some kind of NANDBOOT header
		putimg('V');
		putimg('L');
		putimg('S');
		putimg('I');
		putimg(0x00); putimg(0x03); //NandType 0x0003
		putimg(0x08); // Large page (2K)
		putimg(0x13); // 256 Megabytes
		putimg(0x00); putimg(0x46); //Access time 70 ns
		putimg(0x00); putimg(0x0F); //Total boot size 16 blocks (8 kilobytes);
		putimg('B');
		putimg('o');
		putimg('O');
		putimg('t');
#else
		if (imgType == vs1000_nand || imgType == vs1005_nand) {
		    unsigned char head[32];
		    int cnt = 0;

		    head[cnt++] = 'V'; /* exec */
		    head[cnt++] = 'L'; /* exec */
		    head[cnt++] = (imgType == vs1005_nand) ? 'N' : 'S';
		    head[cnt++] = (imgType == vs1005_nand) ? '5' : 'I';
		    head[cnt++] = nandType>>8; /*nandType hi */
		    head[cnt++] = nandType;    /*nandType lo */
		    head[cnt++] = blockSizeBits;
		    head[cnt++] = flashSizeBits;
		    head[cnt++] = nandWaitNs/256;
		    head[cnt++] = nandWaitNs;
		    head[cnt++] = 0x00; /* number of boot sectors hi */
		    head[cnt++] = 0x0f; /* number of boot sectors lo */

		    if (imgType == vs1005_nand) {
			/* word 6: number of extra words before boot record */
			head[cnt++] = 0;
			head[cnt++] = 1;
			/* word 7: if exists (numExtra > 0), the erase block number
			   of the start of mapper area (including bad erase blocks
			   or not including bad erase blocks?) */
			head[cnt++] = 0;
			head[cnt++] = 2;
		    }
		    Note(0, "Inserting NAND header:"
			 "NandType: %d %s %s addr, %dkB blocks, %dMB flash\n",
			 nandType,
			 (nandType & 1) ? "Large-Page" : "Small-Page",
			 (nandType & 2) ?
			 ((nandType & 4) ? "6-byte" : "5-byte") : "4-byte",
			 (1<<blockSizeBits)/2,
			 (1<<flashSizeBits)/2048);

		    head[cnt++] = 'B';
		    head[cnt++] = 'o';
		    head[cnt++] = 'O';
		    head[cnt++] = 't';

		    for (i=0;i<cnt;i++) {
			putimg(head[i]);
		    }
		} else if (imgType == vs1000_spi) {
		    putimg('V');
		    putimg('L');
		    putimg('S');
		    putimg('I');
		}
#endif
	    }
	    do {
		opcode = fgetc(fi);
		b0 = fgetc(fi);
		b1 = fgetc(fi);
		recSize = b0*256+b1;
		if (isVS1000) {
		    recSize = b1*256+b0;
		}
		b0 = fgetc(fi);
		b1 = fgetc(fi);
		recAddr = b0*256+b1;
		if (isVS1000) {
		    recAddr = b1*256+b0;
		}
		Note(0, "  Opcode: %d Size: %d Addr: %d", opcode, recSize, recAddr);
		fread(recData, 1, recSize, fi);
		endAddr = recAddr;
		if (opcode == 0) {
		    endAddr += recSize / 4;
		} else {
		    endAddr += recSize / 2;
		}
		if ((mask & (1<<opcode))) {
		    Note(0, "Inserting record type %d with %d data bytes into image position %d", opcode, recSize, imgp);
		    fprintf(logfile, "%s%04x-%04x\n", spaces(opcode), recAddr, endAddr);
		    fprintf(logfile, "%s%d>%d\n", spaces(opcode), recAddr, endAddr);
		    if (imgType != vs1000_spi) {
			// Output NAND type bootrecords converted from
			// SPI type bootrecords
			putimg(0x80);
			putimg(opcode);
			recSize = recSize / 2;
			putimg((recSize-1)>>8);
			putimg((recSize-1));
			putimg(recAddr>>8);
			putimg(recAddr);

			if (opcode==0) { // I write
			    for (i = 0; i < recSize / 2; i++) {
				putimg(recData[i*4+3]);
				putimg(recData[i*4+2]);
				putimg(recData[i*4+1]);
				putimg(recData[i*4+0]);
			    }
			} else if (opcode==1) { // X write
			    for (i=0;i<recSize;i++) {
				putimg(recData[i*2+1]);
				putimg(recData[i*2+0]);
			    }
			} else if (opcode==2) { // Y write
			    for (i=0;i<recSize;i++) {
				putimg(recData[i*2+1]);
				putimg(recData[i*2+0]);
			    }
			    putimg(0); //Extra word after Y record in NAND
			    putimg(0);
			}

		    } else {
			putimg(opcode);
			putimg(recSize);
			putimg(recSize>>8);
			putimg(recAddr);
			putimg(recAddr>>8);
			recSize = recSize / 2;

			if (opcode==0) { // I write
			    for (i = 0; i < recSize / 2; i++) {
				putimg(recData[i*4+0]);
				putimg(recData[i*4+1]);
				putimg(recData[i*4+2]);
				putimg(recData[i*4+3]);
			    }
			} else if (opcode==1) { // X write
			    for (i=0;i<recSize;i++) {
				putimg(recData[i*2+0]);
				putimg(recData[i*2+1]);
			    }
			} else if (opcode==2) { // Y write
			    for (i=0;i<recSize;i++) {
				putimg(recData[i*2+0]);
				putimg(recData[i*2+1]);
			    }
			}
		    }
		} else {
		    //writeln ('Not inserting record type '+inttostr(opcode));
		}
	    } while (3 != opcode && !feof(fi));
	}
	fclose(fi);
    }
}


void InsertOVL3Image(const char *filename, unsigned short mask) {
    FILE *fi;
    unsigned char b0,b1,b2,b3,opcode;
    int recSize,recAddr,endAddr;
    unsigned char recData[65536];
    int i;
    unsigned short checksum;

    Note(0, "Add V3 overlay code segment %s at location %05xh.",
	 filename, imgp);

    ovlChecksum = 0;
    fprintf(logfile, "%s:\n", filename);

    if (strlen(filename) < 4) {
	Note(2, "filename must be at least 4 characters long.");
	exit(10);
    }

    //V3 Header
    putimg('V');
    putimg('3');
    putimg(filename[0]);
    putimg(filename[1]);
    putimg(filename[2]);
    putimg(filename[3]);

    if ((fi = fopen(filename, "rb"))) {
	isVS1000 = 0; //first let's assume that it's vs1053.
	b0 = fgetc(fi);
	b1 = fgetc(fi);
	b2 = fgetc(fi);
	if (b0!='P' || b1!='&' || b2!='H') { //not P&H
	    isVS1000 = 1;
	    b3 = fgetc(fi);
	    if (b0 != 'V' || b1 != 'L' || b2 != 'S') {
		Note(2, "%s is not in VS1053 or VS1000 boot record format.",
		     filename);
		isVS1000 = 0;
		exit(10);
	    }
	}

	do {
	    opcode = fgetc(fi);
	    b0 = fgetc(fi);
	    b1 = fgetc(fi);
	    recSize = b0*256+b1;
	    if (isVS1000) {
		recSize = b1*256+b0;
	    }
	    b0 = fgetc(fi);
	    b1 = fgetc(fi);
	    recAddr = b0*256+b1;
	    if (isVS1000) {
		recAddr = b1*256+b0;
	    }
	    Note(0, "  Opcode: %d Size: %d Addr: %d",
		 opcode, recSize, recAddr);

	    fread(recData, 1, recSize, fi);
	    // endAddr is only for reporting purposes
	    endAddr = recAddr;
	    if (opcode == 0) {
		endAddr += recSize / 4;
	    } else {
		endAddr += recSize / 2;
	    }
	    if (opcode < 3)
		endAddr--;

	    if ((mask & (1<<opcode)) && opcode == 0 && recSize > 0) {
		fprintf(logfile,"%s%04x-%04x\n",
			spaces(opcode),recAddr, endAddr);

		Note(0, "Inserting V3 record type %d", opcode);
		if ((recSize & 3)) {
		    Note(0, "Warning: I record size not divisible by 4.");
		}
		recSize = recSize / 4;
		// recSize counts now in 32 bit words.
		/* Only I records */
		putimg(recSize >> 8);
		putimg(recSize);
		putimg(recAddr >>8);
		putimg(recAddr);
		for (i=0;i<recSize;i++) {
		    if (isVS1000) {
			putimg(recData[i*4+3]);
			putimg(recData[i*4+2]);
			putimg(recData[i*4+1]);
			putimg(recData[i*4+0]);
		    } else {
			putimg(recData[i*4+0]);
			putimg(recData[i*4+1]);
			putimg(recData[i*4+2]);
			putimg(recData[i*4+3]);
		    }
		}
	    } else if ((mask & (1<<opcode)) && recSize > 0) {
		fprintf(logfile,"%s%04x-%04x\n",
			spaces(opcode), recAddr, endAddr);
		
		Note(2, "No support for V3 record type opcode %d", opcode);
	    } else {
		Note(0, "Not inserting record type %d",opcode);
	    }
	} while (opcode != 3 && !feof(fi));
	fclose(fi);
    } else {
	Note(1, "nonexistent eeprom image file %s", filename);
    }
    /* end marker */
    putimg(0);
    putimg(0);
    //writeln('Checksum1: '+inttostr(ovlChecksum));

    Note(0, "ovlChecksum %04x", ovlChecksum);
    checksum = 65536 - ovlChecksum;
    putimg(checksum >> 8);
    putimg(checksum);
}


// OVL4 image is {"V5" Signature[10] BootFromXRecord(~500 bytes) Aq.purif. ad 512 bytes}* 0000
void EndOVL5Image(void) {
    unsigned short checksum;
    putimg(0);
    putimg(0);
    //writeln('Checksum1: '+inttostr(ovlChecksum));

    Note(0, "ovlChecksum %04x", ovlChecksum);
    checksum = 65536 - ovlChecksum;
    putimg(checksum >> 8);
    putimg(checksum);
}
void InsertOVL5Image(const char *filename, unsigned short mask) {
    FILE *fi;
    unsigned char b0,b1,b2,b3,opcode;
    int recSize,recAddr,endAddr;
    unsigned char recData[65536];
    int i, p, iwordsToWrite;
    int filenameAlreadyPut;

    Note(0, "Add V5 overlay code segment %s at location %05xh.",
	 filename, imgp);

    filenameAlreadyPut = 0;
    fprintf(logfile, "%s:\n", filename);

    if (strlen(filename) < 4) {
	Note(2, "filename must be at least 4 characters long.");
	exit(10);
    }

    if ((fi = fopen(filename, "rb"))) {
	isVS1000 = 0; //first let's assume that it's vs1053.
	b0 = fgetc(fi);
	b1 = fgetc(fi);
	b2 = fgetc(fi);
	if (b0!=80 || b1!=38 || b2!=72) { //not P&O
	    isVS1000 = 1;
	    b3 = fgetc(fi);
	    if (b0 != 'V' || b1 != 'L' || b2 != 'S') {
		Note(2, "%s is not in VS1053 or VS1000 boot record format.",
		     filename);
		isVS1000 = 0;
		exit(10);
	    }
	}

	do {
	    opcode = fgetc(fi);
	    b0 = fgetc(fi);
	    b1 = fgetc(fi);
	    recSize = b0*256+b1;
	    if (isVS1000) {
		recSize = b1*256+b0;
	    }
	    b0 = fgetc(fi);
	    b1 = fgetc(fi);
	    recAddr = b0*256+b1;
	    if (isVS1000) {
		recAddr = b1*256+b0;
	    }
	    Note(0, "  Opcode: %d Size: %d Addr: %d",
		 opcode, recSize, recAddr);

	    fread(recData, 1, recSize, fi);
	    // endAddr is only for reporting purposes
	    endAddr = recAddr;
	    if (opcode == 0) {
		endAddr += recSize / 4;
	    } else {
		endAddr += recSize / 2;
	    }
	    if (opcode < 3)
		endAddr--;

	    if ((mask & (1<<opcode)) && opcode == 0 && recSize > 0) {
		fprintf(logfile,"%s%04x-%04x\n",
			spaces(opcode),recAddr, endAddr);

		Note(0, "Inserting V5 record type %d", opcode);
		if ((recSize & 3)) {
		    Note(0, "Warning: I record size not divisible by 4.");
		}
		recSize = recSize / 4;
		// recSize counts now in 32 bit words.
		p = 0;
		while (recSize > 0) {
		    //V5 Header
		    putimg('V');
		    putimg('5');
		    putimg(0);
		    if (!filenameAlreadyPut) {
			putimg(filename[0]);
		    } else {
			putimg(0);
		    }
		    filenameAlreadyPut = 1;
		    putimg(0);
		    putimg(filename[1]);
		    putimg(0);
		    putimg(filename[2]);
		    putimg(0);
		    putimg(filename[3]);
		    putimg(0);
		    putimg(0);
		    iwordsToWrite = recSize;
		    if (iwordsToWrite > 120) {
			iwordsToWrite = 120;
		    }
		    recSize = recSize - iwordsToWrite;

		    putimg(0x80);
		    putimg(0x00); // I write opcode

		    putimg((iwordsToWrite*2-1) >> 8);
		    putimg((iwordsToWrite*2-1));

		    putimg(recAddr >>8);
		    putimg(recAddr);
		    recAddr = recAddr + iwordsToWrite;
		    for (i = 0; i < iwordsToWrite; i++) {
			putimg(recData[p*4+3]);
			putimg(recData[p*4+2]);
			putimg(recData[p*4+1]);
			putimg(recData[p*4+0]);
			p++;
		    }
		    while ((imgp & 511))
			putimg(0xff);
		    //repeat putimg($ff) until imgp mod 512 = 0;
		}
	    } else if ((mask & (1<<opcode)) && recSize > 0) {
		fprintf(logfile,"%s%04x-%04x\n",
			spaces(opcode), recAddr, endAddr);
		
		Note(0, "Inserting V5 record type %d", opcode);
		if ((recSize & 1)) {
		    Note(0, "Warning: X/Y record size not divisible by 2.");
		}
		recSize = recSize / 2;
		// recSize counts now in 16 bit words.
		p = 0;
		while (recSize > 0) {
		    //V5 Header
		    putimg('V');
		    putimg('5');
		    putimg(0);
		    if (!filenameAlreadyPut) {
			filenameAlreadyPut = 1;
			putimg(filename[0]);
		    } else {
			putimg(0);
		    }
		    putimg(0);
		    putimg(filename[1]);
		    putimg(0);
		    putimg(filename[2]);
		    putimg(0);
		    putimg(filename[3]);
		    putimg(0);
		    putimg(0);
		    iwordsToWrite = recSize;
		    if (iwordsToWrite > 123*2) {
			iwordsToWrite = 123*2;
		    }
		    recSize = recSize - iwordsToWrite;

		    putimg(0x80);
		    putimg(opcode); // X/Y write opcode

		    putimg((iwordsToWrite-1) >> 8);
		    putimg((iwordsToWrite-1));

		    putimg(recAddr >>8);
		    putimg(recAddr);
		    recAddr = recAddr + iwordsToWrite;
		    for (i = 0; i < iwordsToWrite; i++) {
			putimg(recData[p*2+1]);
			putimg(recData[p*2+0]);
			p++;
		    }
		    if (opcode == 2) {
			putimg(0);
			putimg(0);
		    }
		    while ((imgp & 511))
			putimg(0xff);
		}
	    } else {
		Note(0, "Not inserting record type %d",opcode);
	    }
	} while (opcode != 3 && !feof(fi));
	fclose(fi);
    } else {
	Note(1, "nonexistent eeprom image file %s", filename);
    }
}

#define MAX_OVERLAYS 100
#define MAX_OVERLAY_ENTRIES 100
char staticName[100] = "";
int nOverlays;
char overlayName[MAX_OVERLAYS][100];
int overlaySegment[MAX_OVERLAYS];
int nEntries[MAX_OVERLAYS];
char entryName[MAX_OVERLAYS][MAX_OVERLAY_ENTRIES][100];
int entryAddress[MAX_OVERLAYS][MAX_OVERLAY_ENTRIES];
int isUnresolved = 0;
char unresolvedEntry[MAX_OVERLAYS*MAX_OVERLAY_ENTRIES][100];
int unresolvedEntries = 0;
int isInconsistent = 0;
int imageSize = 0;
int addToOverlay = 0;
const char *addToOverlayOverlay[MAX_OVERLAYS];
const char *addToOverlayLinkedObject[MAX_OVERLAYS];
 

void OvlMakeNandImage(const char *param) {
    char tmpName[256];
    int i;
    imgp = 0;
    if (imgType == vs1000_nand || imgType == vs1005_nand) {
	ALIGNMENT = 512;
    } else {
	ALIGNMENT = 16; /* for eeprom image */
    }

    Note(0, "MakeImage(%s)\n", param);
    logfile = fopen("overlays.txt", "wb");
    fprintf(logfile, "MakeOverlaidImage generated report of eeprom image content for %s\n", param);
    fprintf(logfile, "\n");
    fprintf(logfile, "Addresses (in hex):     Code        Initialized Data      Startaddr\n");
    fprintf(logfile, "                          I           X           Y           E\n");
    sprintf(tmpName, "%s.coff.img", staticName);
    OvlInsertBootImage(tmpName, MASK_HEADER+MASK_I+MASK_X+MASK_Y);
    /* Add initialized and const data from overlays to static image */
    for (i=0;i<nOverlays;i++) {
	sprintf(tmpName, "%s.coff.img", overlayName[i]);
	if (flags & FLG_OVLCONSTANTS) {
	    /* Keep initialized and const data in overlays */
	} else {
	    OvlInsertBootImage(tmpName, MASK_X + MASK_Y);
	}
    }
    sprintf(tmpName, "%s.coff.img", staticName);
    OvlInsertBootImage(tmpName, MASK_E);
    Note(-1, "Static image size is %d bytes.", imgp);
    if (imgType == vs1000_nand || imgType == vs1005_nand) {
	if (imgp > 8192) {
	    Note(2, "Static image is too large (max 8 kilobytes).");
	    exit(10);
	}
	while (imgp < 8192) {
	    putimg(0xff); //fill static to 8kB
	}
    }

    //Insert code from overlays
    for (i=0;i<nOverlays;i++) {
	int mask = MASK_I;
	if (flags & FLG_OVLCONSTANTS) {
	    /* Keep initialized and const data in overlays */
	    mask = MASK_I+MASK_X+MASK_Y;
	}
	while ((imgp % ALIGNMENT))
	    putimg(0xff);
	overlaySegment[i] = imgp / ALIGNMENT;
	ovlChecksum = 0;

	sprintf(tmpName, "%s.coff.img", overlayName[i]);
	if (imgType == vs1000_nand) {
	    InsertOVL5Image(tmpName, mask);
	} else {
	    InsertOVL3Image(tmpName, mask);
	}
	{
	    int j;
	    for (j=0;j<addToOverlay;j++) {
		if (!strcmp(addToOverlayOverlay[j], overlayName[i])) {
		    char tmpNameOut[256];
		    enum IMGTYPE oldImgType = imgType;
		    Note(-1, "Adding to overlay %s: %s",
			 addToOverlayOverlay[j],
			 addToOverlayLinkedObject[j]);

		    imgType = vs1000_spi;
		    sprintf(tmpNameOut, "%s.img", addToOverlayLinkedObject[j]);

		    blockNum = 0;
		    SendFlashData(1/*putHeader*/, 0x50, addToOverlayLinkedObject[j], tmpNameOut);
		    imgType = oldImgType;
		    while ((imgp % ALIGNMENT))
			putimg(0xff);
		    if (imgType == vs1000_nand) {
			InsertOVL5Image(tmpNameOut, MASK_I|MASK_X|MASK_Y);
		    } else {
			InsertOVL3Image(tmpName, mask);
		    }
		}
	    }
	}
	if (imgType == vs1000_nand) {
	    EndOVL5Image();
	} else {
	}
    }
    Note(-1, "Output image size is %d bytes.", imgp);
    imageSize = imgp;
}

void OvlWriteOutputImage(const char *param) {
    FILE *fp;
    Note(0, "Writing output image %s.", param);
    if ((fp = fopen(param, "wb"))) {
	fwrite(img, 1, imgp, fp);
	fclose(fp);
    } else {
	Note(2, "Could not open %s for writing!", param);
    }
}

void OvlReadProjectFile(const char *fileName) {
    int i, nSubStrings;
    char s[256];
    char tmpName[256+10];
    FILE *f;

    nOverlays = 0;
    isUnresolved = 0;
    unresolvedEntries = 0;
    memset(nEntries, 0, sizeof(nEntries));
//ovlmain
//ovl1 OverlayInitLcd OverlayInitMlc OverlayInitPlayTime OverlayInitPowerOff OverlayInitUSB
//ovl2 OverlayIdleHook OverlayPlayCurrent
//ovl3 OverlayPlayLoop

    if ((f = fopen(fileName, "rb"))) {
	while (fgets(s, 256-1, f)) {
	    char *t, *t2;
//	    printf("line: %s", s);
	    nSubStrings = 0;
	    nEntries[nOverlays] = 0;
	    t = s;
	    while (t &&
		   ((t2 = strstr(t, " ")) ||
		    (t2 = strstr(t, ",")) ||
		    (t2 = strstr(t, "\n"))) && t2 != t) {
		if (nSubStrings == 0) {
		    memcpy(overlayName[nOverlays], t, t2-t);
		    overlayName[nOverlays][t2-t] = '\0';
//printf("Overlay name '%s'\n", overlayName[nOverlays]);
		} else {
		    memcpy(entryName[nOverlays][nEntries[nOverlays]], t, t2-t);
		    entryName[nOverlays][nEntries[nOverlays]][t2-t] = '\0';
//printf("entry name '%s'\n", entryName[nOverlays][nEntries[nOverlays]]);
		    nEntries[nOverlays]++;
		}
		t = t2+1;
		nSubStrings++;
	    }
//	    printf("subStrings: %d\n", nSubStrings);
	    if (nSubStrings == 1 && staticName[0] == 0) {
		strcpy(staticName, overlayName[nOverlays]);
	    }
	    if (nSubStrings > 1) {
		struct COFF *coff = COFF_Create(0);
		if (!coff) {
		    Note(2, "Could not create COFF handle");
		    exit(10);
		}
		sprintf(tmpName, "%s.coff", overlayName[nOverlays]);
		if (!COFF_Read(coff, tmpName, NULL)) {
		    printf("%s: %s\n", tmpName, COFF_Error(coff));
		    COFF_Delete(coff);
		    goto bail;
		} else {
		    Note(0, "Read COFF File %s: %s",
			 tmpName, COFF_Error(coff));
		}
		for (i=0;i<nEntries[nOverlays];i++) {
		    int sectNum;
		    struct FILEHDR *header;
		    struct SECTIONHDR *sHdr;
		    //struct SYMENT *symTmp;
		    int physicalAddress = 0;
		    /*TODO: find symbols and not sections,
		      but our tools generate each function to a
		      section with the same name. */
		    header = COFF_GetHeader(coff);
		    for (sectNum=1; sectNum<=header->f_nscns; sectNum++) {
			sHdr = COFF_GetSection(coff, sectNum);
			if (!sHdr) {
			}
			if (!strcmp(COFF_GetString(coff, &sHdr->_s_name),
				    entryName[nOverlays][i])) {
			    physicalAddress = sHdr->s_paddr;
			    break;
			} else {
			}
		    }
		    if (sectNum > header->f_nscns) {
			/* not found */
			isUnresolved = 1;
			strcpy(unresolvedEntry[unresolvedEntries++],
			       entryName[nOverlays][i]);
			physicalAddress = 0;
		    }
		    entryAddress[nOverlays][i] = physicalAddress;
		}
		COFF_Delete(coff);
	    bail:
		nOverlays++;
	    }
	}
    }
}



/*
  Changes the symbol value and all the relocatable references to it.
 */
int RelocateSymbol(struct COFF *coff, struct SYMENT *symbol, int newValue) {
    int sectNum, relocNum;
    struct FILEHDR *header;
    struct SECTIONHDR *sectHeader;
    struct RELOC *relocs;
    void *sectData;

    header = COFF_GetHeader(coff);
    for (sectNum=1; sectNum<=header->f_nscns; sectNum++) {
	sectHeader = COFF_GetSection(coff, sectNum);
	if (!sectHeader) {
	    Note(2, "RelocateSymbol: Section not found");
	    return -1;
	}
	/* If no relocations, skip to next section */
	if (!sectHeader->s_nreloc)
	    continue;

	relocs = COFF_GetSectionReloc(coff, sectNum);
	for (relocNum=1; relocNum<=sectHeader->s_nreloc; relocNum++) {
	    struct SYMENT *relocSym = NULL;

	    /* If the reloc points to symbol */
	    if (relocs[relocNum-1].r_symndx > 0 &&
		(relocSym = COFF_GetSymbol(coff, relocs[relocNum-1].r_symndx))
		 && relocSym == symbol) {
		LL realVal = 0;
		/* correct the data */
		sectData = COFF_GetSectionData(coff, sectNum);

		Note(0, "RelocateSymbol: reloc %d(%s), cur 0x%08x",
		     relocNum,
		     COFF_GetString(coff, &relocSym->_n_name),
		     relocs[relocNum-1].r_curval);

		relocs[relocNum-1].r_curval = newValue;
		switch (R_TYPE(relocs[relocNum-1].r_vaddr)) {
		case R_32HI:
		    realVal = (newValue >> dataword);
		    break;
		case R_32LO:
		    realVal = (newValue & WORDMASK(dataword));
		    break;
		case R_16BIT:
		    realVal = newValue & WORDMASK(dataword);
		    break;
		default:
		    Note(2, "RelocateSection: unknown relocation type %ld",
			 R_TYPE(relocs[relocNum-1].r_vaddr));
		    return -1;
		}
/*fprintf(stderr, "RelocateSection reloc value %08lx, new %08lx\n",
	relocs[relocNum-1].r_curval, newVal);*/
		if ((sectHeader->s_flags & STYP_TEXT)?
		    SetRelocation(((ULL *)sectData) +
				  R_VADDR(relocs[relocNum-1].r_vaddr),
				  realVal) :
		    SetDataRelocation(((ULL *)sectData) +
				      R_VADDR(relocs[relocNum-1].r_vaddr),
				      realVal)) {
		    char temp[100];
		    struct RELOC *reloc = &relocs[relocNum-1];

		    if ((sectHeader->s_flags & STYP_TEXT)) {
			UnAsm(((ULL*)sectData)
			      [R_VADDR(relocs[relocNum-1].r_vaddr)],
			      temp, reloc?reloc->r_symndx:0,
			      reloc?reloc->r_curval:0, reloc?NULL:NULL);
		    } else {
			UnAsmData(((ULL*)sectData)
				  [R_VADDR(relocs[relocNum-1].r_vaddr)],
				  temp, reloc?reloc->r_symndx:0,
				  reloc?reloc->r_curval:0, reloc?NULL:NULL);
		    }
		    Note(2, "RelocateSymbol: relocation: %s", WhyFailed());
		    Note(1, "c%04lx n%04lx sect %d@0x%04lx %d %s s%d %s",
			 relocs[relocNum-1].r_curval, newValue,
			 sectNum,
			 R_VADDR(relocs[relocNum-1].r_vaddr),
			 R_TYPE(relocs[relocNum-1].r_vaddr),
			 COFF_GetString(coff, &sectHeader->_s_name),
			 relocs[relocNum-1].r_symndx,
			 temp);
		}
	    }
	}
    }
    Note(0, "RelocateSymbol: sym %s, 0x%08lx -> 0x%08lx",
	 COFF_GetString(coff, &symbol->_n_name),
	 symbol->n_value,
	 newValue);
    symbol->n_value = newValue;
    return 0;
}








//int COFF_FindSymbol(struct COFF *coff, const char *name);
int OvlCheckConsistency(const char *fileName) {
    int i;
    char tmpName[256];
    int o, e, redo = 0;
//    FILE *f;

    /*
      Note: Must check and fix the static part also!
    */

    isInconsistent = 0;
    for (i=-1;i<nOverlays;i++) {
	char *name;
	struct COFF *coff = COFF_Create(0);
	int modified = 0;
	if (!coff) {
	    Note(2, "Could not create COFF handle");
	    exit(10);
	}
	if (i < 0) {
	    name = staticName;
	} else {
	    name = overlayName[i];
	}
	sprintf(tmpName, "%s.coff", name);
	if (!COFF_Read(coff, tmpName, NULL)) {
	    Note(1, "%s: %s\n", tmpName, COFF_Error(coff));
	    isInconsistent = 1;
	    goto bail;
	}
	/* check symbols */
	for (o=0; o<nOverlays; o++) {
	    int s;
	    struct SYMENT *sym;
	    sprintf(tmpName, "_%s_seg", overlayName[o]);
	    s = COFF_FindSymbol(coff, tmpName);
	    if (s) {
		sym = COFF_GetSymbol(coff, s);
		if (sym->n_value != overlaySegment[o]
		    && !isInconsistent) {
		    Note(0, "_%s_seg in %s is %d, should be %d, fixing", overlayName[o], name, sym->n_value, overlaySegment[o]);

		    if (RelocateSymbol(coff, sym, overlaySegment[o])) {
			/* Could not relocate the symbol. */
			isInconsistent = 1;
		    } else {
			modified = 1;
		    }
		}
	    } else {
		/* no symbol -- no need to check */
	    }
	}
	/* check entry points (offsets) */
	for (o=0; o < nOverlays; o++) {
	    for (e=0;e<nEntries[o];e++) {
		int s;
		struct SYMENT *sym;
		sprintf(tmpName, "_%s_ofs", entryName[o][e]);
		s = COFF_FindSymbol(coff, tmpName);
		if (s) {
//		    printf("Found symbol %s\n", tmpName);
		    sym = COFF_GetSymbol(coff, s);
		    if (entryAddress[o][e] && entryAddress[o][e] != sym->n_value &&
			!isInconsistent) {
			Note(0, "_%s_ofs in %s is %d, should be %d, fixing",
			       entryName[o][e], name, sym->n_value, entryAddress[o][e]);
			if (RelocateSymbol(coff, sym, entryAddress[o][e])) {
			    /* Could not relocate the symbol. */
			    isInconsistent = 1;
			} else {
			    modified = 1;
			}
		    }
		} else {
//		    printf("Did not find symbol %s\n", tmpName);
		}
	    }
	}
    bail:
	if (modified) {
	    char tmpNameOut[256];
	    redo = 1;
	    Note(-1, "Writing modified %s.coff to disk", name);
	    sprintf(tmpName, "%s.coff", name);
	    if (!COFF_Write(coff, tmpName, NULL)) {
		Note(2, "%s", COFF_Error(coff));
	    } else {
		enum IMGTYPE oldImgType = imgType;

		imgType = vs1000_spi;
		Note(-1, "Recreating %s.coff.img (vs1000_spi)", name);
		sprintf(tmpNameOut, "%s.coff.img", name);

		blockNum = 0;
		SendFlashData(1/*putHeader*/, 0x50, tmpName, tmpNameOut);
		imgType = oldImgType;
	    }
	}
	COFF_Delete(coff);
    }
    return redo;
}


void OvlWriteHeaderFile(const char *fileName) {
    FILE *f;
    int o, e;

    Note(-1, "Writing header file %s", fileName);
    if ((f = fopen(fileName, "wb"))) {
	fprintf(f,"// AUTOMATICALLY GENERATED List of Overlay Entries.\n");
	fprintf(f,"// Do not edit.\n");
	fprintf(f,"\n");
	fprintf(f,"#ifndef OVL_ENTRY_H\n");
	fprintf(f,"#define OVL_ENTRY_H\n");
	fprintf(f,"\n");
	fprintf(f,"// All addresses are defined as array start addresses so the linker can relocate them.\n");
	for (o=0;o<nOverlays;o++) {
	    fprintf(f,"extern u_int16 %s_seg[];\n", overlayName[o]);
	    for (e=0;e<nEntries[o];e++) {
		fprintf(f,"extern u_int16 %s_ofs[];\n", entryName[o][e]);
		//#define SystemInitOv(a)(CallOverlay((u_int16)init_seg,(u_int16)SystemInit_ofs,((u_int16)a)))
		fprintf(f,"#define %sOv(a)(CallOverlay((u_int16)%s_seg,(u_int16)%s_ofs,((u_int16)a)))\n",
			entryName[o][e],
			overlayName[o],
			entryName[o][e]);
	    }
	}
	fprintf(f,"\n");
	fprintf(f,"#endif\n");
	fclose(f);
    } else {
	Note(1, "Could not open %s for writing\n", fileName);
    }
}

void OvlWriteSymbolFile(const char *fileName) {
    FILE *f;
    int o, e;
    Note(-1, "Writing symbol file %s", fileName);
    if ((f = fopen(fileName, "wb"))) {
	for(o=0;o<nOverlays;o++) {
	    fprintf(f,"_%s_seg=%d\n", overlayName[o], overlaySegment[o]);
//	    fprintf(f,"_%s_seg=0\n", overlayName[o]);
	    for (e=0;e<nEntries[o];e++) {
		fprintf(f,"_%s_ofs=%d\n", entryName[o][e], entryAddress[o][e]);
//		fprintf(f,"_%s_ofs=0\n", entryName[o][e]);
	    }
	}
	fclose(f);
    } else {
	Note(1, "Could not open %s for writing", fileName);
    }
}


#if 0
Procedure WriteFile(fileName:string);
var f:textfile; s:string; I:Integer;
begin
  assignfile(f,fileName);
  rewrite(f);
  writeln (f,'');

  closefile(f);
end;


Procedure Nag;
begin
  if isUnresolved then begin
    writeln ('Warning: At this linker pass, the project has unresolved entry points.');
    writeln ('Unresolved entries are: '+unresolvedEntries);
  end;
  if isInconsistent then begin
    writeln ('Warning: At this linker pass, the image is inconsistent. Address info is now updated for next pass.');
  end;
  if isInconsistent or isUnresolved then begin
    writeln ('The final image cannot be written at this linker pass.');
    writeln ('Warning: A rebuild of the solution is required.');
  end;
end;
#endif
















int freelist_page[1000], freelist_address[1000], freelist_size[1000], nFreelist;

void OvlReadFreeList(const char *fileName) {
    FILE *f;
    char s[256];
    Note(0, "Reading free mem list (\"####\"-records) from %s", fileName);
    if ((f = fopen(fileName,"rb"))) {
	nFreelist = 0;
	while (fgets(s, 255, f)) {
	    // get info from "#### Free 2 4096 2048 ####" lines
//printf("line: %s\n", s);
	    if (!strncmp(s, "#### ", 5)) {
		char *p;
		freelist_page[nFreelist] = strtol(s + 9, &p, 10);
		freelist_address[nFreelist] = strtol(p, &p, 10);
		freelist_size[nFreelist] = strtol(p, &p, 10);
//printf("Got! %d %d %d\n", freelist_page[nFreelist], freelist_address[nFreelist], freelist_size[nFreelist]);
		nFreelist++;
	    }
	}
    } else {
	Note(1, "Could not open %s for reading", fileName);
    }
}


void OvlMemremainStatic(const char *fileName, const char *ovl_code_mem) {
    FILE *f;
    int i, iAddr, iSize;

    // Read in the memory description
    OvlReadFreeList(fileName);
    // Find largest code space chunk
    iSize = 0;
    iAddr = 0;
    for (i=0; i<nFreelist; i++) {
	if (freelist_page[i] == 0 && freelist_size[i] > iSize) {
	    iAddr = freelist_address[i];
	    iSize = freelist_size[i];
	}
    }
    Note(0, "Write out the address (%d) and size (%d) of the largest code memory chunk.", iAddr, iSize);
    Note(0, "this will be used as the only code memory available to overlays.");
    if ((f = fopen(ovl_code_mem, "wb"))) { //ovl_code_mem
	fprintf(f, "#### Free 0 %d %d ####\n", iAddr, iSize);
	
	fclose(f);
    }
}

const char memspaces[] = "IXY";

void OvlMemremainOverlay(const char *fileName, const char *ovl_code_mem, const char *outName) {
    FILE *f;
    int i, iAddr, iSize;

    /* Read available code memory that can be used by overlays. */
    Note(0, "Get the address and length of the code chunk set for overlays from file %s.", ovl_code_mem);
    OvlReadFreeList(ovl_code_mem);
    if (nFreelist != 1) {
	Note(2, "cannot read ovl code memory chunk info from %s",
	     ovl_code_mem);
	exit(10);
    }
    /* Remember it. */
    iAddr = freelist_address[0];
    iSize = freelist_size[0];

    /* Read what was left from the last linking stage. */
    Note(0, "Read the free memory list from previous link pass (to get info of available X and Y mem)");
    OvlReadFreeList(fileName);

    Note(0, "Write out a new memory description");
    if ((f = fopen(outName, "wb"))) {
	fprintf(f,"MEMORY\n");
	fprintf(f,"{\n");
	fprintf(f,"  page 0:\n");
	fprintf(f,"    code_overlay: origin = %d, length = %d\n",
		iAddr, iSize);
	/* Then write what's left of the data areas. */
	for (i=0;i<nFreelist;i++) {
	    if (freelist_page[i] != 0) {
		fprintf(f,"  page %d:\n", freelist_page[i]);
		fprintf(f,"    %c_%d: origin = %d, length = %d\n",
			memspaces[freelist_page[i]],
			freelist_address[i],
                        freelist_address[i],
                        freelist_size[i]);
	    }
	}
	fprintf(f,"}\n");
	fclose(f);
    }
}

/*
  makenandoverlay operation:

  1) generate overlays.abs and overlays.h from overlay_control file
  +) compile and link static part
  2) memremain_static:
     create mem_overlay and ovl_code_mem from mem_desc.available
  +) compile and link each overlay (with staticsymbols.o)
  3) memremain_overlay:
     create mem_overlay from mem_desc.available and ovl_code_mem
  4) create overlay image file, use overlay_control to determine binaries
     checks that overlay offsets and entrypoints match


  TODO: integrate with coff2allboot, then we don't need to generate
 */

/***************************************************************************/


void AddData(int page, int s_paddr, int val) {
    int ss = 0;
    char *data=NULL;
    if (flags & FLG_NOMERGE) {
	ss = blockNum;
    }
    for (ss = 0; ss < blockNum; ss++) {
	if (page == block[ss].page &&
	    s_paddr == block[ss].addr + block[ss].size
	    /*limit the size 'cause compressed records can be 32kB uncompr.*/
	    && block[ss].size + 1 < 0x2000) {
	    /* Add to the tail */
	    unsigned char *tmp = realloc(block[ss].data,
					 block[ss].len +
					 ((page == 0)?4:2));
	    if (tmp) {
		block[ss].data = tmp;
		data = tmp + block[ss].len; /* point to tail */
		block[ss].size += 1; /* size in words */
		block[ss].len += ((page == 0) ? 4 : 2);
		ss = -1;
		break;
	    }
	}
	if (page == block[ss].page && s_paddr + 1 == block[ss].addr
	    /*limit the size 'cause compressed records can be 32kB uncompr.*/
	    && block[ss].size + 1 < 0x2000) {
	    /* Add to the head -- although quite rare, mainly
	       data memory (init/const) */
	    unsigned char *tmp = malloc(block[ss].len +
					((page == 0)? 4 : 2));
	    if (tmp) {
		/* copy old data to the tail */
		memcpy(tmp + ((page == 0)?4:2),
		       block[ss].data, block[ss].len);
		free(block[ss].data);
		block[ss].data = tmp;
		data = tmp; /* point to head -- new data here */
		block[ss].size += 1; /* size in words */
		block[ss].len += ((page == 0) ? 4 : 2);
		block[ss].addr = s_paddr;
		ss = -1;
		break;
	    }
	} else {
#if 0
	    printf("page %d block[ss].page %d, %04x+%04x=%04x %04x\n",
		   page, block[ss].page,
		   s_paddr, s_size, s_paddr+s_size, block[ss].addr);
#endif
	}
    }
    if (ss != -1) {
	/* create new header */
	block[blockNum].page = page;
	block[blockNum].addr = s_paddr; /* physical addr! */
	block[blockNum].size = 1;
	block[blockNum].len = ((page == 0) ? 4 : 2);
	data = calloc(1, (page == 0) ? 4 : 2);
	block[blockNum].data = data;
	blockNum++;
    }
    if (page == 0) {
	if (imgType == vs1000_spi) {
	    *data++ = val>>0;
	    *data++ = val>>8;
	    *data++ = val>>16;
	    *data++ = val>>24;
	} else {
	    *data++ = val>>24;
	    *data++ = val>>16;
	    *data++ = val>>8;
	    *data++ = val>>0;
	}
    } else {
	if (imgType == vs1000_spi) {
	    *data++ = val>>0;
	    *data++ = val>>8;
	} else {
	    *data++ = val>>8;
	    *data++ = val>>0;
	}
    }
}



#define MAXFILES 256
int main(int argc, char *argv[]) {
    int n;
    int putHeader = 1;
    char *file[MAXFILES], *outFile = NULL;
    int files = 0;

    blockNum = 0;

    for (n=1; n<argc; n++) {
	if (!strcasecmp("memremain_static", argv[n])) {
	    flags |= FLG_MEMSTATIC | FLG_OVERLAY;
	} else if (!strcasecmp("memremain_overlay", argv[n])) {
	    flags |= FLG_MEMOVERLAY | FLG_OVERLAY;
	} else if (!strcmp(argv[n], "--add-to-overlay")) {
	    if (n+2 < argc) {
		if (addToOverlay < MAX_OVERLAYS) {
		    addToOverlayOverlay[addToOverlay] = argv[n+1];
		    addToOverlayLinkedObject[addToOverlay] = argv[n+2];
		    addToOverlay++;
		} else {
		    fprintf(stderr, "%s: too many --add-to-overlays\n",
			    argv[0]);
		    flags |= FLG_ERROR;
		}
		n+=2;
	    } else {
		flags |= FLG_ERROR;
		break;
	    }
	} else if (!strcmp(argv[n], "--no-compression")) {
	    flags |= FLG_NOCOMPRESSION;
	} else if (!strcmp(argv[n], "--no-merge")) {
	    flags |= FLG_NOMERGE;
	} else if (!strcmp(argv[n], "--ignore-size")) {
	    flags |= FLG_IGNORESIZE;
	} else if (argv[n][0]=='-') {
	    int i = 1, ch;
	    char *val, *tmp;
	    long tmpval;

	    while (argv[n][i]) {
		switch (ch = argv[n][i]) {
		case 'h':
		case '?':
		    flags |= FLG_ERROR;
		    break;
		case 'n':
		    putHeader = 0;
		    break;

		case 'O':
		    flags |= FLG_OVERLAY;
		    break;
		case 'C':
		    flags |= FLG_OVLCONSTANTS;
		    break;
		case 'v':
		    flags |= FLG_VERBOSE;
		    break;

		case 'o':
		    if (outFile) {
			fprintf(stderr, "%s: too many output files\n",
				argv[0]);
			flags |= FLG_ERROR;
			break;
		    }
		    if (argv[n][i+1]) {
			outFile = argv[n]+i+1;
		    } else if (n+1 < argc) {
			outFile = argv[n+1];
			n++;
		    } else {
			flags |= FLG_ERROR;
			break;
		    }
		    i = strlen(argv[n])-1;
		    break;

#ifdef WITH_CRYPT
		case 'k':
		    if (argv[n][i+1]) {
			val = argv[n]+i+1;
			i = strlen(argv[n])-1;
		    } else if (n+1 < argc) {
			val = argv[++n];
			i = strlen(argv[n])-1;
		    } else {
			flags |= FLG_ERROR;
			val = "";
		    }
		    if (flags & FLG_ERROR) {
			/* */
		    } else {
			unsigned char *p = val;
			memset(rc4key, 0, sizeof(rc4key));
			/* default mask */
			memcpy(rc4key+8,
			       "\xff\xff\xff\xff\xff\xff\xff\xff", 8);
			rc4keylen = 0;
			while (*p && *p != ',')
			    p++;
			rc4keylen = ParseKeyOrMask(rc4key, p);
			if (rc4keylen != 8) {
			    flags |= FLG_ERROR;
			    fprintf(stderr, "wrong key size: 8 bytes required\n");
			}
			if (*p == ',') {
			    while (*++p)
				;
			    /* Custom mask */
			    ParseKeyOrMask(rc4key+8, p);
			}
			flags |= FLG_CRYPTED;
#if 1
			{
			    int i;
			    for (i=0;i<rc4keylen;i++) {
				printf("%02x ", rc4key[i]);
			    }
			    printf("\n");
			    for (i=0;i<8;i++) {
				printf("%02x ", rc4key[8+i]);
			    }
			    printf("\n");
			}
#endif
		    }
		    break;
#endif/*WITH_CRYPT*/

		case 'i':
		    if (argv[n][i+1]) {
			val = argv[n]+i+1;
			i = strlen(argv[n])-1;
		    } else if (n+1 < argc) {
			val = argv[++n];
			i = strlen(argv[n])-1;
		    } else {
			flags |= FLG_ERROR;
			val = "";
		    }
		    if (flags & FLG_ERROR) {
		    } else if (!strcasecmp(val, "vs1000nand")) {
			if (execAddr == 0)
			    execAddr = 0x50;
			putHeader = 1;
			imgType = vs1000_nand;
		    } else if (!strcasecmp(val, "vs1000ramdisk")) {
			if (execAddr == 0)
			    execAddr = 0x50;
			putHeader = 0;
			imgType = vs1000_ramdisk;
		    } else if (!strcasecmp(val, "vs1000raw")) {
			if (execAddr == 0)
			    execAddr = 0x50;
			putHeader = 0;
			imgType = vs1000_raw;
		    } else if (!strcasecmp(val, "vs1000spi")) {
			if (execAddr == 0)
			    execAddr = 0x50;
			putHeader = 1;
			imgType = vs1000_spi;
		    } else if (!strcasecmp(val, "vs1005nand")) {
			if (execAddr == 0)
			    execAddr = 0x80;
			putHeader = 1;
			imgType = vs1005_nand;
		    } else if (!strcasecmp(val, "vs1005raw")) {
			if (execAddr == 0)
			    execAddr = 0x80;
			putHeader = 0;
			imgType = vs1005_raw;
		    } else if (!strcasecmp(val, "vs1005fspi")) {
			if (execAddr == 0)
			    execAddr = 0x80;
			putHeader = 1;
			imgType = vs1005f_spi;
		    } else if (!strcasecmp(val, "vs1005spi")) {
			if (execAddr == 0)
			    execAddr = 0x80;
			putHeader = 1;
			imgType = vs1005_spi;
		    } else if (!strcasecmp(val, "vs1005gsd")) {
			if (execAddr == 0)
			    execAddr = 0x80;
			putHeader = 0;
			imgType = vs1005g_sd;
		    } else if (!strcasecmp(val, "vs10xx") ||
			       !strcasecmp(val, "vs10xxspi")) {
			if (execAddr == 0)
			    execAddr = 0x50;
			putHeader = 1;
			imgType = vs10xx_spi;
		    } else if (!strcasecmp(val, "plugin")) {
			imgType = vs10xx_plugin;
		    } else if (!strcasecmp(val, "cmd")) {
			imgType = vs10xx_cmd;
		    } else {
			fprintf(stderr,
				"Error: unknown image type: \"%s\"\n", val);
			flags |= FLG_ERROR;
		    }
		    break;

		case 'E':
		case 'x':
		case 'w':
		case 't':
		case 'b':
		case 's':
		case 'c':
		    if (argv[n][i+1]) {
			val = argv[n]+i+1;
		    } else if (n+1 < argc) {
			val = argv[++n];
		    } else {
			flags |= FLG_ERROR;
			break;
		    }
		    i = strlen(argv[n])-1;
		    //tmpval = strtol(val, &tmp, 0);
		    tmpval = strtoul(val, &tmp, 0);
		    if (*tmp) {
			fprintf(stderr,
				"Error: invalid number: \"%s\"\n", val);
			flags |= FLG_ERROR;
			break;
		    }
		    if (ch == 'c') {
			if (tmpval == 0) {
			    /* 16M small-page flash */
			    nandWaitNs = 80;
			    nandType = 0;
			    blockSizeBits = 5;
			    flashSizeBits = 15;
			} else if (tmpval == 1) {
			    /* 128M small-page flash */
			    nandWaitNs = 100;
			    nandType = 2;
			    blockSizeBits = 5;
			    flashSizeBits = 18;
			} else if (tmpval == 2) {
			    /* 256M large-page flash */
			    nandWaitNs = 100;
			    nandType = 3;
			    blockSizeBits = 8;
			    flashSizeBits = 19;
			} else {
			    fprintf(stderr,
				    "Error: unknown chip: \"%s\"\n", val);
			    flags |= FLG_ERROR;
			}
		    } else if (ch == 'x') {
			execAddr = tmpval;
		    } else if (ch == 'w') {
			nandWaitNs = tmpval;
		    } else if (ch == 't') {
			nandType = tmpval;
		    } else if (ch == 'b') {
			blockSizeBits = tmpval;
		    } else if (ch == 's') {
			flashSizeBits = tmpval;
		    } else if (ch == 'E') {
			/* For vs10xx only. No decryption in vsomd! */
			if (tmpval == 0) {
			    flags |= FLG_ERROR;
			}
			encryptionKey = tmpval ^ 0xaaaaaaaa;
		    }
		    break;

		case 'd':
		{
		    char *t;
		    unsigned long addr=0, val=0;
		    int page = -1;
		    if (argv[n][i+1]) {
			t = &argv[n][i+1];
		    } else if (n < argc) {
			t = &argv[++n][0];
		    } else {
			flags |= FLG_ERROR;
			break;
		    }
		    /*
		      x:0xc01a=0,0xc005=0x1234,y:1234=2345,i:0x10=0x20000014
		     */
		    while (t && *t) {
			if (tolower(*t) == 'x') {
			    page = 1;
			    t++;
			} else if (tolower(*t) == 'y') {
			    page = 2;
			    t++;
			} else if (tolower(*t) == 'i') {
			    page = 0;
			    t++;
			} else {
			}
			if (*t == ':') {
			    t++;
			}
			val = strtol(t, &t, 0);
			if (t && *t == '=') {
			    addr = val;
			    val = strtol(t+1, &t, 0);
			} else {
			    addr++;
			}
			AddData(page, addr, val);
			if (t && *t == ',') {
			    t++;
			    continue;
			}
		    }
		    i=strlen(argv[n])-1;
		    break;
		}

		default:
		    fprintf(stderr, "Error: Unknown option \"%c\"\n",
			    argv[n][i]);
		    flags |= FLG_ERROR;
		}
		i++;
	    }
	} else {
	    if (files < MAXFILES) {
		file[files++] = argv[n];
		if (!strcasecmp("overlay_control", argv[n])) {
		    flags |= FLG_OVERLAY;
		}
	    } else {
		fprintf(stderr, "Too many files (%d) specfied!\n", files);
		flags |= FLG_ERROR;
	    }
	}
    }
    if ((flags & FLG_VERBOSE))
	fprintf(stderr, "%s: I'm verbose.\n", argv[0]);

    /* Overlay making detected... */
    if ((flags & FLG_MEMSTATIC)) {
	if (files == 2) {
	    OvlMemremainStatic(file[0], "ovl_code_mem"); // put aside info of largest code memory segment
	    OvlMemremainOverlay(file[0], "ovl_code_mem", file[1]); // write memory desctiption for overlay
	    return 0;
	}
	flags |= FLG_ERROR;
    } else if ((flags & FLG_MEMOVERLAY)) {
	if (files == 2) {
	    OvlMemremainOverlay(file[0], "ovl_code_mem", file[1]); // write memory desctiption for overlay
	    return 0;
	}
	flags |= FLG_ERROR;
    } else if ((flags & FLG_OVERLAY)) {

	if (files == 2) {
	    img = malloc(MAXIMGSIZE);
	    Note(0, "Opening overlay control file %s", file[1]);
	    OvlReadProjectFile(file[1]);

	    OvlMakeNandImage(file[0]);
	    Note(0, "Checking that image is consistent with all module object files.");
	    if (OvlCheckConsistency(file[1])) {
		/* Redo image file with fixed coff files! */
		OvlMakeNandImage(file[0]);
		if (OvlCheckConsistency(file[1])) {
		    /* Redo image file with fixed coff files! */
		    OvlMakeNandImage(file[0]);
		    if (OvlCheckConsistency(file[1])) {
			isInconsistent = 1;
		    }
		}
	    }
	    OvlWriteHeaderFile("overlays.h");
	    OvlWriteSymbolFile("overlays.abs");

	    if (isUnresolved) {
		int i;
		Note(1,"At this linker pass, the project has unresolved entry points.");
		printf("Unresolved entries are: ");
		for (i=0;i<unresolvedEntries;i++) {
		    printf(" %s", unresolvedEntry[i]);
		}
		printf("\n");
	    }
	    if (isInconsistent) {
		Note(1, "At this linker pass, the image is inconsistent. Address info is now updated for next pass.");
	    }
	    if (isInconsistent || isUnresolved) {
		Note(1, "The final image cannot be written at this linker pass.");
		Note(1, "Warning: A rebuild of the solution is required.");
	    }
	    if (!isUnresolved && !isInconsistent) {
		Note(-1, "No unresolved entries, writing image %s.", file[0]);
		OvlWriteOutputImage(file[0]);
		free(img);
		return 0;
	    }
	    free(img);
	    return 0; /*must return ok for make to continue*/
	} else {
	    Note(2, "Two files expected, got %d", files);
	}
	flags |= FLG_ERROR;
    } else {

	if (!outFile && files) {
	    outFile = file[--files];
	}
	if (!outFile || files != 1) {
	    fprintf(stderr, "Exactly two filenames wanted!\n");
	    flags |= FLG_ERROR;
	}
    }


    if ((flags & FLG_ERROR)) {
	fprintf(stderr, "Usage: %s [-i <type>] [-w <waitns>] [-t <type>] [-b <block>] [-s <size>] [-x <exec>] [-c <chip>] [-o <outfile>] <infile> <outfile>\n",
		argv[0]);
	fprintf(stderr, " -o <outfile>  set output file\n");
	fprintf(stderr, " -x<val>       set execution address\n");
	fprintf(stderr, " -d<assign>    add data, m:addr=val(,[m:][addr=]val)*\n");
	fprintf(stderr, "               -d x:0xc00c=1,2,3,4,y:0x187f=10\n");
	fprintf(stderr, " -n            no NAND header\n");
	fprintf(stderr, " -i<type>      vs1000nand vs1000spi vs1000ramdisk vs1005nand vs1005spi vs1005fspi vs1005gsd vs10xx plugin cmd\n");
	fprintf(stderr, " --no-compression   force no compression\n");
	fprintf(stderr, " --no-merge         prevent merging of sections\n");
	fprintf(stderr, " --ignore-size      ignore size for NAND images\n");
	fprintf(stderr, " -t<type>      NAND type\n");
	fprintf(stderr, " -b<size>      NAND erase block bits\n");
	fprintf(stderr, " -s<size>      NAND total size bits\n");
	fprintf(stderr, " -w<waitns>    NAND rd/wr active time in ns\n");
	fprintf(stderr, " -c<chip>      NAND pre-defined setting\n");
	fprintf(stderr, " -k <keyval>,[<mask>] VS1005 unique id, optional mask\n");
	fprintf(stderr, "**** Overlay support ****\n");
	fprintf(stderr, "    %s [-C] [--add-to-overlay <ovlname> <linkedobject>]* <boot-image> overlay_control\n",
		argv[0]);
	fprintf(stderr, "    %s [-C] memremain_static mem_desc.available\n",
		argv[0]);
	fprintf(stderr, "    %s [-C] memremain_overlay mem_desc.available\n",
		argv[0]);
	fprintf(stderr, "    %s [-C] memremain_overlay mem_desc.available\n",
		argv[0]);
	return EXIT_FAILURE;
    }
    return SendFlashData(putHeader, execAddr, file[0], outFile);
}


