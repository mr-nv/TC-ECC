//==============================================================================
// drawing.h
//==============================================================================
#ifndef _DRAWING_
#define _DRAWING_

#include "client.h"
#include "color.h"
#include "cvar.h"
#include <windows.h>
#include <gl\gl.h>

int DrawLen(char *fmt);
void DrawHudStringCenter (int x, int y, int r, int g, int b, const char *fmt, ... );
void DrawHudString (int x, int y, int r, int g, int b, const char *fmt, ... );
void DrawConStringCenter (int x, int y, int r, int g, int b, const char *fmt, ... );
void DrawConString (int x, int y, int r, int g, int b, const char *fmt, ... );
void gDrawBoxAtScreenXY( int x, int y, int r, int g, int b, int alpha, int radius);
void whiteBorder(int x,int y,int w, int h);
void blackBorder(int x,int y,int w, int h);
void tintArea(int x,int y,int w,int h, struct ColorEntry* clr);
void tintArea(int x,int y,int w,int h,int r,int g,int b,int a);
void colorBorder(int x,int y,int w, int h,int r,int g,int b,int a);
void dddBorder(int x,int y,int w, int h, int a);
void dddBorder2(int x,int y,int w, int h, int a);
void PrintWithFont(int x, int y, int r, int g, int b, const char *fmt, ... );
void InitVisuals(void);
void DrawCrosshair(void);

extern int displayCenterX;
extern int displayCenterY;
extern SCREENINFO screeninfo;
extern float fCurrentFOV;

#endif