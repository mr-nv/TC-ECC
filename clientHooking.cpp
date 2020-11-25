//==================================================================================
#include "clientHooking.h"
#include "client.h"
#include <windows.h>
//==================================================================================
CLIENT gClient				= { NULL };
DWORD dwClientPtr					= 0x01A179F0;
bool bClientActive					= FALSE;
bool bEngineActive						= FALSE;
//==================================================================================
// Client functions jumpgates...
//==================================================================================
DWORD retaddress;

__declspec( naked )void Gateway2_V_CalcRefdef( void )
{
	__asm
	{
		call PostV_CalcRefdef;
		jmp retaddress;
	}
}

DWORD CalcRef = (DWORD)&Gateway2_V_CalcRefdef;
__declspec( naked )void Gateway1_V_CalcRefdef( void )
{
	
	__asm
	{
		push esi;
		mov esi,dword ptr ss:[esp+0x10];
		push esi;
		call PreV_CalcRefdef;
		add esp,4;
		mov esi,dword ptr ss:[esp+0x0c];
		mov retaddress,esi;
		push CalcRef;
		pop esi;
		mov dword ptr ss:[esp+0x0c],esi;
		pop esi;
		ret;
	}
}

__declspec( naked )void Gateway2_CL_CreateMove( void )
{
	__asm
	{
		call CL_CreateMove;
		jmp retaddress;
	}
}

DWORD CreateMove = (DWORD)&Gateway2_CL_CreateMove;
__declspec( naked )void Gateway1_CL_CreateMove( void )
{
	
	__asm
	{
		push esi;
		mov esi,dword ptr ss:[esp+0x28];
		mov retaddress,esi;
		push CreateMove;
		pop esi;
		mov dword ptr ss:[esp+0x28],esi;
		pop esi;
		ret;
	}
}

__declspec( naked )void Gateway2_HUD_Redraw( void )
{
	__asm
	{
		call HUD_Redraw;
		jmp retaddress;
	}
}

DWORD Redraw= (DWORD)&Gateway2_HUD_Redraw;
__declspec( naked )void Gateway1_HUD_Redraw( void )
{
	__asm
	{
		push esi;
		mov esi,dword ptr ss:[esp+0x10];
		mov retaddress,esi;
		push Redraw
		pop esi;
		mov dword ptr ss:[esp+0x10],esi;
		pop esi;
		ret;
	}
}

__declspec( naked )void Gateway2_HUD_PostRunCmd( void )
{
	__asm
	{
		call HUD_PostRunCmd;
		jmp retaddress;
	}
}

DWORD PostRunCmd= (DWORD)&Gateway2_HUD_PostRunCmd;
__declspec( naked )void Gateway1_HUD_PostRunCmd( void )
{
	__asm
	{
		push esi;
		mov esi,dword ptr ss:[esp+0x38];
		mov retaddress,esi;
		push PostRunCmd
		pop esi;
		mov dword ptr ss:[esp+0x38],esi;
		pop esi;
		ret;
	}
}

__declspec( naked )void Gateway2_HUD_PlayerMove( void )
{
	__asm
	{
		call HUD_PlayerMove;
		jmp retaddress;
	}
}

DWORD PlayerMove = (DWORD)&Gateway2_HUD_PlayerMove;
__declspec( naked )void Gateway1_HUD_PlayerMove( void )
{
	
	__asm
	{
		push esi;
		mov esi,dword ptr ss:[esp+0x10];
		mov retaddress,esi;
		push PlayerMove
		pop esi;
		mov dword ptr ss:[esp+0x10],esi;
		pop esi;
		ret;
	}
}

__declspec( naked )void Gateway2_HUD_Init( void )
{
	__asm
	{
		call HUD_Init;
		jmp retaddress;
	}
}

DWORD Init = (DWORD)&Gateway2_HUD_Init;
__declspec( naked )void Gateway1_HUD_Init( void )
{
	
	__asm
	{
		push esi;
		mov esi,dword ptr ss:[esp+0x08];
		mov retaddress,esi;
		push Init
		pop esi;
		mov dword ptr ss:[esp+0x08],esi;
		pop esi;
		ret;
	}
}
extern int AddEntResult;
__declspec( naked )void Gateway2_HUD_AddEntity( void )
{
	__asm
	{
		mov AddEntResult, eax;
		call HUD_AddEntity;
		mov eax, AddEntResult;
		jmp retaddress;
	}
}

DWORD AddEnt = (DWORD)&Gateway2_HUD_AddEntity;
__declspec( naked )void Gateway1_HUD_AddEntity( void )
{
	
	__asm
	{
		push esi;
		mov esi,dword ptr ss:[esp+0x14];
		mov retaddress,esi;
		push AddEnt
		pop esi;
		mov dword ptr ss:[esp+0x14],esi;
		pop esi;
		ret;
	}
}
extern int KeyEventResult;
__declspec( naked )void Gateway2_HUD_Key_Event( void )
{
	__asm
	{
		mov KeyEventResult, eax;
		call HUD_Key_Event;
		mov eax, KeyEventResult;
		jmp retaddress;
	}
}

