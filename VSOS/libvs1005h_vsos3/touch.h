/// \file touch.h Touchpad
#ifndef TOUCH_H
#define TOUCH_H

struct TouchInfo {
  s_int16 minX, minY, maxX, maxY;	/*< Calibration data */
  s_int16 calibrationMode;		/*< If non-0, give direct X/Y values */
  s_int16 persistance;			/*< How many TICKS_PER_SEC to keep pushed */
  u_int32 lastPressureTimeCount;	/*< timeCount when last noticed push */
  s_int16 oldX, oldY;			/*< Old X and Y values */
};

extern struct TouchInfo touchInfo;

/* Returns:
   20 if current touch gives a new result,
   10 if replaying old result,
    0 if no touch detected. */
s_int16 GetTouchLocation(s_int16 *x, s_int16 *y);

#endif
