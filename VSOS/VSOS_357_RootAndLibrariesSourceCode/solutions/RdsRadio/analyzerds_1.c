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
  unsigned int line, timestamp, data, check;
  int lastTimestamp = 0;
  unsigned long long rawBits = 0;

  while(scanf("%x %x %x %x", &line, &timestamp, &data, &check) == 4) {
    char d[2];
    u_int16 d16;
    d[0] = data>>8;
    d[1] = data;
    if (!isprint(d[0])) {
      d[0] = '.';
    }
    if (!isprint(d[1])) {
      d[1] = '.';
    }
    printf("l %04x, t %06x -> %04x, dt %04x \"%c%c\", ",
	   line, timestamp, timestamp-lastTimestamp, data, d[0], d[1]);
    printf("check %04x: blk %d, %s, checkword %03x, synd %c %c %c%c %c ",
	   check, check & 3, ((check>>2) & 1) ? "err " : "*OK*", check >> 3,
	   CheckSyndrome(data, check>>3, OFFSET_A) ? '.' : 'X',
	   CheckSyndrome(data, check>>3, OFFSET_B) ? '.' : 'X',
	   CheckSyndrome(data, check>>3, OFFSET_C) ? '.' : 'X',
	   CheckSyndrome(data, check>>3, OFFSET_C2) ? '.' : 'X',
	   CheckSyndrome(data, check>>3, OFFSET_D) ? '.' : 'X');
    lastTimestamp = timestamp;
#if 0
    printf("/");
    d16 = data;
    printf(" %04x", RdsBlockFix(&d16, check>>3, OFFSET_A) ? '.' : '#');
    printf("%c", CheckSyndrome(d16, check>>3, OFFSET_A) ? '.' : 'X');
    d16 = data;
    printf(" %04x", RdsBlockFix(&d16, check>>3, OFFSET_B) ? '.' : '#');
    printf("%c", CheckSyndrome(d16, check>>3, OFFSET_B) ? '.' : 'X');
    d16 = data;
    printf(" %04x", RdsBlockFix(&d16, check>>3, OFFSET_C) ? '.' : '#');
    printf("%c", CheckSyndrome(d16, check>>3, OFFSET_C) ? '.' : 'X');
    d16 = data;
    printf(" %04x", RdsBlockFix(&d16, check>>3, OFFSET_C2) ? '.' : '#');
    printf("%c", CheckSyndrome(d16, check>>3, OFFSET_C2) ? '.' : 'X');
    d16 = data;
    printf(" %04x", RdsBlockFix(&d16, check>>3, OFFSET_D) ? '.' : '#');
    printf("%c", CheckSyndrome(d16, check>>3, OFFSET_D) ? '.' : 'X');
#endif

    rawBits <<= 26;
    rawBits |= (u_int16)data << 10;
    rawBits |= ((check>>3) & 1023);
    printf("   %016llx ", rawBits);
    printf("   %c",
	   CheckSyndrome((u_int16)(rawBits>>(26+10-1)), (u_int16)(rawBits>>(26-1))&1023, OFFSET_A) ? '.':'X');
    printf(" %c",
	   CheckSyndrome((u_int16)(rawBits>>(26+10+0)), (u_int16)(rawBits>>(26+0))&1023, OFFSET_A) ? '.':'X');
    printf(" %c",
	   CheckSyndrome((u_int16)(rawBits>>(26+10+1)), (u_int16)(rawBits>>(26+1))&1023, OFFSET_A) ? '.':'X');

    printf("\n");
  }

  return EXIT_SUCCESS;
}
