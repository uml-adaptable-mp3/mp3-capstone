#ifndef SPEED_SHIFT_H
#define SPEED_SHIFT_H

#include <vstypes.h>


#define SS_WIN 4096
#if 0
#  define SS_MEM_SIZE 2048
#else
#  define SS_MEM_SIZE 2624
#endif
#define SS_OUTBUF_SIZE 64

#ifndef ASM
auto s_int16 AudioOutPart2(s_int16 *p, s_int16 n);
auto void SpeedShiftInit(void);
auto void SpeedShiftSet(double speed);
auto s_int16 SpeedShift(s_int16 *p, u_int16 n);

struct SpeedShift {
  s_int16 phase;			// Offset 0 (needed by SpdOutputSample)
  s_int16 __mem_y *readP1;					// Offset 1
  s_int16 __mem_y *readP2;					// Offset 2
  const u_int16 *winP1, *winP2;				// Offset 3
  s_int16 winAdd1, winAdd2, winVal1, winVal2, winPhase;	// Offset 5
  s_int16 skip, repeat, oldRepeat, dontMove;   		// Offset 10
  double speed;					// Offset 14
  s_int16 set;						// Offset 17
  s_int16 *outBufP;					// Offset 18
  s_int16 outBuf[SS_OUTBUF_SIZE];			// Offset 19
};

struct SpeedShiftY {
  s_int16 mem[SS_MEM_SIZE];
};

extern struct SpeedShift speedShift;
extern struct SpeedShiftY __mem_y speedShiftY;

#endif /* !ASM */

#endif /* !SPEED_SHIFT_H */
