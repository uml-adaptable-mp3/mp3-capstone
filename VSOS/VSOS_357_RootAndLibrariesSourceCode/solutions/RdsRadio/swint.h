#ifndef SWINT_H
#define SWINT_H
//
// StdWidget internal definitions and data structures
//
#define RESX 240 // screen resolution in pixels
#define RESY 320

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

typedef struct LayoutManagerState {
	LayoutManager layoutManager;
	s_int16 currentIndex; // during ProcessWidgets, this is the index of the current widget
	s_int16 lastVisible; // layout manager should update this when a UI element is rendered
	s_int16 drawPosition; // screen position of the last rendered widget
	s_int16 firstVisible; // index of the first visible widget
} LayoutManagerState;

void ProcessWidgets(regz u_int16,regz LMCallback,regz s_int16 finish,regz u_int16);
void WidgetPainter(regz StdWidget*);
void WidgetListener(regz StdWidget*);

extern u_int16 scanLine[];
extern u_int16 btnX,btnY,btnW,btnH;
extern s_int16 btnTransition;
extern u_int16 appRunning;
extern u_int16 currentLayerMask;

typedef void (*RenderEvent)(void);
typedef s_int16 (*MiscEvent)(regz SWEvent,regz s_int16);

typedef struct DialogInfo {
	RenderEvent beginUpdate;
	RenderEvent beginPaint;
	RenderEvent finish;
	LayoutManagerState* clientLayout;
	StdWidget* sysWidgets;
	MiscEvent miscEvent;
	void* extra;
} DialogInfo;

#include <StdWidgetGfx.h>

#define SysButtonSize(w,h) (((w)/2)|(((h)/2)<<8))

#endif