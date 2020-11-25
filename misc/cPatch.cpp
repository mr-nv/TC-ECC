/*	newhook - patch class
 *	Copyright (c) gC 2004
 *
 *  Desc: Holds Several Funcs to patch the memory
 *
 *  $Id: cpatch.cpp,v 1.0 2004/6/28 10:50:25 AM gC Exp $
 */

#pragma warning(disable:4311)
#pragma warning(disable:4312)

#include "cPatch.h"
#include "ade32.hpp" // replace ade with xde 1.01

cPatch * pPatch = new cPatch;



void cPatch::vPatchBytes(DWORD dwAddress,BYTE *bPatch)
{
	int			iLen = ( int ) strlen( ( const char* ) bPatch ); //can't deal with \x00 s
	DWORD		dwOldProtect=0;
	VirtualProtect((DWORD*)dwAddress,iLen,0x40,&dwOldProtect);
	memcpy((void*)dwAddress,(void*)bPatch,iLen);
	VirtualProtect((DWORD*)dwAddress,iLen,dwOldProtect,&dwOldProtect);
}

bool cPatch::bDataCompare(const BYTE* pData, const BYTE* bMask, const char* pszMask)
{
	for(;*pszMask;++pszMask,++pData,++bMask)
		if(*pszMask=='x' && *pData!=*bMask ) 
			return false;
	return (*pszMask)==0;
}

DWORD cPatch::dwFindPattern(DWORD dwAddress,DWORD dwLen,BYTE *bMask,char * pszMask)
{
	for(DWORD i=0; i < dwLen; i++)
		if( bDataCompare( (BYTE*)( dwAddress+i ),bMask,pszMask ) )
			return ( DWORD )( dwAddress + i );
	
	return 0;
}


DWORD cPatch::dwCopyRange(DWORD dwFrom,DWORD dwTo)
{
	DWORD dwOldProtect=0;
	DWORD dwLen		  = dwTo-dwFrom;
	VirtualProtect((DWORD*)dwFrom,dwLen,PAGE_EXECUTE_READWRITE,&dwOldProtect);
	void *pCopy = VirtualAlloc(0,dwLen,0x1000|0x2000,0x40);
	vWrapMemory(dwFrom,(DWORD)pCopy,dwLen); //fix calls/jmps
	VirtualProtect((DWORD*)dwFrom,dwLen,dwOldProtect,&dwOldProtect);
	return (DWORD)pCopy;
}


DWORD cPatch::dwCopyFunction(DWORD dwAddress,DWORD dwMin)
{
	DWORD		dwLen=0;
	
	while(dwLen < dwMin) dwLen += dwOpcodeLength(dwAddress+dwLen);
	
	void *pTrampoline = VirtualAlloc(0,dwLen+5,0x1000|0x2000,0x40);
	vWrapMemory(dwAddress,(DWORD)pTrampoline,dwLen);
	vCreateJMP ((DWORD)pTrampoline+dwLen,dwAddress+dwLen);
	return (DWORD)pTrampoline;	
}

DWORD cPatch::dwRedirectCall(DWORD dwAddress,DWORD dwNewCall)
{
	DWORD dwRetval = dwReconstructJMP(dwAddress);
	vCreateCall(dwAddress,dwNewCall);
	return dwRetval;
}

DWORD cPatch::dwRedirectFunction(DWORD dwAddress,DWORD dwNew,bool boAlgo)
{
	DWORD		dwOldProtect=0;
	DWORD		dwLen=0;
	DWORD       dwMin=(!boAlgo)?5:8;
	VirtualProtect((void*)dwAddress,16,PAGE_EXECUTE_READWRITE,&dwOldProtect);
	DWORD		pTrampoline	   = dwCopyFunction(dwAddress,dwMin);
	if(boAlgo)
	{
		*(WORD*) (dwAddress)   = '\xFF\xF6';	//push esi
	    *(BYTE*) (dwAddress+2) = '\x5E';		//pop  esi
		*(BYTE*) (dwAddress+3) = 0xE9;
		*(DWORD*)(dwAddress+4) = (DWORD)dwNew - (DWORD)dwAddress - 8;
	}
	else
	{
		vCreateJMP(dwAddress,dwNew);
	}
	VirtualProtect((void*)dwAddress,16,dwOldProtect,&dwOldProtect);
	return pTrampoline;
}

DWORD cPatch::dwReconstructJMP(DWORD dwAddress)
{
	DWORD  dwJMP = *(DWORD*)(dwAddress+1);
	dwJMP		+=  dwAddress+5;
	return dwJMP;
}

