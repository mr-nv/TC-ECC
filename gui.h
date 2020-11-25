#ifndef _GUI_
#define _GUI_

#include "drawing.h"
#include "color.h"
#include "TableFont.h"
#include "players.h"

class cGui
{
public:
	void window(int x, int y, int w, int h, float step, char* title);
	void InitFade(void);
	void FillRGBA(GLfloat x, GLfloat y, int w, int h, UCHAR r, UCHAR g, UCHAR b, UCHAR a);
	void FillRGBA(GLfloat x, GLfloat y, int w, int h, ColorEntry* clr);
	void DrawInfoBoxLeft(void);
private:
	void DrawTitleBox(int x, int y, int w, int h, char* title);
	void DrawContentBox(int x, int y, int w, int h, float step);
	void DrawFade(int x, int y, int w);
};
extern cGui gGui;

#endif