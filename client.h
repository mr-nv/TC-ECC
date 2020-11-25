#ifndef _CLIENT_H_
#define _CLIENT_H_

//==================================================================================
// Disable typecast warnings
//==================================================================================
#pragma warning( disable :4311 )
#pragma warning( disable :4312 )
#pragma warning( disable :4313 )
#pragma warning( disable :4018 )
#pragma warning( disable :4800 )
#pragma warning( disable :4244 )
#pragma warning( disable :4101 )
#pragma warning( disable :4715 )
#pragma warning( disable :4305 )

#include <windows.h>
#include <gl/gl.h>
#include <detours.h>

#include "engine/wrect.h"
#include "engine/cl_dll.h"
#include "engine/cdll_int.h"
#include "engine/const.h"
#include "engine/progdefs.h"
#include "engine/eiface.h"
#include "engine/edict.h"
#include "engine/studio_event.h"
#include "engine/entity_types.h"
#include "engine/r_efx.h"
#include "engine/pmtrace.h"
#include "common/ref_params.h"
#include "common/screenfade.h"
#include "common/event_api.h"
#include "common/com_model.h"
#include "misc/parsemsg.h"
#include "misc/sprites.h"
#include "misc/r_studioint.h"
#include "engine/triangleapi.h"
#include "engine/pm_defs.h"
#include "engine/studio.h"

#include "msghook.h"
#include "drawing.h"
#include "interpreter.h"
#include "aimbot.h"
#include "color.h"
#include "players.h"
#include "radar.h"
#include "gui.h"
#include "console.h"
#include "menu.h"
#include "weaponslist.h"
#include "nospread.h"

extern char hackdir[256];
extern bool oglSubtractive;

//==================================================================================
// Create references to structs so that other files using this header can utilize them correctly...
//==================================================================================
extern struct cl_enginefuncs_s gEngfuncs;
extern cl_enginefuncs_s* pEngfuncs;
extern engine_studio_api_s* pStudio;
extern engine_studio_api_t IEngineStudio;

//==================================================================================
// Client Function Prototypes
//==================================================================================
void HUD_Init ( void );
void HUD_Redraw ( float x, int y );
void HUD_PlayerMove ( struct playermove_s *ppmove, qboolean server );
void CL_CreateMove ( float frametime, struct usercmd_s *cmd, int active );
void PreV_CalcRefdef ( struct ref_params_s *pparams );
void PostV_CalcRefdef ( struct ref_params_s *pparams );
void HUD_AddEntity ( int type, struct cl_entity_s *ent, const char *modelname );
void HUD_PostRunCmd ( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
int HUD_Key_Event ( int eventcode, int keynum, const char *pszCurrentBinding );
void HUD_UpdateClientData(client_data_t *cdata, float flTime);

//==================================================================================
// Engine Function Prototypes
//==================================================================================
HSPRITE SPR_Load ( const char *szPicName );
void SPR_Set ( HSPRITE hPic, int r, int g, int b );
void SPR_Draw ( int frame, int x, int y, const struct rect_s *prc );
void SPR_DrawHoles ( int frame, int x, int y, const struct rect_s *prc );
void SPR_DrawAdditive ( int frame, int x, int y, const struct rect_s *prc );
int DrawCharacter ( int x, int y, int number, int r, int g, int b );
int DrawConsoleString ( int x, int y, char *string );
void FillRGBA ( int x, int y, int width, int height, int r, int g, int b, int a );

#endif