#include "players.h"
#include "client.h"
#include "Aimbot.h"
#include "TableFont.h"
#include "cSprites.h"
#include "radar.h"
#include <memory.h>

sMe me;
VecPlayers vPlayers;
float mainViewAngles[3];
#define BOUND_VALUE(var,min,max) if((var)>(max)){(var)=(max);};if((var)<(min)){(var)=(min);}

int Cstrike_SequenceInfo[] = 
{
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0, // 0..9   
	0,	1,	2,	0,	1,	2,	0,	1,	2,	0, // 10..19 
	1,	2,	0,	1,	1,	2,	0,	1,	1,	2, // 20..29 
	0,	1,	2,	0,	1,	2,	0,	1,	2,	0, // 30..39 
	1,	2,	0,	1,	2,	0,	1,	2,	0,	1, // 40..49 
	2,	0,	1,	2,	0,	0,	0,	8,	0,	8, // 50..59 
	0, 16,	0, 16,	0,	0,	1,	1,	2,	0, // 60..69 
	1,	1,	2,	0,	1,	0,	1,	0,	1,	2, // 70..79 
	0,	1,	2, 	32, 40, 32, 40, 32, 32, 32, // 80..89
   	33, 64, 33, 34, 64, 65, 34, 32, 32, 4, // 90..99
	4,	4,	4,	4,	4,	4,	4,	4,	4,	4, // 100..109
	4                                      	// 110
};

ColorEntry * PlayerColor(int ax)
{
	if(vPlayers[ax].team == 1 && ax != Aimbot.iTarget)
		return colorList.get(2);
	if(vPlayers[ax].team == 2 && ax != Aimbot.iTarget)
		return colorList.get(3);
	if(ax == Aimbot.iTarget)
		return colorList.get(4);
}
static inline void strcpy_x2(char* dest, const char* pos)
{
	do{ 
		if(*pos=='.'||*pos=='-'||*pos=='_') {*dest=0;break; } // cut off ".wav"
		*dest=*pos;
		++dest;
		++pos; 
	}
	while(*pos);
}

// fix for ogc
bool bIsEntValid(cl_entity_s * ent,int index)
{

	if (vPlayers[index].updateType() == 2 || vPlayers[index].timeSinceLastUpdate() < 0.3)
		return true;
	UpdateMe(); // Avoid Crash
	cl_entity_s* localEnt = gEngfuncs.GetEntityByIndex(me.ent->index);

	if(ent && !(ent->curstate.effects & EF_NODRAW) && ent->player && !ent->curstate.spectator 
		&& ent->curstate.solid && !(ent->curstate.messagenum < localEnt->curstate.messagenum))
		return true;

	return false;
}

static inline float GetDistanceFrom(const float* pos);
void playerSound(int index, const float*const origin, const char*const sample)
{
	UpdateMe();
	if(index == me.ent->index) return;

	// fill infos:
	PlayerInfo& player = vPlayers[index];

//	if (!player.isAlive())
//		return;
	
	// calc player data 
	player.updateEntInfo();
	player.updateSoundRadar();
	player.SetOrigin(origin);
//	player.updateAddEntity(origin); // ECC ESP compatibility
	player.distance  = GetDistanceFrom (player.origin()); if(player.distance<1) player.distance=1; // avoid division by zero
	player.visible   = false;

	// set weapon
	if( sample && sample[0]=='w' && sample[1]=='e' && sample[7]=='/' ) // ex: weapons/ak-1.wav
	{
		const char* pos = sample+8;
		if(strlen(pos)<30)
		{
			char tmp[32];
			strcpy_x2(tmp,pos);
			
			if(!strcmp(tmp,"zoom")){ strcpy(tmp,"awp"); } // zooming means usually awp

			if(strcmp(tmp,"knife")) // dont update for knife
			{
				player.setWeapon(tmp);
			}
		}
	}
}


void DoExtraRendering(cl_entity_s * ent, int team)
{
	ColorEntry * pClr = PlayerColor(ent->index);
	if(cvar.dlights)
	{
		dlight_t * pDl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
		pDl->color.r = pClr->r;
		pDl->color.g = pClr->g;
		pDl->color.b = pClr->b;
		pDl->origin = ent->origin;
		pDl->die = gEngfuncs.GetClientTime() + 0.1;
		pDl->radius = 100;
		
	}
}

bool CalcScreen(const float *origin, float *vecScreen)
{
	int result = gEngfuncs.pTriAPI->WorldToScreen( const_cast<float*>(origin),vecScreen);
	
	if(vecScreen[0] < 1 && vecScreen[1] < 1 && vecScreen[0] > -1 && vecScreen[1] > -1 && !result)
	{
		vecScreen[0] =  vecScreen[0] * displayCenterX + displayCenterX;
		vecScreen[1] = -vecScreen[1] * displayCenterY + displayCenterY;
		return true;
	}
	return false;
}

