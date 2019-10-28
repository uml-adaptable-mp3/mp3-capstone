#ifndef SWINT_H
#define SWINT_H
//
// StdWidget internal definitions and data structures
//
#define RESX 320 // screen resolution in pixels
#define RESY 240

#ifdef WIN32
void LcdFilledRectangle(s_int16,s_int16,s_int16,s_int16,u_int16*,u_int16);
s_int16 GetTouchLocation(s_int16* x,s_int16* y);
void memset16(void*,u_int16,u_int16);
#else
#include <lcd.h>
#include <touch.h>
#define memset16 memset
#endif

#define NUM_LAYERS 8 // (update StdWidget::flags if changed)
#define MAX_DI 8 // (max number of widgets simultaneously visible in a dynamic widget list)

typedef void (*LMCallback)(regz StdWidget*);
typedef u_int16 (*LayoutManager)(regz struct LayoutManagerState*,regz StdWidget*,regz LMCallback);
typedef void (*FinishLayout)(regz struct LayoutManagerState*);

typedef struct LayoutManagerState {
	LayoutManager layoutManager; // procedure responsible for laying out widgets belonging to this layer
	FinishLayout finishLayout; // procedure responsible for updating areas not covered by widgets
	s_int16 currentIndex; // during ProcessWidgets, this is the index of the current widget
	s_int16 lastVisible; // index of the last rendered widget
	s_int16 firstVisible; // index of the first visible widget
	s_int16 size; // optional field updated by layout manager and passed to finish layout
} LayoutManagerState;

#endif