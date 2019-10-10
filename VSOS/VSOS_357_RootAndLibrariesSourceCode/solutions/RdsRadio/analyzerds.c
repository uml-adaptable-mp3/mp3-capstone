#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define OFFSET_A  0x0fc
#define OFFSET_B  0x198
#define OFFSET_C  0x168
#define OFFSET_C2 0x350
#define OFFSET_D  0x1b4
#define OFFSET_E  0x000

#define u_int16 unsigned short
#define s_int16 short

u_int16 CheckSyndrome(u_int16 data, u_int16 check, u_int16 offset) {
	u_int16 syndrome = 0;
	u_int16 idx = 0;
	u_int16 input = data;
	
	// MSb first for data!
	// syndrome index 9 is in LEFTmost bit, ie. shifts left, input to [6]
#if 0
	printf("#%04x:%04x:%04x#", data, check, offset);
#endif
	for (idx = 0; idx < 16; idx++) {
		u_int16 fb = input ^ syndrome;
		
		//plain shift syndrome [0] (6) goes(remains) zero
		syndrome <<= 1; // msb out
		if ((fb & 0x8000U) != 0) {
			// add generator
			syndrome ^= 0x6e40;
		}
		input <<= 1;
	}
	
	syndrome ^= (offset<<6);
	
	if (syndrome == (check<< 6)) {
		return 0;
	}
	else {
		return 1;
	}
}


u_int16 RdsBlockFix(u_int16 *data, u_int16 check, u_int16 offset) {
  u_int16 syndrome[10];
  u_int16 idx = 0;
  u_int16 input = *data;
  u_int16 result = *data;
  u_int16 par = check ^ offset;
  u_int16 fixes = 0;
	
  // first and current error locations
  u_int16 initial = 0;
  u_int16 current = 0;
	
  memset(syndrome, 0, sizeof(syndrome));
	
  // insert data to syndrome register
  for (idx = 0; idx < 16; idx++) {
    u_int16 lb = syndrome[9]; // loopback
    u_int16 bin = input >> 15; // bit in
		
    syndrome[9] = syndrome[8] ^ bin;
    syndrome[8] = syndrome[7] ^ lb ^ bin;
    syndrome[7] = syndrome[6] ^ lb;
    syndrome[6] = syndrome[5];
    syndrome[5] = syndrome[4] ^ lb;
    syndrome[4] = syndrome[3] ^ lb ^ bin;
    syndrome[3] = syndrome[2] ^ lb ^ bin;
    syndrome[2] = syndrome[1];
    syndrome[1] = syndrome[0] ^ bin;
    syndrome[0] = lb ^ bin;
		
    input <<= 1;
  }
	
  // insert check-bits to syndrome register
  for (idx = 0; idx < 10; idx++) {
    u_int16 lb = syndrome[9]; // loopback
    u_int16 bin = (par >> 9) & 0x0001; // bit in
		
    syndrome[9] = syndrome[8] ^ bin;
    syndrome[8] = syndrome[7] ^ lb ^ bin;
    syndrome[7] = syndrome[6] ^ lb;
    syndrome[6] = syndrome[5];
    syndrome[5] = syndrome[4] ^ lb;
    syndrome[4] = syndrome[3] ^ lb ^ bin;
    syndrome[3] = syndrome[2] ^ lb ^ bin;
    syndrome[2] = syndrome[1];
    syndrome[1] = syndrome[0] ^ bin;
    syndrome[0] = lb ^ bin;
		
    par <<= 1;
  }
	
  // fixes
  for (idx = 0; idx < 26; idx++) {
    u_int16 lb = syndrome[9]; // loopback
		
    if (syndrome[0] == 0 && syndrome[1] == 0 && syndrome[2] == 0
	&& syndrome[3] == 0 && syndrome[4] == 0 && syndrome [9] == 1){
      u_int16 iii = 0;
	    	    
      for (iii = 0; iii < 5; iii++) {
	u_int16 fix;
				
	if ((idx+iii) > 15) {
	  // past the data?
	  if (syndrome[9-iii] != 0) {
	    // uncorrectable
	    fixes = 0xfffe;
	    break;
	  }
	  else {
	    // no problem yet
	    continue;
	  }
	}
				
	fix = syndrome[9-iii] << (15- (idx+iii));
	if (fix != 0) {
	  if (fixes == 0) {
	    if (initial != 0) {
#ifdef DBG
	      puts("ERROR: Cant find first error!");
#endif
	    }
	    initial = idx+iii;
	  }
	  else {
	    if (((idx+iii) -initial) >= 5) {
	      // Too many errors
#ifdef DBG
	      printf("INITIAL: %u, current %u\n",initial, (idx+iii));
#endif
	      fixes = 0xffff;
	      break;
	    }
	  }
	  fixes++;
	}
	
	result ^= fix;
      }
			
      //no more than one error burst
      break;
      
    }
    syndrome[9] = syndrome[8];
    syndrome[8] = syndrome[7] ^ lb;
    syndrome[7] = syndrome[6] ^ lb;
    syndrome[6] = syndrome[5];
    syndrome[5] = syndrome[4] ^ lb;
    syndrome[4] = syndrome[3] ^ lb;
    syndrome[3] = syndrome[2] ^ lb;
    syndrome[2] = syndrome[1];
    syndrome[1] = syndrome[0];
    syndrome[0] = lb;
  }
	
  // check for decode failure; too many errors
  if (fixes == 0) {
    for (idx = 0; idx < 5; idx++) {
      if (syndrome[idx] != 0) {
	// uncorrectable error
	fixes = 0xffff;
	break;
      }
    }
  }
  else {
    *data = result;
  }

  return fixes;
}







