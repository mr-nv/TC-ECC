#include <windows.h>
#include <time.h>
#include "client.h"
//==================================================================================
// Initialize structures for use in client
//==================================================================================
struct cl_enginefuncs_s gEngfuncs		= { NULL };
cl_enginefuncs_s* pEngfuncs				=( cl_enginefuncs_s* )0x01E882A8;
engine_studio_api_s* pStudio			=( engine_studio_api_s* )0x01EA41E0;
double* globalTime						= (double*)0x027DA8C0;
engine_studio_api_t IEngineStudio		= { NULL };
extern bool bAim;
//==================================================================================
// HUD_Init Client Function
//==================================================================================
void HUD_Init ( void )
{
	return;
}

//==================================================================================
// HUD_Redraw Client Function
//==================================================================================
void HUD_Redraw ( float x, int y )
{
	InitVisuals();
	if(cvar.chasecam)
		cmd.exec("chase_active hide 1;r_drawviewmodel hide 0");
	else
		cmd.exec("chase_active hide 0;r_drawviewmodel hide 1");
	Aimbot.FindTarget();
	if(cvar.entesp) DrawEntityEsp();
	gMenu.Draw(200, 200);
	int cW = screeninfo.iWidth/1.8;
	int cH = screeninfo.iHeight/1.9;
	int cX = displayCenterX - cW/2;
	int cY = displayCenterY - cH/2;
	gConsole.draw(cX,cY,cW,cH);
	if (cvar.nosmash)
	{
		static screenfade_t sf = { 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0 }; 
		gEngfuncs.pfnSetScreenFade( &sf ); 
	}
	if (cvar.crosshair) DrawCrosshair();
	ClientTime::reportMapTime(gEngfuncs.GetClientTime());
	if(cvar.infobox) gGui.DrawInfoBoxLeft();
    if ((cvar.radar == 1) || (!mapLoaded && cvar.radar == 2))	{DrawRadar();}
	else if ((cvar.radar == 2) && mapLoaded)					{overview_redraw();}
	Aimbot.DrawAimSpot();
	for (int ax=0;ax<MAX_VPLAYERS;ax++) if (vPlayers[ax].isUpdated() ) { drawPlayerEsp(ax); }
	for(int i=0;i<MAX_VPLAYERS;i++) 
		if(!bIsEntValid(vPlayers[i].getEnt(), i)) vPlayers[i].updateClear();
	}

//==================================================================================
// HUD_PlayerMove Client Function
//==================================================================================
void HUD_PlayerMove ( struct playermove_s *ppmove, qboolean server )
{
	me.pmMoveType = ppmove->movetype;
    VectorCopy(ppmove->velocity,me.pmVelocity);
	
	// copy origin+angles
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(me.pmEyePos);	
    me.pmEyePos[0] += ppmove->origin[0];
	me.pmEyePos[1] += ppmove->origin[1];
	me.pmEyePos[2] += ppmove->origin[2];
	me.pmFlags = ppmove->flags;
	//me.pmEyePos[2] += 4;

	VectorCopy(ppmove->angles,me.viewAngles);
}

//==================================================================================
// CL_CreateMove Client Function
//==================================================================================
float gSpeed;
void CL_CreateMove ( float frametime, struct usercmd_s *cmd, int active )
{
	memcpy(&Aimbot.g_Originalcmd, cmd, sizeof(usercmd_t));
	//LTFX Speed
	float spd = 0.0;	
	if (gSpeed)						spd = gSpeed;
	else if (!cvar.speedkey)		spd = cvar.speed;
	if (cvar.speed>1) *globalTime += frametime * spd/5;
	if (cmd->buttons&IN_ATTACK && (cvar.aspeed || cvar.knivespeed)
		&& !IsCurWeaponNade() && !IsCurWeaponC4()) 
	{
		spd = cvar.aspeed;
		if(cvar.knivespeed && GetCurWeaponId() == WEAPONLIST_KNIFE) spd = 20.0;
		*globalTime += frametime * spd/5;
	}
	// aim
	Aimbot.active = false;
	if ( me.alive )          
	if ( cvar.aim || bAim )          
	if ( me.iClip )           
	if ( Aimbot.HasTarget() )
	{
		// auto aim	
		Aimbot.CalculateAimingView();
		VectorCopy(Aimbot.aim_viewangles, cmd->viewangles);	
		Aimbot.active = true;
	}

	// auto shoot
	if (cvar.shoot && me.iClip && Aimbot.active && !IsCurWeaponNade() && !IsCurWeaponC4()) 
		cmd->buttons |= IN_ATTACK;

	if(me.alive)
	{
		if (cvar.nospread == 1 || (cvar.nospread == 2 && cmd->buttons & IN_ATTACK))
		{
			float offset[3];
			gNoSpread.GetSpreadOffset(me.spread.random_seed, 1, cmd->viewangles, me.pmVelocity, offset);
			cmd->viewangles[0] += offset[0];
			cmd->viewangles[1] += offset[1];
			cmd->viewangles[2] += offset[2];
		}
		if (cvar.recoil)
		{
			cmd->viewangles[0] -= (me.punchangle[0] * cvar.recoil);
			cmd->viewangles[1] -= (me.punchangle[1] * cvar.recoil);
		}
		if(cvar.bunnyhop) me.DoBunnyHop(cmd);
	}
	if(cvar.spinhack && !IsCurWeaponNade())
		Aimbot.FixupAngleDifference(cmd);
}


