//==============================================================================
// drawing.cpp
//==============================================================================

//========================================================================================
#include <windows.h>
#include <vector>
#include "client.h"
#include "color.h"
#include "TableFont.h"
#include "menu.h"
#include "cSprites.h"
#include "drawing.h"
#include "players.h"
#include <gl/gl.h>
#pragma warning(disable:4786)
#pragma warning(disable:4800)
#pragma warning(disable:4244)
#pragma warning(disable:4101)
#pragma warning(disable:4715)
#pragma warning(disable:4305)

extern bool oglSubtractive;

int displayCenterX;
int displayCenterY;
float fCurrentFOV = 90;
SCREENINFO screeninfo;
//==============================================================================
void InitVisuals(void)
{
	static bool bOnce = false;
	if(bOnce) return;
	screeninfo.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&screeninfo);
	displayCenterX = screeninfo.iWidth/2;
	displayCenterY = screeninfo.iHeight/2;
    //displayCenterX = gEngfuncs.GetWindowCenterX();
    //displayCenterY = gEngfuncs.GetWindowCenterY();
	gMenu.Init();
	spritedata.iSpritesLoaded = 0;
	int o = 1; // any value?
	hookpfnSPR_GetList("sprites/WEAPONLIST_knife.txt",&o);
	bOnce = true;
}
//==============================================================================
void DrawCrosshair(void)
{
	ColorEntry* color  = colorList.get(9);  // "cross"
	ColorEntry* colo2  = colorList.get(10); // "cross2"

	int r=color->r, g=color->g, b=color->b, a=color->a;
	int R=colo2->r, G=colo2->g, B=colo2->b, A=colo2->a;

	oglSubtractive = true;

	int tcross = cvar.crosshair;
	if (me.inZoomMode) cvar.crosshair = 6;

	if(cvar.crosshair == 1)
	{
		gEngfuncs.pfnFillRGBA(displayCenterX - 14, displayCenterY, 9, 1,r,g,b,a);
		gEngfuncs.pfnFillRGBA(displayCenterX +5,   displayCenterY, 9, 1,r,g,b,a);
		gEngfuncs.pfnFillRGBA(displayCenterX, displayCenterY - 14, 1, 9,r,g,b,a);
		gEngfuncs.pfnFillRGBA(displayCenterX, displayCenterY +  5, 1, 9,r,g,b,a);
		gEngfuncs.pfnFillRGBA(displayCenterX, displayCenterY     , 1, 1,R,G,B,A); // center
	}
	else if(cvar.crosshair == 2)
	{
		gEngfuncs.pfnFillRGBA(displayCenterX - 14, displayCenterY, 9,2,r,g,b,a); // left
		gEngfuncs.pfnFillRGBA(displayCenterX +6,   displayCenterY, 9,2,r,g,b,a); // right
		gEngfuncs.pfnFillRGBA(displayCenterX, displayCenterY - 14, 2,9,r,g,b,a); // top
		gEngfuncs.pfnFillRGBA(displayCenterX, displayCenterY +  7, 2,9,r,g,b,a); // bottom
		gEngfuncs.pfnFillRGBA(displayCenterX, displayCenterY     , 2,2,R,G,B,A); // center
	}
	else if(cvar.crosshair == 3)
	{
		gEngfuncs.pfnFillRGBA(displayCenterX-25,displayCenterY,50,1,r,g,b,a);
		gEngfuncs.pfnFillRGBA(displayCenterX,displayCenterY-25,1,50,r,g,b,a);

		gEngfuncs.pfnFillRGBA(displayCenterX - 5, displayCenterY, 10, 1,R,G,B,A);
		gEngfuncs.pfnFillRGBA(displayCenterX, displayCenterY - 5,  1,10,R,G,B,A);
	}
	else if(cvar.crosshair == 4)
	{
		gEngfuncs.pfnFillRGBA(displayCenterX-25,displayCenterY,50,2,r,g,b,a);
		gEngfuncs.pfnFillRGBA(displayCenterX,displayCenterY-25,2,50,r,g,b,a);

		gEngfuncs.pfnFillRGBA(displayCenterX - 5, displayCenterY, 10, 2,R,G,B,A);
		gEngfuncs.pfnFillRGBA(displayCenterX, displayCenterY - 5,  2,10,R,G,B,A);
	}
	else if(cvar.crosshair == 5)
	{
		int iX=int(displayCenterX), iY=int(displayCenterY), iSize=25;
		// border top
		tintArea(iX-iSize, iY-iSize, 2*iSize, 1, 255, 255, 255, 180);
		//border bottom
		tintArea(iX-iSize, iY+iSize, 2*iSize, 1, 255, 255, 255, 180);
		//border left
		tintArea(iX-iSize, iY-iSize+1, 1, 2*iSize-1, 255, 255, 255, 180);
		//border right
		tintArea(iX+iSize, iY-iSize, 1, 2*iSize+1, 255, 255, 255, 180);

		//cross
		tintArea(iX, iY-iSize+1, 1, 2*iSize-1, 0, 160, 0, 180);
		tintArea(iX-iSize+1, iY, 2*iSize-1, 1, 0, 160, 0, 180);
	}
	else if(cvar.crosshair == 6)
	{
		gEngfuncs.pfnFillRGBA(0,displayCenterY,2*displayCenterX,1,r,g,b,a);
		gEngfuncs.pfnFillRGBA(displayCenterX,0,1,2*displayCenterY,r,g,b,a);

		gEngfuncs.pfnFillRGBA(displayCenterX - 5, displayCenterY, 10, 1,R,G,B,A);
		gEngfuncs.pfnFillRGBA(displayCenterX, displayCenterY - 5,  1,10,R,G,B,A);
	}
	oglSubtractive = false;

	cvar.crosshair = tcross;
}
//==============================================================================
int DrawLen(char *fmt)
{
	int len=0;
	for ( char * p = fmt; *p; p++ ) len+=screeninfo.charWidths[*p]; 
	return len;
}
//==============================================================================
void DrawHudStringCenter (int x, int y, int r, int g, int b, const char *fmt, ... )
{
	va_list va_alist;
	char buf[256];

	va_start (va_alist, fmt);
	_vsnprintf (buf, sizeof(buf), fmt, va_alist);
	va_end (va_alist);
	
	// y-check
	int borderY = displayCenterY*2-18;
	if( y<0 || y>borderY ) { return; }

	int drawLen = DrawLen(buf);
	x = x-drawLen/2;

	int borderX = displayCenterX*2-11;
	int minX = x;
	int maxX = x+drawLen;
	bool needSingleCheck = (minX<1 || maxX>borderX);

	if( needSingleCheck )
	{
		for ( char * p = buf; *p; p++ )
		{
			int next = x + screeninfo.charWidths[*p];
			// IMPORTANT NOTE: when drawing admin-mod style charactters
			//    you MAY NOT provide x/y coordinates that cause drawing
			//    off screen. This causes HL to crash or just quit
			if( x>0 && x<borderX )
				gEngfuncs.pfnDrawCharacter (x, y, *p, r, g, b);
			x = next;
		}
	} else {
		for ( char * p = buf; *p; p++ )
		{
			int next = x + screeninfo.charWidths[*p];
			
			// IMPORTANT NOTE: when drawing admin-mod style charactters
			//    you MAY NOT provide x/y coordinates that cause drawing
			//    off screen. This causes HL to crash or just quit
			gEngfuncs.pfnDrawCharacter (x, y, *p, r, g, b);
			x = next;
		}
	}
}