/*
  YLE @ 29a0
 */

int main(void) {
  unsigned int line=0, data, check;
  unsigned long rawBits = 0;
  int phase = 15;
  int currByte = 0;
  int groupBitPhase = -1;
  static const u_int16 checkOffset[5] = {
    OFFSET_A, OFFSET_B, OFFSET_C, OFFSET_C2, OFFSET_D
  };

  while (1) {
    char d[2];
    u_int16 d16;
    int i, syndr[5];
    int anyGood = 0;

    line++;
    if (++phase >= 8) {
      phase = 0;
      if ((currByte = fgetc(stdin)) < 0) {
	goto finally;
      }
    }
    rawBits = (rawBits << 1) | ((currByte >> (7-phase)) & 1);

    if (groupBitPhase < 0 || (++groupBitPhase >= 26)) {
      data = (rawBits >> 10 & 0xffff);
      check = rawBits & 0x3ff;

      d[0] = data>>8;
      d[1] = data;
      if (!isprint(d[0])) {
	d[0] = '.';
      }
      if (!isprint(d[1])) {
	d[1] = '.';
      }
      for (i=0; i<5; i++) {
	syndr[i] = CheckSyndrome(data, check, checkOffset[i]);
	if (!syndr[i]) {
	  anyGood = 1;
	}
      }
      if (anyGood) {
	groupBitPhase = 0;
      } else {
	groupBitPhase = -1;
      }

      printf("l %04x, dt %04x \"%c%c\", ",
	     line, data, d[0], d[1]);
      printf("check %04x: synd %c %c %c%c %c ",
	     check,
	     syndr[0] ? '.' : 'X',
	     syndr[1] ? '.' : 'X',
	     syndr[2] ? '.' : 'X',
	     syndr[3] ? '.' : 'X',
	     syndr[4] ? '.' : 'X');
#if 0
      printf("/");
      d16 = data;
      printf(" %04x", RdsBlockFix(&d16, check, OFFSET_A) ? '.' : '#');
      printf("%c", CheckSyndrome(d16, check, OFFSET_A) ? '.' : 'X');
      d16 = data;
      printf(" %04x", RdsBlockFix(&d16, check, OFFSET_B) ? '.' : '#');
      printf("%c", CheckSyndrome(d16, check, OFFSET_B) ? '.' : 'X');
      d16 = data;
      printf(" %04x", RdsBlockFix(&d16, check, OFFSET_C) ? '.' : '#');
      printf("%c", CheckSyndrome(d16, check, OFFSET_C) ? '.' : 'X');
      d16 = data;
      printf(" %04x", RdsBlockFix(&d16, check, OFFSET_C2) ? '.' : '#');
      printf("%c", CheckSyndrome(d16, check, OFFSET_C2) ? '.' : 'X');
      d16 = data;
      printf(" %04x", RdsBlockFix(&d16, check, OFFSET_D) ? '.' : '#');
      printf("%c", CheckSyndrome(d16, check, OFFSET_D) ? '.' : 'X');
#endif      
      printf("\n");
    }
  }

 finally:
  return EXIT_SUCCESS;
}