//==================================================================================
// Called before V_CalcRefdef Client Function
//==================================================================================
void PreV_CalcRefdef ( struct ref_params_s *pparams )
{
	UpdateMe();
	VectorCopy(pparams->punchangle,me.punchangle);
	if (cvar.recoil) for (int i=0; i<3; i++) pparams->punchangle[i] = 0; 	
	if ( me.alive )          
	if ( cvar.aim || bAim )          
	if ( me.iClip )           
	if ( Aimbot.HasTarget() )
	{
		Aimbot.CalculateAimingView();
		VectorCopy(Aimbot.aim_viewangles, pparams->cl_viewangles);
		VectorCopy(Aimbot.aim_viewangles, pparams->viewangles);
	} 
}


//==================================================================================
// Called after V_CalcRefdef Client Function
//==================================================================================
void PostV_CalcRefdef ( struct ref_params_s *pparams )
{
	if( pparams->nextView == 0 )
	{
		// Primary Screen, below all other Viewports Sent
		// update vectors for CalcScreen
		VectorCopy(pparams->viewangles,mainViewAngles);
		VectorCopy(pparams->vieworg,mainViewOrigin);	
		if ( me.alive )          
		if ( cvar.aim || bAim )          
		if ( me.iClip )           
		if ( Aimbot.HasTarget() )
		{
			// auto aim	
			Aimbot.CalculateAimingView();
			VectorCopy(Aimbot.aim_viewangles, pparams->viewangles);	
			Aimbot.active = true;
		}
	}
}


//==================================================================================
// HUD_AddEntity Client Function
//==================================================================================
int AddEntResult = 1;
void HUD_AddEntity ( int type, struct cl_entity_s *ent, const char *modelname )
{
	UpdateMe();
	AddEntResult=1;
	if(0){}
	else if( isValidEnt(ent) ) 
	{
		if(me.alive)
		{
			gEngfuncs.CL_CreateVisibleEntity(ET_PLAYER, ent);
			AddEntResult = 0;
		}
		vPlayers[ent->index].updateAddEntity(ent->origin); 
		vPlayers[ent->index].setAlive();
		vPlayers[ent->index].updateEntInfo();
		ent->curstate.rendermode = 0;	// Against the Evil Admins
		ent->curstate.renderfx = 0;		// and WC3 Mod
		playerCalcExtraData(ent->index, ent);
		if(cvar.glowshells)
		{
			color24 color;
			ColorEntry *clr = PlayerColor(ent->index);
			color.r = clr->r;
			color.g = clr->g;
			color.b = clr->b;
			ent->curstate.renderamt = 1;
			ent->curstate.renderfx |= kRenderFxGlowShell;
			ent->curstate.rendercolor = color;
		}
	}
	if(ent->index == me.ent->index) 
	{ 
		if(cvar.chasecam == 2)
		{
			ent->curstate.renderamt = 60;
			ent->curstate.rendermode = kRenderTransAdd;
		}
		int px = ent->index;

	}
	if(me.ent->curstate.iuser1 == 4 && me.ent->curstate.iuser2 == ent->index)
		AddEntResult=0;
}


//==================================================================================
// HUD_PostRunCmd Client Function
//==================================================================================
void HUD_PostRunCmd ( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed )
{
	gNoSpread.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);
}