//========================================================================================
void DrawHudString (int x, int y, int r, int g, int b, const char *fmt, ... )
{
	va_list va_alist;
	char buf[256];

	va_start (va_alist, fmt);
	_vsnprintf (buf, sizeof(buf), fmt, va_alist);
	va_end (va_alist);
	
	// y-check
	int borderY = displayCenterY*2-18;
	if( y<0 || y>borderY ) { return; }
	
	bool needSingleCheck=false;
	int borderX = displayCenterX*2-11;
		
	int drawLen = DrawLen(buf);
	if( x<1 ){ needSingleCheck=true; }
	else
	{
		int maxX = x+drawLen;
		needSingleCheck = (maxX>borderX);
	}

	if( needSingleCheck )
	{
		for ( char * p = buf; *p; p++ )
		{
			int next = x + screeninfo.charWidths[*p];
			// IMPORTANT NOTE: when drawing admin-mod style charactters
			//    you MAY NOT provide x/y coordinates that cause drawing
			//    off screen. This causes HL to crash or just quit
			if( x>0 && x<borderX )
				gEngfuncs.pfnDrawCharacter (x, y, *p, r, g, b);
			x = next;
		}
	}else {
		for ( char * p = buf; *p; p++ )
		{
			int next = x + screeninfo.charWidths[*p];
			// IMPORTANT NOTE: when drawing admin-mod style charactters
			//    you MAY NOT provide x/y coordinates that cause drawing
			//    off screen. This causes HL to crash or just quit
			gEngfuncs.pfnDrawCharacter (x, y, *p, r, g, b);
			x = next;
		}
	}	
}

//========================================================================================

void DrawConString (int x, int y, int r, int g, int b, const char *fmt, ... )
{
	va_list va_alist;
	char buf[256];

	va_start (va_alist, fmt);
	_vsnprintf (buf, sizeof(buf), fmt, va_alist);
	va_end (va_alist);

	int length, height;

	y += 4;
	gEngfuncs.pfnDrawConsoleStringLen( buf, &length, &height );
	gEngfuncs.pfnDrawSetTextColor(r/255.0f, g/255.0f, b/255.0f);
	gEngfuncs.pfnDrawConsoleString(x,y,buf);
}

