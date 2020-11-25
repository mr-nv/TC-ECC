/*	newhook - engine hooks
 *	Copyright (c) gC 2004
 *
 *  Desc: Hl-specific engine hooks
 *
 *  $Id: engine.cpp,v 2.0 2004/7/20 12:30:04 AM gC Exp $
 */


#include "engine.h"
#include "cPatch.h"

//#define PROCKEY			0x7A32BC85



DWORD g_dwHW_Base;		//hw.dll code base
DWORD g_dwHW_Len;		//hw.dll code len



DWORD g_dwOrigLoadLibraryA;		//stores address to backuped instructions of LoadLib
DWORD g_dwReturn;				//real HW_DecodeDll return address
DWORD g_dwCL_ParseSound_Begin;
DWORD g_dwCL_ParseSound_Call;
DWORD g_dwCL_ParseSound_End;
DWORD g_dwSvc_Table;
DWORD g_dwTimeOfs;

DWORD g_dwJmp;
DWORD g_dwCall;
DWORD g_dwPtr;


HL_DecodeFunc_t			pHL_DecodeFunc;
sound_t					pSound;
svc_entry_s*			pSvcDispatch;
extern r_studio_interface_s**  ppinterface;
BYTE					g_FixValue[5];
DWORD					g_dwFixaddr= NULL;
double*					g_globalTime;

void FixupCRCValue()
{
	memcpy((void*)g_dwFixaddr,(void*)g_FixValue,sizeof(g_FixValue));
}

DWORD dwTraceStack( DWORD dwEsp , DWORD dwAddress )
{
	DWORD dwPos	   = 0xC; //skip args
	while ( true )
	{
		if( (dwEsp + dwPos) > ( dwEsp + 0x1A0 ) )
			break;

		if( *(DWORD*)( dwEsp + dwPos ) == dwAddress )
			return dwPos;

		dwPos++;
	}
	return 0;
}



__inline void    Decrypt(byte * pOfs, int iLen)
{
        byte xor = 0x57;
        for (int i = 0x44; i < iLen; i++) 
        {
                byte newdata = pOfs[i]^xor;
                pOfs[i] = newdata;
                xor += newdata + 0x57;
        }
}

__inline void Encrypt( BYTE *pOfs, int iLen )
{
        BYTE xor = 0x57;
        BYTE bDecoded;
        for( int i = 0x44; i < iLen; i++ )
        {
                bDecoded = pOfs[i] ^ xor;
                xor += pOfs[i] + 0x57;
                pOfs[i] = bDecoded;
        }
}

DWORD dwHook_SvcEntry( DWORD dwBegin,	 // begin of the parser
					   DWORD dwEnd,		 // end of the parser
					   DWORD dwCall,	 // call to the original handler
					   DWORD dwHandler,  // our handler
					   int iPos			 // svc_table element ##x
				)
{	
	//Make a copy of the original Handler
	DWORD dwMy_Handler			= pPatch->dwCopyRange( dwBegin , dwEnd );
	//Hook Svc_Table ( hw.dll )
	pSvcDispatch[iPos].pfnHandler = (DWORD)dwMy_Handler;
	//Redirect Call
	return pPatch->dwRedirectCall((dwMy_Handler+dwCall),dwHandler);
}

void playerSound(int index, const float*const origin, const char*const sample);
void S_StartDynamicSound( int entnum, int entchannel, char* sample, float* origin, float volume, float attenuation, int timeofs , int pitch )
{
	if (entnum >= 1 && entnum < 32 && sample && sample[0] && origin)
		playerSound(entnum,origin,sample);


	(*pSound)(entnum,entchannel,sample,origin,volume,attenuation,timeofs,pitch);
}


__inline void FindOffsets()
{
	g_dwReturn				= pPatch->dwFindPattern( g_dwHW_Base,g_dwHW_Len,(BYTE*)"\x57\x50\x51\x53\xE8","xxxxx");
	g_dwCL_ParseSound_Begin = pPatch->dwFindPattern( g_dwHW_Base,g_dwHW_Len,(BYTE*)"\x83\xEC\x68\x53\x55\x56\x57\x68\x00\x00\x00\x00\xE8","xxxxxxxx????x");
	g_dwCL_ParseSound_Call  = pPatch->dwFindPattern( g_dwHW_Base,g_dwHW_Len,(BYTE*)"\x50\x57\xE8\x00\x00\x00\x00\x83\xC4\x20\x5F\x5E\x5D\x5B\x83\xC4\x68\xC3","xxx????xxxxxxxxxxx");
	g_dwCL_ParseSound_End	= pPatch->dwFindPattern( g_dwHW_Base,g_dwHW_Len,(BYTE*)"\x50\x57\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x0C\x5F\x5E\x5D\x5B\x83\xC4\x68\xC3","xxx????x????xxxxxxxxxxx");
	g_dwSvc_Table			= pPatch->dwFindPattern( g_dwHW_Base,g_dwHW_Len,(BYTE*)"\x51\x53\x56\x57\x68\xFF\xFF\xFF\xFF\x33\xDB\xE8\xFF\xFF\xFF\xFF\x83\xC4\x04\x33\xF6\xBF","xxxxx????xxx????xxxxxx");
	g_dwTimeOfs				= pPatch->dwFindPattern( g_dwHW_Base,g_dwHW_Len,(BYTE*)"\x51\xDD\x05\x00\x00\x00\x00\xD9\x5C\x24\x00\xD9\x44\x24\x00\x59","xxx????xxxxxxxxx");

	if( !g_dwReturn )
		g_dwReturn = pPatch->dwFindPattern( g_dwHW_Base , g_dwHW_Len , (BYTE*)"\x83\xC4\x40\x8B\xF8","xxxxx");
	else
		g_dwReturn += 9;

	//correct values
	g_dwTimeOfs				+= 1;
	g_dwSvc_Table			+= 0x15;
	g_dwCL_ParseSound_End	+= 0x16;
	g_dwCL_ParseSound_Call	+= 2;
	g_dwCL_ParseSound_Call	-= g_dwCL_ParseSound_Begin;
	g_globalTime			= (double*)(*(DWORD*)(g_dwTimeOfs  + 2));
	//set locations
	pSvcDispatch			  = (svc_entry_s*)(*(DWORD*)(g_dwSvc_Table + 1 ) - 4 );
	
}

