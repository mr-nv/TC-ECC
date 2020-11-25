//==================================================================================
#include <windows.h>
#include <detours.h>
#include "clientHooking.h"
#include "cvar.h"
//==================================================================================
typedef bool (WINAPI *g_QPC)(LARGE_INTEGER*);
g_QPC QueryPerformanceCounterOrg = NULL;
//==================================================================================
char hackdir[256];
//==================================================================================
void HookGLFunctions(void);
void UnHookGLFunctions(void);
//==================================================================================
bool WINAPI QueryPerformanceCounterNew(LARGE_INTEGER* pLI)
{
	if ( !bClientActive )
		ActivateClient( );
	if ( !bEngineActive )
		ActivateEngine( );
	return (QueryPerformanceCounterOrg)(pLI);
}
//==================================================================================
void PatchEngine ( );
void HookFunctions(void)
{
	//patch the engine
	PatchEngine();
	QueryPerformanceCounterOrg = (g_QPC)DetourFunction((LPBYTE)DetourFindFunction("Kernel32.dll", "QueryPerformanceCounter"), (LPBYTE)QueryPerformanceCounterNew);
	HookGLFunctions();
}
//==================================================================================
void UnHookFunctions(void)
{
	DetourRemove((LPBYTE)QueryPerformanceCounterOrg, (LPBYTE)QueryPerformanceCounterNew);
	UnHookGLFunctions();
}
//==================================================================================
bool WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		GetModuleFileName((HINSTANCE)hinstDLL,hackdir,255);
		char* pos = hackdir+strlen(hackdir);
		while(pos>=hackdir && *pos!='\\') --pos;
			pos[1]=0;
		HookFunctions();
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
		UnHookFunctions();
	return true;
}
//==================================================================================