//todo: fix cond. jmps
void cPatch::vWrapMemory(DWORD dwAddress,DWORD dwNew,DWORD dwMinLen)
{
	DWORD dwLen =0;
	while(dwLen < dwMinLen)
	{
		DWORD dwOpcodeLen = dwOpcodeLength(dwAddress+dwLen);
		memcpy((void*)(dwNew+dwLen),(void*)(dwAddress+dwLen),dwOpcodeLen);
		if( *(BYTE*)(dwAddress+dwLen) == 0xE9 )
			vCreateJMP(dwNew+dwLen,dwReconstructJMP(dwAddress+dwLen));
		if( *(BYTE*)(dwAddress+dwLen) == 0xE8 )
			vCreateCall(dwNew+dwLen,dwReconstructJMP(dwAddress+dwLen));
		 dwLen += dwOpcodeLen;
	}
}


void cPatch::vCreateJMP(DWORD dwAddress,DWORD dwNew)
{
	DWORD		dwOldProtect=0;
	VirtualProtect((DWORD*)dwAddress,5,0x40,&dwOldProtect);
	*(BYTE*) (dwAddress)	= 0xE9;	//jmp
	*(DWORD*)(dwAddress+1)	= dwNew - (dwAddress+5);	//relative offset
	VirtualProtect((DWORD*)dwAddress,5,dwOldProtect,&dwOldProtect);
}

void cPatch::vCreateCall(DWORD dwAddress,DWORD dwNew)
{
	DWORD		dwOldProtect=0;
	VirtualProtect((DWORD*)dwAddress,5,0x40,&dwOldProtect);
	*(BYTE*) (dwAddress)	= 0xE8;	//call
	*(DWORD*)(dwAddress+1)	= dwNew - (dwAddress+5);
	VirtualProtect((DWORD*)dwAddress,5,dwOldProtect,&dwOldProtect);
}

DWORD cPatch::dwWriteProcess (DWORD dwPid, void * pAddr, void * pBuf, DWORD dwLen)
{
	DWORD dwOld, dwWriteLen;
	HANDLE hProcess = OpenProcess (PROCESS_VM_OPERATION|PROCESS_VM_WRITE, FALSE, dwPid);

	if ( !hProcess ){
		return 0;
	}
	if (!VirtualProtectEx (hProcess , pAddr, dwLen, PAGE_READWRITE, &dwOld)
	 || !WriteProcessMemory(hProcess, pAddr, pBuf,  dwLen, &dwWriteLen)
	 || !VirtualProtectEx (hProcess , pAddr, dwLen, dwOld, &dwOld) ){
	
		 dwWriteLen = 0;
	 }
	CloseHandle (hProcess);
    return dwWriteLen;
}


DWORD cPatch::dwReadProcess (DWORD dwPid, void * pAddr, void * pBuf, DWORD dwLen)
{
	DWORD dwOld, dwReadLen;
	HANDLE hProcess = OpenProcess (PROCESS_VM_OPERATION|PROCESS_VM_READ, FALSE, dwPid);

	if ( !hProcess ){
		return 0;
	}
	if (!VirtualProtectEx (hProcess, pAddr, dwLen, PAGE_READONLY, &dwOld)
	 || !ReadProcessMemory(hProcess, pAddr, pBuf, dwLen, &dwReadLen)
	 || !VirtualProtectEx (hProcess, pAddr, dwLen, dwOld, &dwOld) ){
	
		 dwReadLen = 0;
	 }
	CloseHandle (hProcess);
    return dwReadLen;
}

DWORD cPatch::dwGetPID(char *szName)
{
	HANDLE			hProc;
	PROCESSENTRY32  hProcEntry;
	hProcEntry.dwSize = sizeof(PROCESSENTRY32);
	hProc			  = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if( hProc == INVALID_HANDLE_VALUE ){
		return 0;
	}
	if( Process32First(hProc, &hProcEntry) ){
		while( Process32Next(hProc, &hProcEntry) ){
			if( !stricmp(hProcEntry.szExeFile,szName) ){
				CloseHandle( hProc );
				return hProcEntry.th32ProcessID;
			}
		}
	}
	CloseHandle( hProc );
	return 0;
}

DWORD cPatch::dwOpcodeLength(DWORD dwAddress)
{
	disasm_struct diza;
	disasm((BYTE*)dwAddress,&diza);
	if ( ( diza.disasm_flag == C_ERROR ) || (diza.disasm_flag & C_BAD) )
		return 1; 
	return diza.disasm_len;
}

cPatch::cPatch()
{
}

cPatch::~cPatch()
{
}