void UpdateMe(void)
{
	me.ent    = gEngfuncs.GetLocalPlayer();
	static cl_entity_s dummy;
	memset((char*)&dummy,0,sizeof(dummy));
	if(!me.ent){ me.ent = &dummy; }
}

void DrawEntityEsp(void)
{
 	cl_entity_s * pMe = gEngfuncs.GetLocalPlayer();
	for(int i=1; i<1024; i++)
	{
		cl_entity_s * pEnt = gEngfuncs.GetEntityByIndex(i);
		ColorEntry * clr;
		if(pEnt && (pEnt->curstate.messagenum+10 > pMe->curstate.messagenum) && pEnt->model && pEnt->model->name)
		{
			float tmp[2];
			char mdl[64];
			char *wpn = NULL;

			strncpy(mdl, pEnt->model->name, 64);
			wpn = mdl;

			if(wpn && CalcScreen(pEnt->origin, tmp) && strstr(wpn, "w_"))
			{
				wpn += 9;
				wpn[strlen(wpn)-4] = 0;
				clr = colorList.get(5);
				PrintWithFont(tmp[0], tmp[1],clr->r,clr->g,clr->b, "%s", wpn);
			}
			else if(wpn && (strstr(wpn, "hostage") || (strstr(wpn, "scientist"))))
			{
				vec3_t forward, right, up, entOrg ;

				wpn += 7;
				wpn[strlen(wpn)-4] = 0;

				VectorCopy(pEnt->origin, entOrg);
				gEngfuncs.pfnAngleVectors(pEnt->angles, forward, right, up);
				entOrg = entOrg + up * 40;
				clr = colorList.get(6);
				if(CalcScreen(entOrg, tmp))
					PrintWithFont(tmp[0], tmp[1],clr->r,clr->g,clr->b, "%s", wpn);
			}
		}
		else if(!pEnt)
			break;
	}
}

char* gGetWeaponName( int weaponmodel ) 
{ 
    static char weapon[50]; 
    weapon[0]=0; 
    model_s* mdl = pStudio->GetModelByIndex( weaponmodel ); 
    if( !mdl ){ return weapon; } 
    char* name = mdl->name;  if( !name )          { return weapon; } 
     
    int len = strlen(name);  if( len>48 || len<10){ return weapon; } 
    if (len > 25)//to catch shield string 
    { 
        strcpy(weapon,name+23); len -= 23; 
    } 
    else//no shield 
    { 
        strcpy(weapon,name+9); len -=9; 
    } 
    if(len>4)weapon[len-4]=(char)0; 
        return weapon; 
} 

static inline float GetDistanceFrom(const float* pos)
{	
	register double a = pos[0] - me.pmEyePos[0];
	register double b = pos[1] - me.pmEyePos[1];
	register double c = pos[2] - me.pmEyePos[2];
	return sqrt(a*a + b*b + c*c);
}

void playerCalcExtraData(int ax, cl_entity_s* ent)
{
	PlayerInfo& r = vPlayers[ax];

	// playername and model
	r.updateEntInfo();

	// weapon
	if( cvar.weapon && pStudio->GetModelByIndex )
	{
		vPlayers[ax].setWeapon(gGetWeaponName(vPlayers[ax].getEnt()->curstate.weaponmodel));
	}

	// distance
	r.distance  = GetDistanceFrom (r.origin()); if(r.distance<1) r.distance=1; // avoid division by zero
	Aimbot.calcFovangleAndVisibility(ax);
	vPlayers[ax].bGotHead = false;
}

extern bool bAim;
bool bDoAimKey(int eventcode)
{
	if(!cvar.aimkey) return false;
	char *szWeapon;
	UpdateMe();
	if(me.alive && eventcode == 0) szWeapon = gGetWeaponName(me.ent->curstate.weaponmodel);
	if(!strcmp(szWeapon, "hegrenade") || !strcmp(szWeapon, "smokegrenade") || !strcmp(szWeapon, "c4") || !strcmp(szWeapon, "flashbang") || !me.alive) 
	{ 
		if(eventcode == 0)
			gEngfuncs.pfnClientCmd("-attack"); 
		cvar.aim = 0;
		bAim = false;
		return false; 
	}
	return true;
}

