//==================================================================================
//
//	UserMessage Hooks - Credits to System (OGC)
//
//==================================================================================
#include <windows.h>
#include <mmsystem.h>
#include <memory.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "misc/parsemsg.h"
#include "client.h"


void logme(char * fmt, ... );
extern cl_enginefuncs_s* pEngfuncs;
int RoundsPlayed = 0;
bool bMsgHook = false;
void AtRoundStart();

#pragma warning(disable:4786)
#pragma warning(disable:4800)
#pragma warning(disable:4244)
#pragma warning(disable:4101)
#pragma warning(disable:4715)
#pragma warning(disable:4305)

DWORD HookUserMsgOrg;
pfnUserMsgHook TeamInfoOrg=NULL;
pfnUserMsgHook SetFOVOrg=NULL;
pfnUserMsgHook CurWeaponOrg=NULL;
pfnUserMsgHook ScoreAttribOrg=NULL;
pfnUserMsgHook HealthOrg=NULL;
pfnUserMsgHook BatteryOrg=NULL;
pfnUserMsgHook ScoreInfoOrg=NULL;
pfnUserMsgHook DeathMsgOrg=NULL;
pfnUserMsgHook SayTextOrg=NULL;
pfnUserMsgHook ResetHUDOrg=NULL;
pfnUserMsgHook TextMsgOrg=NULL;
pfnUserMsgHook DamageOrg=NULL;
pfnUserMsgHook AmmoXOrg=NULL;
pfnUserMsgHook WeaponListOrg=NULL;
pfnUserMsgHook MoneyOrg=NULL;
extern float fCurrentFOV;
//==================================================================================
static int TeamInfo (const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf,iSize);
	int px = READ_BYTE();
	char * teamtext = READ_STRING();
	const char * STR_TERROR = "TERRORIST"; 
    const char * STR_CT = "CT"; 
    const char * STR_UNASSIGNED = "UNASSIGNED"; 
    const char * STR_SPECTATOR = "SPECTATOR"; 
    if  (!strcmpi (teamtext, STR_TERROR)) vPlayers[px].team = 1; 
    else if  (!strcmpi (teamtext, STR_CT)) vPlayers[px].team = 2; 
    else if  (!strcmpi (teamtext, STR_UNASSIGNED)) vPlayers[px].team = 0; 
    else if  (!strcmpi (teamtext, STR_SPECTATOR)) vPlayers[px].team = 0; 
    else { 
        vPlayers[px].team = -1; 
    } 

	if(px == gEngfuncs.GetLocalPlayer()->index)
	{
		if  (!strcmpi (teamtext, STR_TERROR)) me.team = 1; 
		else if  (!strcmpi (teamtext, STR_CT)) me.team = 2; 
		else if  (!strcmpi (teamtext, STR_UNASSIGNED)) me.team = 0; 
		else if  (!strcmpi (teamtext, STR_SPECTATOR)) me.team = 0; 
		else { 
			me.team = -1; 
		} 
	}
	return (*TeamInfoOrg)(pszName, iSize, pbuf);
}

