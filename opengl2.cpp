//==================================================================================
// opengl hoo
//==================================================================================
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"GLu32.lib")
#pragma comment(lib,"GLaux.lib")
#pragma warning(disable:4715)
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>

bool oglSubtractive = false;
//==================================================================================
//	create offset stores
//==================================================================================
#define PTR_CREATE(glPtr) FARPROC pOrig_##glPtr;
//---
PTR_CREATE(glBegin)
PTR_CREATE(glBlendFunc)
PTR_CREATE(glClear)
PTR_CREATE(glEnable)
PTR_CREATE(glEnd)
PTR_CREATE(glPopMatrix)
PTR_CREATE(glPushMatrix)
PTR_CREATE(glVertex3f)
PTR_CREATE(glVertex3fv)
PTR_CREATE(glViewport)
PTR_CREATE(wglSwapBuffers)

//==================================================================================
//	store offsets
//==================================================================================
#define PTR_STORE(function) \
	pOrig_##function = (FARPROC)(GetProcAddress(GetModuleHandle("opengl32.dll"),#function)); \
	if (*pOrig_##function == NULL) \
		pResult = false;
//---
bool StoreOffsets( void )
{
	bool pResult = true;
	//---
	PTR_STORE(glBegin)			//else logme("Found glBegin at 0x%x"		,*pOrig_glBegin);
	PTR_STORE(glBlendFunc)		//else logme("Found glBlendFunc at 0x%x"	,*pOrig_glBlendFunc);
	PTR_STORE(glClear)			//else logme("Found glClear at 0x%x"		,*pOrig_glClear);
	PTR_STORE(glEnable)			//else logme("Found glEnable at 0x%x"		,*pOrig_glEnable);
	PTR_STORE(glEnd)			//else logme("Found glEnd at 0x%x"			,*pOrig_glEnd);
	PTR_STORE(glPopMatrix)		//else logme("Found glPopMatrix at 0x%x"	,*pOrig_glPopMatrix);
	PTR_STORE(glPushMatrix)		//else logme("Found glPushMatrix at 0x%x"	,*pOrig_glPushMatrix);
	PTR_STORE(glVertex3f)		//else logme("Found glVertex3f at 0x%x"		,*pOrig_glVertex3f);
	PTR_STORE(glVertex3fv)		//else logme("Found glVertex3fv at 0x%x"	,*pOrig_glVertex3fv);
	PTR_STORE(glViewport)		//else logme("Found glViewport at 0x%x"		,*pOrig_glViewport);
	PTR_STORE(wglSwapBuffers)	//else logme("Found wglSwapBuffers at 0x%x"	,*pOrig_wglSwapBuffers);
	//---
	return pResult;
}

//==================================================================================
//	Hooked OpenGL Functions
//==================================================================================
void __stdcall pHooked_glBegin ( GLenum mode )
{
	//---

	/*
		<your code here>
							*/

	//---
	__asm
	{
		push mode
		call dword ptr [pOrig_glBegin]
	}
}

//==================================================================================
void __stdcall pHooked_glBlendFunc ( GLenum sfactor,  GLenum dfactor )
{
	//---

	/*
		<your code here>
							*/	

	//---
  if (oglSubtractive)
	{
		__asm
		{ 
			push 0x0303 // GL_ONE_MINUS_SRC_ALPHA
			push 0x0302	// GL_SRC_ALPHA
			call dword ptr [pOrig_glBlendFunc]
		}
	}
	else
	{
		__asm
		{ 
			push dfactor
			push sfactor
			call dword ptr [pOrig_glBlendFunc]
		}
	}
}

//==================================================================================
void __stdcall pHooked_glClear ( GLbitfield mask )
{
	//---

	/*
		<your code here>
							*/

	//---
    __asm
	{
		push mask
		call dword ptr [pOrig_glClear]
	}
}

//==================================================================================
void __stdcall pHooked_glEnable ( GLenum cap )
{
	//---

	/*
		<your code here>
							*/	

	//---
    __asm
	{
		push cap
		call dword ptr [pOrig_glEnable]
	}
}

//==================================================================================
void __stdcall pHooked_glEnd ( void )
{
	//---

	/*
		<your code here>
							*/	

	//---
    __asm
	{
		call dword ptr [pOrig_glEnd]
	}
}

//==================================================================================
void __stdcall pHooked_glPopMatrix ( void )
{
	//---

	/*
		<your code here>
							*/	

	//---
    __asm
	{
		call dword ptr [pOrig_glPopMatrix]
	}
}

//==================================================================================
void __stdcall pHooked_glPushMatrix ( void )
{
	//---

	/*
		<your code here>
							*/	

	//---
    __asm
	{
		call dword ptr [pOrig_glPushMatrix]
	}
}

//==================================================================================
void __stdcall pHooked_glVertex3f ( GLfloat x,  GLfloat y,  GLfloat z )
{
	//---

	/*
		<your code here>
							*/

	//---
    __asm
	{
		push z
		push y
		push x
		call dword ptr [pOrig_glVertex3f]
	}
}

//==================================================================================
void __stdcall pHooked_glVertex3fv ( const GLfloat *v )
{
	//---

	/*
		<your code here>
							*/

	//---
    __asm
	{
		push v
		call dword ptr [pOrig_glVertex3fv]
	}
}

//==================================================================================
void __stdcall pHooked_glViewport ( GLint x,  GLint y,  GLsizei width,  GLsizei height )
{
	//---

	/*
		<your code here>
							*/	

	//---
    __asm
	{
		push height
		push width
		push y
		push x
		call dword ptr [pOrig_glViewport]
	}
}

//==================================================================================
void __stdcall pHooked_wglSwapBuffers ( HDC hDC )
{
	//---

	/*
		<your code here>
							*/	

	//---
    __asm
	{
		push hDC
		call dword ptr [pOrig_wglSwapBuffers]
	}
}

//==================================================================================
//	Hooking the gl functions
//==================================================================================
#define HOOK_APPLY(func_name) if(!lstrcmp(lpProcName,#func_name)) { \
				*pProc = (FARPROC) &pHooked_##func_name; \
				return; }
//---
void ApplyOpenGLHook (FARPROC* pProc,LPCSTR lpProcName)
{
	HOOK_APPLY(wglSwapBuffers)
	HOOK_APPLY(glBegin)
	HOOK_APPLY(glBlendFunc)
	HOOK_APPLY(glClear)
	HOOK_APPLY(glEnable)
	HOOK_APPLY(glEnd)
	HOOK_APPLY(glPopMatrix)
	HOOK_APPLY(glPushMatrix)
	HOOK_APPLY(glVertex3f)
	HOOK_APPLY(glVertex3fv)
	HOOK_APPLY(glViewport)
}
