#include "gui.h"

#pragma warning(disable:4786)
#pragma warning(disable:4800)
#pragma warning(disable:4244)
#pragma warning(disable:4101)
#pragma warning(disable:4715)
#pragma warning(disable:4305)

cGui gGui;
float r[15];
float g[15];
float b[15];


void cGui::FillRGBA(GLfloat x, GLfloat y, int w, int h, UCHAR r, UCHAR g, UCHAR b, UCHAR a)
{
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(r,g,b,a);
	  glBegin(GL_QUADS);
	    glVertex2f(x,y);
		glVertex2f(x+w,y);
		glVertex2f(x+w,y+h);
		glVertex2f(x,y+h);
	  glEnd();
	glDisable(GL_BLEND);

	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void cGui::FillRGBA(GLfloat x, GLfloat y, int w, int h, ColorEntry* clr)
{
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(clr->r,clr->g,clr->b,clr->a);
	  glBegin(GL_QUADS);
	    glVertex2f(x,y);
		glVertex2f(x+w,y);
		glVertex2f(x+w,y+h);
		glVertex2f(x,y+h);
	  glEnd();
	glDisable(GL_BLEND);

	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void cGui::InitFade(void)
{
/*	r[0] = 151; g[0] = 201; b[0] = 251;
	r[1] = 146; g[1] = 194; b[1] = 251;
	r[2] = 136;	g[2] = 186; b[2] = 251;
	r[3] = 130;	g[3] = 185;	b[3] = 251;
	r[4] = 128;	g[4] = 184; b[4] = 250;
	r[5] = 125; g[5] = 183; b[5] = 250;
	r[6] = 126;	g[6] = 183;	b[6] = 250;
	r[7] = 115;	g[7] = 177;	b[7] = 249;
	r[8] = 103;	g[8] = 170; b[8] = 249;
	r[9] = 91;	g[9] = 164;	b[9] = 249;
	r[10]= 79;	g[10]= 157;	b[10]= 248;
	r[11]= 55;	g[11]= 144;	b[11]= 248;
	r[12]= 46;	g[12]= 139;	b[12]= 248;
	r[13]= 37;	g[13]= 134;	b[13]= 247;
	r[14]= 23;	g[14]= 126;	b[14]= 247;*/
	for(int i = 0; i < 15; i++) 
	{
		r[i] = 249 - (i*5); 
		g[i] = 117 - (i*7); 
		b[i] = 0;// - (i*0.5);
	}

}
extern SCREENINFO screeninfo;
void Print(int x, int y, int step, const char* fmt, ...)
{
	va_list va_alist;
	char buf[256];

	va_start (va_alist, fmt);
	_vsnprintf (buf, sizeof(buf), fmt, va_alist);
	va_end (va_alist);
	gFont.drawString(false, x, y+step*11, 255, 255, 255, buf);
}
int ammo = 0;
void cGui::DrawInfoBoxLeft(void)
{
	int x = 10;
	int y = screeninfo.iHeight/2;
	window(x, y, 120, 55, 3, "Infobox");
	ammo = me.iClip;
	if(ammo < 0) ammo = 0;
	Print(x,y,0,"Health: %i", me.iHealth);
	Print(x,y,1,"Ammo: %i", ammo);
	Print(x,y,2,"Armor: %i", me.iArmor);
	Print(x,y,3,"K: %i - D: %i - HS: %i", me.iKills, me.iDeaths, me.iHs);
	Print(x,y,4,"Money: %i", me.iMoney);
}

void cGui::DrawFade(int x, int y, int w)
{
	for(int i=0; i < 15; i++)
		FillRGBA(x, y+i, w, 1, r[i], g[i], b[i], 180);
}

void cGui::DrawTitleBox(int x, int y, int w, int h, char* title)
{
	ColorEntry * clr = colorList.get(0);
//	FillRGBA(x, y, w, h, clr);		    // Title Background
	DrawFade(x, y, w);					// Fade Background
	FillRGBA(x, y, w, 1, 0, 0, 0, 255); // Outline - Top
	FillRGBA(x, y, 1, h, 0, 0, 0, 255); // Outline - Left
	FillRGBA(x+w,y,1, h, 0, 0, 0, 255); // Outline - Right
	FillRGBA(x+4,y+5,8,8,0, 0, 0, 255); // Box Filling
	whiteBorder(x+4,y+5,8,8);			// White Border
	PrintWithFont(x+18,y+5,255,255,255,title);// Window Title
}

void cGui::DrawContentBox(int x, int y, int w, int h, float step)
{
	ColorEntry * clr = colorList.get(1);
//	FillRGBA(x, y, w, h, clr);			 // Content Box Background
//	for(int i=0; i < h; i++)
//		FillRGBA(x,y+i, w, 1, 255-(step*i), 255-(step*i), 255-(step*i), 180);
	FillRGBA(x,y,w,h,136,131,127,200);
	blackBorder(x+1, y+1, w-1, h);		 // Content Box Outlines
}

void cGui::window(int x, int y, int w, int h, float step, char* title)
{
	DrawTitleBox(x-2, y-17, w, 15, title); // Title Box
	DrawContentBox(x-2, y-2, w, h, step);  // Content Box
}