//==================================================================================
int CurWeapon(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iState = READ_BYTE();
	int iID    = READ_CHAR();
	int iClip  = READ_CHAR();
	if (iState) me.iClip = iClip;
	WeaponListCurWeapon(iState, iID, iClip);
	return (*CurWeaponOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int ScoreAttrib(const char *pszName, int iSize, void *pbuf )
{
	UpdateMe();
	BEGIN_READ(pbuf, iSize);
    int idx  = READ_BYTE();
    int info = READ_BYTE();
	if(idx==me.ent->index) me.alive = ((info&1)==0);
	return (*ScoreAttribOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int SetFOV(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	me.iFOV = READ_BYTE();
	if(!me.iFOV) { me.iFOV=90; }

	if(me.iFOV==90){ me.inZoomMode=false; }
	else           { me.inZoomMode=true;  }
	fCurrentFOV = me.iFOV;
	return (*SetFOVOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int Health(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	me.iHealth = READ_BYTE();
	return (*HealthOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int Battery(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	me.iArmor = READ_BYTE();
	return (*BatteryOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	//---



	//---
	return (*ScoreInfoOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int DeathMsg(const char *pszName, int iSize, void *pbuf)
{
	UpdateMe();
	BEGIN_READ( pbuf, iSize );
	int killer = READ_BYTE();
	int victim = READ_BYTE();
	int headshot = READ_BYTE();
    char* weaponName = READ_STRING();
	if( killer==me.ent->index && headshot)
		me.iHs++;
	if( killer==me.ent->index && victim != me.ent->index )
		me.iKills++;
	if( victim==me.ent->index )
		me.iDeaths++;
	vPlayers[victim].setDead();
	vPlayers[victim].updateClear();
	if(cvar.blood && headshot)
	{
		for(int spray_count = 0;spray_count<5;spray_count++)
		{
			cl_entity_s *pEnt = vPlayers[victim].getEnt();
			vec3_t blood_origin;
			vec3_t blood_direction;
			int speed;

			blood_origin[0] = pEnt->origin[0];
			blood_origin[1] = pEnt->origin[1];
			blood_origin[2] = pEnt->origin[2]+30;

			blood_direction[0] = (float)(rand()%41)-20;
			blood_direction[1] = (float)(rand()%41)-20;
			blood_direction[2] = (float)(rand()%251)+50;

			speed = (rand()%101)+100;

			gEngfuncs.pEfxAPI->R_BloodStream
			( 
				blood_origin,
				blood_direction, 
				70, 
				speed
			);
		}
	}
	return (*DeathMsgOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int SayText(const char *pszName, int iSize, void *pbuf)
{
	//---



	//---
	return (*SayTextOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int TextMsg(const char *pszName, int iSize, void *pbuf)
{
	//---



	//---
	return (*TextMsgOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	//---
	AtRoundStart();
	RoundsPlayed++;
	//---
	return (*ResetHUDOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int Damage(const char *pszName, int iSize, void *pbuf )
{
	//---



	//---

	return (*DamageOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int AmmoX(const char *pszName, int iSize, void *pbuf )
{
	int ID, Count;
	BEGIN_READ(pbuf, iSize);
	ID = READ_BYTE();
	Count = READ_BYTE();
	WeaponListAmmoX(ID, Count);
	return (*AmmoXOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int WeaponListMsg(const char *pszName, int iSize, void *pbuf )
{
	char *weaponname;
	int ammo1type, max1, ammo2type, max2, slot, slotpos, id, flags;
	BEGIN_READ(pbuf, iSize);
	weaponname = READ_STRING();
	ammo1type = READ_CHAR();
	max1 = READ_BYTE();
	ammo2type = READ_CHAR();
	max2 = READ_BYTE();
	slot = READ_CHAR();
	slotpos = READ_CHAR();
	id = READ_CHAR();
	flags = READ_BYTE();
	WeaponListAdd(weaponname, ammo1type, max1, ammo2type, max2, slot, slotpos, id, flags);
	return (*WeaponListOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int Money(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	me.iMoney = READ_SHORT();
	return (*MoneyOrg)(pszName,iSize,pbuf);
}

//==================================================================================
int	HookUserMsg (char *szMsgName, pfnUserMsgHook pfn)
{
	#define REDIRECT_MESSAGE(name) \
		else if (!strcmp(szMsgName,#name)) \
		{ \
			name##Org = pfn; \
			return gEngfuncs.pfnHookUserMsg (szMsgName, ##name ); \
		}
	
	int retval = gEngfuncs.pfnHookUserMsg (szMsgName, pfn);

	if(0){}
	REDIRECT_MESSAGE( TeamInfo    )
	REDIRECT_MESSAGE( CurWeapon   )
	REDIRECT_MESSAGE( ScoreAttrib )
	REDIRECT_MESSAGE( SetFOV      )
	REDIRECT_MESSAGE( Health	  )
	REDIRECT_MESSAGE( Battery     )
	REDIRECT_MESSAGE( ScoreInfo	  )
	REDIRECT_MESSAGE( DeathMsg    ) 
	REDIRECT_MESSAGE( SayText     )
	REDIRECT_MESSAGE( TextMsg     )
	REDIRECT_MESSAGE( ResetHUD    )
	REDIRECT_MESSAGE( Damage	  )
	REDIRECT_MESSAGE( AmmoX		  )
	REDIRECT_MESSAGE( Money		  )
	else if (!strcmp(szMsgName,"WeaponList")) // Because the Class is called like the Msg
	{
		WeaponListOrg = pfn;
		retval = gEngfuncs.pfnHookUserMsg(szMsgName, WeaponListMsg);
	}
	return retval;
}