void drawPlayerEsp(int ax)
{
	if(!vPlayers[ax].isAlive() || strstr(vPlayers[ax].getName(), "\\noname\\")) return;

	ColorEntry* color = PlayerColor(ax);
	if (cvar.radar){drawRadarPoint((float*)vPlayers[ax].origin(),color->r,color->g,color->b,255,false,3);}
	DoExtraRendering(vPlayers[ax].getEnt(), vPlayers[ax].team);

	// rest of code is only for players on screen:
	float vecScreen[2];
	if( !CalcScreen(vPlayers[ax].origin(),vecScreen) ) { return; }	

	// distance and boxradius also needed for esp offset
	float distance = vPlayers[ax].distance/22.0f;
	extern float fCurrentFOV;
	int   boxradius = (300.0*90.0) / (distance*fCurrentFOV);	 
	BOUND_VALUE(boxradius,1,200);

	if (cvar.box == 1)
		gDrawBoxAtScreenXY(vecScreen[0],vecScreen[1],color->r,color->g,color->b,color->a,boxradius);
	else if (cvar.box == 2)
		gDrawBoxAtScreenXY(vecScreen[0],vecScreen[1],color->r,color->g,color->b,color->a,10);	
	
	// ====== show visibility
	const char* format_string; 
	const char* format_int;
	if (vPlayers[ax].visible) { format_string="%s";  format_int="%i"; }
	else                      { format_string="-%s-";format_int="-%i-"; }

	// ===== prepare text esp:
	enum{ CHAR_HEIGHT = 11 };
	int text_dist = (int)(boxradius);
	int ystep = -CHAR_HEIGHT;

	int x = vecScreen[0];
	int y = vecScreen[1];
	if (me.alive) y = y-text_dist-CHAR_HEIGHT+10;
	
	if (cvar.weapon)
	{
		const char* displayname = vPlayers[ax].getWeapon();
		bool bGotSeq = false;
		int seqinfo = getSeqInfo(ax);
		const char* tmpSeq;
		if (seqinfo & SEQUENCE_ARM_C4)
			tmpSeq = "- ARMING C4 -";
		else if (seqinfo & SEQUENCE_RELOAD)
			tmpSeq = "- RELOADING -";
		else if (seqinfo & SEQUENCE_THROW)
			tmpSeq = "- THROWING - ";
		else if (seqinfo & SEQUENCE_SHIELD)
			tmpSeq = "- SHIELDED -";
		else if (seqinfo & SEQUENCE_SHIELD_SIDE)
			tmpSeq = "- SHIELD TO SIDE -";
		else if (seqinfo & SEQUENCE_DIE)
			tmpSeq = "";

		if ( (seqinfo & SEQUENCE_ARM_C4) || (seqinfo & SEQUENCE_THROW) || 
			(seqinfo & SEQUENCE_SHIELD) || (seqinfo & SEQUENCE_SHIELD_SIDE) ||
			( seqinfo & SEQUENCE_RELOAD ) )
		{
			displayname = tmpSeq;	
			bGotSeq = true;
		}
		int index;
		if(cvar.weapon == 1)
			gFont.drawString(true, x,y,color->r,color->g,color->b,"%s",displayname);
		else if(cvar.weapon == 2 && !bGotSeq)
		{
			char buf[255];
			if (strstr(displayname,"nade"))
				{ sprintf(buf,"d_grenade");	}
			else if (strstr(displayname,"c4"))
				{ sprintf(buf,"c4"); }
			else if (strstr(displayname,"mp5"))
				{ sprintf(buf,"d_mp5navy");	}
			else{ sprintf(buf, "d_%s",displayname); }
			index = GetSpriteIndex(buf);
			if(strstr(buf,"d_N-A") || strstr(buf,"dryfire")) index = -1;
			if (index != -1)
			{				
								
				gEngfuncs.pfnSPR_Set(spritedata.spriteinfo[index].hspr, color->r,color->g,color->b); // Get sprite ready
				gEngfuncs.pfnSPR_DrawAdditive(0, vecScreen[0]-10, vecScreen[1]-42, &(spritedata.spriteinfo[index].rc)); // Draw sprite
			}
		}
		if(cvar.weapon == 2 && bGotSeq || index == -1)
			gFont.drawString(true, x,y,color->r,color->g,color->b,"%s",displayname);
		y += ystep;
	}
	
	if (cvar.name )
	{
		char displayname[32];
		int len = (cvar.name==1) ? 8 : cvar.name;
		if(len<0 || len>30) { len = 30; }
		strncpy(displayname,vPlayers[ax].getName(),len);
		displayname[len] = 0;
		
		gFont.drawString(true,x,y,color->r,color->g,color->b,format_string,displayname); 
		y += ystep;
	}

	if (cvar.distance)
	{
		//PrintWithFont(x,y,color->r,color->g,color->b,format_int,  (short int)distance);
		gFont.drawString(true, x, y, color->r,color->g,color->b, format_int, (short int)distance);
		y += ystep;
	}
}

void sMe::DoBunnyHop(struct usercmd_s *usercmd)
{
	if((usercmd->buttons & IN_JUMP) && !(me.pmFlags & FL_ONGROUND)) usercmd->buttons &= ~IN_JUMP;
}

int getSeqInfo(int ax)
{
	return Cstrike_SequenceInfo[vPlayers[ax].getEnt()->curstate.sequence];
}