DWORD HL_DecodeDll( BYTE* pData, char **ppszDllName, DWORD *pdwExportTable, int iSize ) 
{
	header_t * hdr = (header_t* )( pData + 0x44);
	segment_t* seg = (segment_t*)( pData + 0x5C);
	

	//retrieve hw.dll base & iLen
	Decrypt( pData , iSize );
		g_dwHW_Base	 	 = seg->baseaddress;
		g_dwHW_Len	 	 = seg->memorysize;
	Encrypt ( pData , iSize );

	DWORD dwRetval =  (*pHL_DecodeFunc) ( pData , ppszDllName , pdwExportTable , iSize );

	
	//retrieves all the offsets
	FindOffsets();
	//Hook S_StartDynamicSound caller
	pSound = (sound_t)dwHook_SvcEntry( g_dwCL_ParseSound_Begin ,
									   g_dwCL_ParseSound_End ,
									   g_dwCL_ParseSound_Call ,
									   (DWORD)S_StartDynamicSound,
									   6 );
	return dwRetval;
}


__declspec(naked)void naked_Pre_ST_DecodeDLL()
{
	_asm   push		edi
	_asm   call		dword ptr ds:[ g_dwCall ]
	_asm   pop		ecx
	_asm   mov		ebx,eax
	_asm   mov		ecx,dword ptr ds:[ g_dwPtr ]
	_asm   mov		ecx,[ ecx ]
	_asm   push		esi
	_asm   push		edi
	_asm   push		ebx
	_asm   mov		eax,dword ptr ds:[ ecx ]
	_asm   call		dword ptr ds:[ eax + 0x54 ]
	_asm   push		edi
	_asm   push		dword ptr ss:[ esp + 0x1C ]
	_asm   push		dword ptr ss:[ esp + 0x1C ]
	_asm   push		ebx
	_asm   call		HL_DecodeDll
	_asm   jmp		dword ptr ds:[ g_dwJmp ]
}
__declspec(naked)void naked_Pre_CS_DecodeDLL()
{
	_asm   mov		ecx,dword ptr ss:[ esp + 0x48 ]
	_asm   mov		edx,dword ptr ss:[ esp + 0x44 ]
	_asm   push		edi
	_asm   push		ecx
	_asm   push		edx
	_asm   push		ebx
	_asm   call		HL_DecodeDll
	_asm   jmp		[ g_dwJmp ]
}

void PatchEngine ( )
{	
	DWORD dwDecode;
	PIMAGE_DOS_HEADER     pDosHdr = (PIMAGE_DOS_HEADER)	   ( (DWORD) GetModuleHandle ( 0 ) );
	PIMAGE_NT_HEADERS     pNtHdr  = (PIMAGE_NT_HEADERS)	   ( (BYTE*) ( pDosHdr ) + pDosHdr->e_lfanew );
	PIMAGE_SECTION_HEADER pSecHdr = (PIMAGE_SECTION_HEADER)( pNtHdr + 1 );

	for (int i=0; i < pNtHdr->FileHeader.NumberOfSections;i++)
	{
		if ((pSecHdr->Characteristics & IMAGE_SCN_CNT_CODE ) == IMAGE_SCN_CNT_CODE )
		{
		dwDecode = pPatch->dwFindPattern( (DWORD)(pDosHdr)+pSecHdr->VirtualAddress ,  pSecHdr->SizeOfRawData, 
				(BYTE*)"\x55\x8B\xEC\x51\x83\x65\xFC\x00\x53\x8B\x5D\x08\x57\x6A\x44\xB2\x57","xxxxxxxxxxxxxxxxx");
			if(!dwDecode)
			{
				dwDecode = pPatch->dwFindPattern( (DWORD)(pDosHdr)+pSecHdr->VirtualAddress, pSecHdr->SizeOfRawData,
					(BYTE*)"\x51\x53\x55\x8B\x6C\x24\x10\x56\x8B\x74\x24\x20\x57","xxxxxxxxxxxxx");
			}
			if( dwDecode )
				break; 
		}
		pSecHdr++;
	}
	

	if( !dwDecode )
		return;


	if( ( dwDecode & 0x0FF00000) == 0x01400000 ) //steam
	{
		g_dwJmp			= ( dwDecode-0x1D );
		g_dwCall		= pPatch->dwReconstructJMP( ( dwDecode - 0x42 ) );
		g_dwPtr			= *(DWORD*)( dwDecode-0x38);
		g_dwFixaddr		= dwDecode-0x43;
		memcpy((void*)g_FixValue,(void*)g_dwFixaddr,sizeof(g_FixValue));
		pPatch->vCreateJMP( ( dwDecode-0x43 ),(DWORD)naked_Pre_ST_DecodeDLL);
	}
	else
	{
		
		g_dwJmp		= (dwDecode-0x21);
		g_dwFixaddr	= dwDecode-0x32;
		memcpy((void*)g_FixValue,(void*)g_dwFixaddr,sizeof(g_FixValue));
		pPatch->vCreateJMP( ( dwDecode-0x32 ),( DWORD )naked_Pre_CS_DecodeDLL );
	}
	
	pHL_DecodeFunc = (HL_DecodeFunc_t)(dwDecode);
}