DWORD KeyEvent = (DWORD)&Gateway2_HUD_Key_Event;
__declspec( naked )void Gateway1_HUD_Key_Event( void )
{
	
	__asm
	{
		push esi;
		mov esi,dword ptr ss:[esp+0x14];
		mov retaddress,esi;
		push KeyEvent
		pop esi;
		mov dword ptr ss:[esp+0x14],esi;
		pop esi;
		ret;
	}
}

__declspec(naked) void NullStub()
{
	_asm ret;
}

int iUpdateResult;
__declspec( naked ) void JumpGate_HUD_UpdateClientData() // Thank you Patrick for
 {														 // giving me the Client-
 	_asm	mov [iUpdateResult],eax						 // Gateway for this Function
 	_asm	call HUD_UpdateClientData
 	_asm	mov eax,[iUpdateResult]
 	_asm	jmp [retaddress]
 }


DWORD dwUpdateClD = (DWORD)&JumpGate_HUD_UpdateClientData;
__declspec( naked ) void GateWay_HUD_UpdateClientData()
 {
 	_asm	push esi
 	_asm	mov esi,[esp+0x10]
 	_asm	mov [retaddress],esi
 	_asm	push [dwUpdateClD]
 	_asm	pop esi
 	_asm	mov [esp+0x10],esi
 	_asm	pop esi
 	_asm	retn
}

//==================================================================================
//==================================================================================
bool ActivateClient()
{
	if ( !IsBadReadPtr( ( LPCVOID )dwClientPtr, sizeof DWORD ) )
	{
		if ( *( ( DWORD* )dwClientPtr ) != NULL )
		{
			DWORD dwClientRealPtr;

			__asm {
				push eax;
				push edx;
				xor eax, eax;
				xor edx, edx;

				mov edx, dwClientPtr;
				mov eax, [edx];
				mov dwClientRealPtr, eax;

				pop edx;
				pop eax;
			}

			// Copy client  to local structure
			memcpy( &gClient, ( LPVOID )dwClientRealPtr, sizeof CLIENT );

			// Hook client  functions
			gClient.HUD_Init =						( HUD_INIT_FUNCTION )			&Gateway1_HUD_Init;
			gClient.HUD_Redraw =					( HUD_REDRAW_FUNCTION )			&Gateway1_HUD_Redraw;
			gClient.HUD_PlayerMove =				( HUD_CLIENTMOVE_FUNCTION)		&Gateway1_HUD_PlayerMove;
			gClient.CL_CreateMove =					( HUD_CL_CREATEMOVE_FUNCTION )	&Gateway1_CL_CreateMove;
			gClient.V_CalcRefdef =					( HUD_V_CALCREFDEF_FUNCTION )	&Gateway1_V_CalcRefdef;
			gClient.HUD_AddEntity =					( HUD_ADDENTITY_FUNCTION )		&Gateway1_HUD_AddEntity;
			gClient.HUD_PostRunCmd =				( HUD_POSTRUNCMD_FUNCTION )		&Gateway1_HUD_PostRunCmd;
			gClient.HUD_Key_Event =					( HUD_KEY_EVENT_FUNCTION )		&Gateway1_HUD_Key_Event;
			gClient.HUD_UpdateClientData =			( HUD_UPDATECLIENTDATA_FUNCTION)&GateWay_HUD_UpdateClientData;
			

			// Store new  address into client  pointer
			DWORD dwClientNewPtr = ( DWORD ) &gClient;

			__asm {
				push eax;
				push edx;
				xor eax, eax;
				xor edx, edx;
				
				mov edx, dwClientPtr;
				mov eax, dwClientNewPtr;
				mov [edx], eax;

				pop edx;
				pop eax;
			}


			bClientActive = TRUE;
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		return FALSE;
	}
	return TRUE;
}



//==================================================================================
// Copy enginefuncs_s struct to local gEngfuncs and setup engine hooks
//==================================================================================
bool ActivateEngine()
{
	if( pEngfuncs->pfnHookUserMsg && pEngfuncs->pfnHookEvent )
	{
		
		memcpy( &gEngfuncs, pEngfuncs, sizeof( cl_enginefunc_t ) );
		if( pStudio->GetModelByIndex )
		{
			memcpy( &IEngineStudio, pStudio, sizeof( IEngineStudio ) );
		} else {
			return FALSE;
		}

		pEngfuncs->pfnSPR_Load =				&SPR_Load;
		pEngfuncs->pfnSPR_Set =					&SPR_Set;
		pEngfuncs->pfnSPR_Draw =				&SPR_Draw;
		pEngfuncs->pfnSPR_DrawHoles =			&SPR_DrawHoles;
		pEngfuncs->pfnSPR_DrawAdditive =		&SPR_DrawAdditive;
		pEngfuncs->pfnDrawCharacter =			&DrawCharacter;
		pEngfuncs->pfnDrawConsoleString =		&DrawConsoleString;
		pEngfuncs->pfnFillRGBA =				&FillRGBA;

		pEngfuncs->pfnHookUserMsg =				&HookUserMsg;

#define Echo gConsole.echo
		gGui.InitFade();
		Echo("Rumpelstielzchen by Sruh and h1web :");
		Echo("Homepage: http://www.overclock.ch");
		Echo("Chat: irc.rizon.net #ecc");
		cvar.Init();
		cvar.Load();

		bEngineActive = TRUE;

	} else {
		return FALSE;
	}
	return TRUE;
}
//==================================================================================
