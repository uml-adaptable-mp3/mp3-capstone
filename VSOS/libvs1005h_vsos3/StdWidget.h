#ifndef STDWIDGET_H
#define STDWIDGET_H
//
// +-------------------------------------------------+
// | StdWidget: UI graphics library for VSOS         |
// | Copyright (c) 2013 - 2014 VLSI Solution Oy      |
// +-------------------------------------------------+
//
// (the main API calls are located at the end of this file.)

#define regz register

#ifdef WIN32
typedef unsigned short u_int16;
typedef signed short s_int16;
typedef int s_int32;
typedef unsigned u_int32;
#else
#include <vstypes.h>
#endif

typedef unsigned wchar; // 16-bit character type

typedef s_int16 (*SliderValueCallback)(regz struct StdWidget*,regz s_int16 value);

typedef struct SliderValue {
	s_int16 current,max; // a slider control's value ranges from 0..max
	SliderValueCallback accept;
	// accept function is called once for each adjustment to the slider's value. It should return
	// the closest acceptable value (can be NULL in which case any value between 0 and max is OK)
} SliderValue;
//
// WidgetData: the internal state of an StdWidget
//
typedef union WidgetData {
	struct Button { // applies to the default widget type
		void* ptr_p; // user-defined data
		u_int16 int_p;
	} wt_button;
	struct Custom { // widgets using the WT_CUSTOM type
		unsigned width; // size hint for the layout manager (the custom painting method should
		u_int16 height; // prepare for different actual size whenever possible)
	} wt_custom;
} WidgetData;

typedef struct BtnPos { s_int16 x,y,w,h; } BtnPos;