//==================================================================================
// HUD_UpdateClientData Client Function
//==================================================================================
void HUD_UpdateClientData(client_data_t *cdata, float flTime)
{
	for (int i = 0; i < MAX_VPLAYERS ;i++)
	{
		if(!bIsEntValid(vPlayers[i].getEnt(),i))
			vPlayers[i].updateClear();
	}
	WeaponListUpdate(cdata->iWeaponBits);
	// Another great place to Update Stuff
}

//==================================================================================
// HUD_Key_Event Client Function
//==================================================================================
int KeyEventResult = 1;
char* gGetWeaponName( int weaponmodel );
int HUD_Key_Event ( int eventcode, int keynum, const char *pszCurrentBinding )
{
	KeyEventResult=1;
	//Handle Menu on INS
	if (keynum == 147 && eventcode) {gMenu.Activated=!gMenu.Activated;cvar.Save();KeyEventResult=0;return 0;}
	if (eventcode)
		KeyEventResult=gMenu.Key(keynum);
	if (keynum == 148 && eventcode) {gConsole.active = !gConsole.active;cvar.Save();KeyEventResult=0;return 0;}

	if( gConsole.active && eventcode )
	{
		gConsole.key(keynum,true);
		KeyEventResult = 0;		
		return 0;		
	}
	//Handle HLH AIMING
	if(!bDoAimKey(eventcode)) { KeyEventResult = 1; return 0; }
	if (cvar.aimkey && !IsCurWeaponNade() && !IsCurWeaponC4())
	{
		if (keynum == (240+cvar.aimkey) && !gMenu.Activated) {KeyEventResult = DoHLHAiming(eventcode);}
	}
		
	//Handle Speed
	if (cvar.speedkey)
		if (cvar.speedkey && !IsCurWeaponNade() && !IsCurWeaponC4())
		if (keynum == (240+cvar.speedkey) ) {KeyEventResult = DoSpeed(eventcode);}
}


//==================================================================================
// SPR_Load Engine function
//==================================================================================
HSPRITE SPR_Load ( const char *szPicName )
{
	return gEngfuncs.pfnSPR_Load( szPicName );
}

//==================================================================================
// SPR_Set Engine Function
//==================================================================================
void SPR_Set ( HSPRITE hPic, int r, int g, int b )
{
	return gEngfuncs.pfnSPR_Set( hPic, r, g, b );
}

//==================================================================================
// SPR_Draw Engine Function
//==================================================================================
void SPR_Draw ( int frame, int x, int y, const struct rect_s *prc )
{
	return gEngfuncs.pfnSPR_Draw( frame, x, y, prc );
}


//==================================================================================
// SPR_DrawHoles Engine Function
//==================================================================================
void SPR_DrawHoles ( int frame, int x, int y, const struct rect_s *prc )
{
	return gEngfuncs.pfnSPR_DrawHoles( frame, x, y, prc );
}

//==================================================================================
// SPR_DrawAdditive Engine Function
//==================================================================================
void SPR_DrawAdditive ( int frame, int x, int y, const struct rect_s *prc )
{
	return gEngfuncs.pfnSPR_DrawAdditive( frame, x, y, prc );
}


//==================================================================================
// DrawCharacter Engine Function
//==================================================================================
int DrawCharacter ( int x, int y, int number, int r, int g, int b )
{
	return gEngfuncs.pfnDrawCharacter( x, y, number, r, g, b );
}

//==================================================================================
// DrawConsoleString Engine Function
//==================================================================================
int DrawConsoleString ( int x, int y, char *string )
{
	return gEngfuncs.pfnDrawConsoleString( x, y, string );
}

//==================================================================================
// FillRGBA Engine Function
//==================================================================================
void FillRGBA ( int x, int y, int width, int height, int r, int g, int b, int a )
{
	return gEngfuncs.pfnFillRGBA( x, y, width, height, r, g, b, a );
}
//==============================================================================
void AtMapChange()
{
	me.iKills = 0;
	me.iHs = 0;
	me.iDeaths = 0;
	mapLoaded = false;
	overview_loadcurrent();
}
//==================================================================================
// AtRoundStart - Called every start of a round
//==================================================================================
void AtRoundStart(void)
{
	static char currentMap[100];
	if( strcmp(currentMap,gEngfuncs.pfnGetLevelName()) )
	{
		strcpy(currentMap,gEngfuncs.pfnGetLevelName());
		AtMapChange();
	}
	Aimbot.iTarget = 0;
	for(int i=0;i<MAX_VPLAYERS;i++) 
	{ 
		vPlayers[i].updateClear();
		vPlayers[i].bGotHead = false;
		vPlayers[i].setAlive();
	}
}
