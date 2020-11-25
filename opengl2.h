//==================================================================================
//
//	panzer OpenGL Hack Base - Version: 1.10 (03/19/2004) - opengl.h
//	
//	Credits:	Xen (his method of hooking wglSwapBuffers inspired me making this)
//				System (apihook.cpp, apihook.h - providing 'InterceptDllCall(..)')
//				Pa7r1ck (hint with getprocaddress, Jordon told me :p)
//
//				(see opengl.cpp for more info)
//
//==================================================================================
#ifndef _OPENGL_
#define _OPENGL_


bool StoreOffsets( void );
void ApplyOpenGLHook (FARPROC* pProc,LPCSTR lpProcName);
extern bool oglSubtractive;

#endif