typedef void (*UserEvent)(regz struct StdWidget*,regz s_int16 x,regz s_int16 y,regz u_int16 hover);
// UserEvent is called when user interacts with the widget. A button widget does not receive hover events.
typedef wchar* (*CaptionCallback)(regz struct StdWidget* widget);
// system calls CaptionCallback when a new caption (text) is needed for a standard widget using the WF_DCAP flag
typedef s_int16 (*DListCallback)(regz struct StdWidget* result,regz u_int16 index);
// system calls DListCallback whenever new widgets are revealed in a widget list (WT_DLIST type)
// (DListCallback should return 0 after the last item to indicate end-of-list)
typedef void (*RasterizationCallback)(regz u_int16* target,regz s_int16 line,regz BtnPos* width);
typedef RasterizationCallback (*RenderCallback)(regz struct StdWidget*,regz u_int16 pitch);
// System calls RenderCallback when a custom styled (WT_CUSTOM) widget should be drawn.
// RenderCallback should return a pointer to a function which can output any requested
// scan line of the widget into the target buffer (in RGB565 pixel format).
//
// WidgetContent: the visible content of an StdWidget
//
typedef union WidgetContent {
	void* init; // not used
	wchar* caption; // static caption string
	CaptionCallback captionCallback; // WF_DCAP dynamic caption string
	DListCallback dynamicListCallback; // WF_DLIST dynamic list of widgets
	SliderValue* sliderValue; // WT_SLIDER widget
} WidgetContent;
//
// StdWidget defines the standard UI element (button, text field, etc)
//
typedef struct StdWidget {
	u_int16 flags; // one of the WL* | one of the WT_* | a combination of WF_*
	u_int16 symbol; // symbol is used together with (sometimes instead of) the caption
	WidgetContent content; // content defines what is shown on the widget
	UserEvent userEvent; // system calls this function whenever an event occurs
	RenderCallback renderCallback; // function responsible for drawing the widget
	u_int16 msgType; // UIMSG
	WidgetData data; // the internal state of the widget
} StdWidget;
//
// StdWidget layers (StdWidget::flags, bits 0..2)
// a layer definition associates each widget with one of the layout managers
//
#define WL0	0
#define WL1	1
#define WL2	2
#define WL3	3
#define WL4	4
#define WL5	5
#define WL6	6
#define WL7	7
//
// StdWidget types (StdWidget::flags, bits 13..15)
//
#define WT_BI				13 // bit index of the widget type field (occupies most significant bits)
#define WT_BUTTON			(0<<WT_BI) // a button is the basic type of a widget (appearance depends on Style Manager)
#define WT_SLIDER			(1<<WT_BI) // a slider can be adjustable value, progress bar, volume bar etc
#define WT_DLIST			(2<<WT_BI) // a dynamic list of widgets
#define WT_RESERVED		(3<<WT_BI) // 3..6 reserved space for additional standard widget types
#define WT_CUSTOM			(7<<WT_BI) // custom widget type (NOTE: you need to provide a RenderCallback for this type!)
//
// StdWidget flags (bits 3..12)
//
#define WF_HIDDEN			(1<<3) // widget exists but should not be drawn at all (other widgets may take its place)
#define WF_INACTIVE		(1<<4) // widget does not respond to user-generated events but is drawn normally
#define WF_DISABLED		((1<<5)|WF_INACTIVE) // widget is inactive and should be drawn in 'grayed' state
#define WF_ACTIVATED		(1<<6) // widget is activated (i.e. pressed down)
#define WF_ALTSTYLE		(1<<7) // a hint for the style manager to draw the widget in an alternative style
#define WF_DCAP			(1<<8) // dynamic caption: provide a CaptionCallback for this kind of widget
#define WF_SELECTED		(1<<9) // a hint for the style manager to draw the widget in a selected state
#define WF_PAINT			(1<<10) // request to repaint the widget (flag will be cleared when done)
#define WF_RESERVED0		(1<<11) // reserved for future extension
#define WF_RESERVED1		(1<<12) // -"-
// Derived widget types:
#define WT_END_OF_LIST 	0xFFFF
#define WT_LABEL			(WT_BUTTON|WF_INACTIVE) // a text label is a button which cannot be interacted with
// Some convenience macros:
#define WidgetType(w)	((w)->flags&(7<<WT_BI)) // returns the widget type from a pointer to an StdWidget
#define LayerMask(l)		(1<<(l))
#define RGB555(r,g,b) 	(((r)<<11)|((g)<<6)|(b))
#define GRAY5(a) 			(((a)<<11)|((a)<<6)|(a))
//
typedef struct IconInfo {
	s_int16 w,h,bpp,num; // icon size is w*h; bpp=bits per pixel; num=max number of icons in RAM
	u_int16* iconData;
} IconInfo;
//
typedef void (*StyleManager)(regz struct LayoutManagerState**,regz BtnPos**,regz IconInfo**);
//
typedef enum SWC {
	swcGetIconInfo,		// returns a pointer to IconInfo (the app must fill iconData)
	swcGetLayoutManager,	// returns layout manager of a specified layer (iparam)
	swcSetLayoutManager,	// redefines layout manager (pparam) of the specified layer (iparam)
	swcRepaint,			// force full repaint of the screen
} SWC;
//
typedef void (*StdWidgetInitProc)(regz StyleManager,regz void*);
#define SWInit(lib,sm,up) (*(((StdWidgetInitProc*)(lib))+2+ENTRY_1))(sm,up)
typedef void (*StdWidgetUpdateProc)(regz u_int16 hiddenLayerMask,regz StdWidget* widgetList);
#define SWUpdate(lib,hlm,wl) (*(((StdWidgetUpdateProc*)(lib))+2+ENTRY_2))(hlm,wl)
typedef void* (*StdWidgetCtrlProc)(regz SWC,regz s_int16 iparam,regz void* pparam);
#define SWCtrl(lib,c,ip,pp) (*(((StdWidgetCtrlProc*)(lib))+2+ENTRY_3))(c,ip,pp)
//
#define SWSGet(lib) (*(((StyleManager*)(lib))+2+ENTRY_1))

#endif