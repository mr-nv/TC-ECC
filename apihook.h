#include <windows.h>
#pragma warning(disable:4311)
#pragma warning(disable:4312)

void *InterceptDllCall( HMODULE hModule, char *szDllName, char *szFunctionName, DWORD pNewFunction );