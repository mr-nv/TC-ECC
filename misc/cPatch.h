
#ifndef CPATCH_H
#define CPATCH_H

#include <windows.h>
#include <tlhelp32.h>


class cPatch
{

	public: 
			 cPatch();
			~cPatch();
			DWORD dwCopyFunction	(DWORD,DWORD);
			DWORD dwCopyRange		(DWORD,DWORD);
			DWORD dwRedirectFunction(DWORD,DWORD,bool = false);
			DWORD dwWriteProcess	(DWORD, void *,void *,DWORD);
			DWORD dwReadProcess		(DWORD, void *,void *,DWORD);
			DWORD dwRedirectCall	(DWORD,DWORD);
			DWORD dwGetPID			(char *);
			DWORD dwFindPattern		(DWORD,DWORD,BYTE *,char *);
			void  vCreateJMP		(DWORD,DWORD);		
			void  vCreateCall		(DWORD,DWORD);
			void  vPatchBytes		(DWORD,BYTE *);
			DWORD dwReconstructJMP	(DWORD);
	private:
			bool  bDataCompare		(const BYTE*, const BYTE*, const char* );
			void  vWrapMemory		(DWORD,DWORD,DWORD);
			DWORD dwOpcodeLength	(DWORD);
};
extern cPatch *pPatch;
#endif