//========================================================================================

void DrawConStringCenter (int x, int y, int r, int g, int b, const char *fmt, ... )
{
	va_list va_alist;
	char buf[256];

	va_start (va_alist, fmt);
	_vsnprintf (buf, sizeof(buf), fmt, va_alist);
	va_end (va_alist);

	int length, height;

	gEngfuncs.pfnDrawConsoleStringLen( buf, &length, &height );
	x -= length/2;
	y += 4;
	gEngfuncs.pfnDrawSetTextColor(r/255.0f, g/255.0f, b/255.0f);
	gEngfuncs.pfnDrawConsoleString(x,y,buf);
}

//========================================================================================

void gDrawBoxAtScreenXY( int x, int y, int r, int g, int b, int alpha, int radius=1)
{
	int radius2 = radius<<1;
	gEngfuncs.pfnFillRGBA(x-radius+2, y-radius, radius2-2,2,r,g,b,alpha);
	gEngfuncs.pfnFillRGBA(x-radius, y-radius, 2,radius2,r,g,b,alpha);
	gEngfuncs.pfnFillRGBA(x-radius, y+radius, radius2,2,r,g,b,alpha);
	gEngfuncs.pfnFillRGBA(x+radius, y-radius, 2,radius2+2,r,g,b,alpha);
}

//========================================================================================

void whiteBorder(int x,int y,int w, int h)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA( x-1, y-1, w+2, 1 ,255,255,255,254 ); //top
	gEngfuncs.pfnFillRGBA( x-1, y, 1, h-1 ,255,255,255,254 );	//left
	gEngfuncs.pfnFillRGBA( x+w, y, 1, h-1 ,255,255,255,254 );	//right
	gEngfuncs.pfnFillRGBA( x-1, y+h-1, w+2, 1 ,255,255,255,254 ); //bottom
	oglSubtractive = false;
}

//========================================================================================

void blackBorder(int x,int y,int w, int h)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA( x-1, y-1, w+2, 1 ,0,0,0,254 ); //top
	gEngfuncs.pfnFillRGBA( x-1, y, 1, h-1 ,0,0,0,254 );	//left
	gEngfuncs.pfnFillRGBA( x+w, y, 1, h-1 ,0,0,0,254 );	//right
	gEngfuncs.pfnFillRGBA( x-1, y+h-1, w+2, 1 ,0,0,0,254 ); //bottom
	oglSubtractive = false;
}

//========================================================================================

void tintArea(int x,int y,int w,int h, ColorEntry* clr)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(x,y,w,h,clr->r,clr->g,clr->b,clr->a);
	oglSubtractive = false;
}

//========================================================================================

void tintArea(int x,int y,int w,int h,int r,int g,int b,int a)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(x,y,w,h,r,g,b,a);
	oglSubtractive = false;
}

//========================================================================================

void colorBorder(int x,int y,int w, int h,int r,int g,int b,int a)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA( x-1, y-1, w+2, 1 ,r,g,b,a ); //top
	gEngfuncs.pfnFillRGBA( x-1, y, 1, h-1 ,r,g,b,a );	//left
	gEngfuncs.pfnFillRGBA( x+w, y, 1, h-1 ,r,g,b,a );	//right
	gEngfuncs.pfnFillRGBA( x-1, y+h-1, w+2, 1 ,r,g,b,a ); //bottom
	oglSubtractive = false;
}

//========================================================================================

void dddBorder(int x,int y,int w, int h, int a)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA( x-1, y-1, w+2, 1 ,255,255,255,a ); //top
	gEngfuncs.pfnFillRGBA( x-1, y, 1, h-1 ,255,255,255,a );	//left
	gEngfuncs.pfnFillRGBA( x+w, y, 1, h-1 ,0,0,0,a );	//right
	gEngfuncs.pfnFillRGBA( x-1, y+h-1, w+2, 1 ,0,0,0,a ); //bottom
	oglSubtractive = false;
}

//========================================================================================

void dddBorder2(int x,int y,int w, int h, int a)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA( x-1, y-1, w+2, 1 ,0,0,0,a ); //top
	gEngfuncs.pfnFillRGBA( x-1, y, 1, h-1 ,0,0,0,a );	//left
	gEngfuncs.pfnFillRGBA( x+w, y, 1, h-1 ,255,255,255,a );	//right
	gEngfuncs.pfnFillRGBA( x-1, y+h-1, w+2, 1 ,255,255,255,a ); //bottom
	oglSubtractive = false;
}

//========================================================================================
void PrintWithFont(int x, int y, int r, int g, int b, const char *fmt, ... )
{
	va_list va_alist;
	char buf[256];

	va_start (va_alist, fmt);
	_vsnprintf (buf, sizeof(buf), fmt, va_alist);
	va_end (va_alist);
	gFont.drawString(false, x, y, r, g, b ,buf);
}
//========================================================================================