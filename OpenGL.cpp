//==================================================================================
// New OpenGL Hook ( Detours ) - Hysteria
//==================================================================================
#include <windows.h>
#include <detours.h>
#include <gl/gl.h>
#include "cvar.h"
#include "players.h"
#include "aimbot.h"
//==================================================================================
typedef void (APIENTRY *g_glBegin)(GLenum);
typedef void (APIENTRY *g_glBlendFunc)(GLenum, GLenum);
typedef void (APIENTRY *g_glClear)(GLbitfield);
typedef void (APIENTRY *g_glPopMatrix)(void);
typedef void (APIENTRY *g_glVertex3fv)(const GLfloat*);
typedef void (APIENTRY *g_wglSwapBuffers)(HDC);
//==================================================================================
#define SetupOrg(name) g_##name Org_##name = NULL;
SetupOrg(glBegin);
SetupOrg(glBlendFunc);
SetupOrg(glClear);
SetupOrg(glPopMatrix);
SetupOrg(glVertex3fv);
SetupOrg(wglSwapBuffers);
//==================================================================================
bool oglSubtractive = false;
bool bDrawingSmoke=false;
bool bDrawingScope=false;
bool bDrawnWorld = false;
bool bStartedDrawingEnts = false;
bool bSkyTex=false;
GLfloat curcol[4];
extern int displayCenterX, displayCenterY;
#define Check(name) if(!Org_##name) return;
//==================================================================================
void APIENTRY New_glBegin(GLenum mode)
{
	Check(glBegin);
	if(!bStartedDrawingEnts && bDrawnWorld && cvar.wall > 1)
	{
		if(mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN)
		{
			bStartedDrawingEnts = true;
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}
	if(cvar.whitewalls && mode != GL_TRIANGLES && GL_TRIANGLE_STRIP && GL_TRIANGLE_FAN && mode != GL_QUADS)
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	if(mode == GL_POLYGON && cvar.wall == 1)
	{
		float color[4];

		glGetFloatv(GL_CURRENT_COLOR, color);
		glDisable  (GL_DEPTH_TEST);
		glEnable  (GL_BLEND);
		glColor4f  (color[0], color[1], color[2], 0.5f );
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	if(mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN && cvar.wall == 1)
		glEnable(GL_DEPTH_TEST);
	glGetFloatv(GL_CURRENT_COLOR, curcol);
	if (mode==GL_QUADS && cvar.wall == 1)
		bSkyTex=true;
	else
		bSkyTex=false;
	if (mode==GL_QUADS)
	{		 
		if ((curcol[0]==curcol[1])&&(curcol[0]==curcol[2]) && (curcol[0]!=0.0) && (curcol[0]!=1.0) && 
			cvar.nosmash==1)
			bDrawingSmoke = true;	
	}
	else 
		bDrawingSmoke = false; 		
	if ( (curcol[0]==curcol[1] && curcol[0]==curcol[2]) && (curcol[0]==0.0 || curcol[0]==1.0) 
			&& me.inZoomMode && curcol[3]==1.0)
		bDrawingScope = true;	
	else
		bDrawingScope = false;	
	(Org_glBegin)(mode);
}
//==================================================================================
void APIENTRY New_glBlendFunc (GLenum sfactor, GLenum dfactor)
{
	Check(glBlendFunc);
	if(oglSubtractive) { (Org_glBlendFunc)(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); }
	else { (Org_glBlendFunc)(sfactor,dfactor); }
}
//==================================================================================
void APIENTRY New_glClear (GLbitfield mask)
{
	Check(glClear);
	if(cvar.wall == 1)
	{
		if(mask & GL_DEPTH_BUFFER_BIT)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			mask |= GL_COLOR_BUFFER_BIT;
		}
		mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
	}
	(Org_glClear)(mask);
}
//==================================================================================
void APIENTRY New_glPopMatrix (void)
{
	Check(glPopMatrix);
	cl_entity_t *pEnt = pStudio->GetCurrentEntity();
    if(pEnt && pEnt->player)
		{ Aimbot.CalculateHitbox(pEnt); vPlayers[pEnt->index].bDrawn = true; }
	(Org_glPopMatrix)();
}
//==================================================================================
void APIENTRY New_glVertex3fv (const GLfloat *v)
{
	bDrawnWorld = true;
	Check(glVertex3fv);
	if(cvar.fullbright)
	{
		cl_entity_s * pEnt = pStudio->GetCurrentEntity();
		pEnt->curstate.rendermode |= kRenderGlow;
	}
	if (bDrawingSmoke || bSkyTex) return;
	(Org_glVertex3fv)(v);
}
//==================================================================================
void APIENTRY New_wglSwapBuffers (HDC hDC)
{
	bDrawnWorld = false;
	bStartedDrawingEnts = false;
	Check(wglSwapBuffers);
	(Org_wglSwapBuffers)(hDC);
}
//==================================================================================
void HookGLFunctions(void)
{
#define Hook(name) Org_##name = (g_##name)DetourFunction((LPBYTE)DetourFindFunction("opengl32.dll", ###name), (LPBYTE)New_##name);
	Hook(glBegin);
	Hook(glBlendFunc);
	Hook(glClear);
	Hook(glPopMatrix);
	Hook(glVertex3fv);
	Hook(wglSwapBuffers);
}
//==================================================================================
void UnHookGLFunctions(void)
{
#define UnHook(name) DetourRemove((LPBYTE)Org_##name, (LPBYTE)New_##name);
	UnHook(glBegin);
	UnHook(glBlendFunc);
	UnHook(glClear);
	UnHook(glPopMatrix);
	UnHook(glVertex3fv);
	UnHook(wglSwapBuffers);
}
//==================================================================================