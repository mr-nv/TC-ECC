#include <stdio.h>
#include <fstream>
#include <windows.h>
#include "cvar.h"
#include "interpreter.h"
#include "client.h"
using namespace std;

cCVARS cvar;
//=======================================================================================
void cCVARS::Init(void)
{
	#define INITCVAR(name,value) cmd.AddCvarFloat(#name, &##name );##name=##value;
	INITCVAR(nospread,1)		INITCVAR(recoil,1.75)		INITCVAR(hadj_h,2.5)
	INITCVAR(hadj_f, 0.5)		INITCVAR(hadj_r,0.0)		INITCVAR(fov,1)
	INITCVAR(autowall,0)		INITCVAR(aimingmethod,1)	INITCVAR(shoot,0)
	INITCVAR(weapon,1)			INITCVAR(name,0)			INITCVAR(distance,0)
	INITCVAR(box,0)				INITCVAR(aim,0)				INITCVAR(radar,1)
	INITCVAR(radars,80)			INITCVAR(radary,100)		INITCVAR(radarx,90)
	INITCVAR(aimspot,1)			INITCVAR(aspeed,0)			INITCVAR(speed,0)
	INITCVAR(aimkey,1)			INITCVAR(speedkey,0)		INITCVAR(knivespeed,0)
	INITCVAR(nosmash,1)			INITCVAR(crosshair,5)		INITCVAR(wall,0)
	INITCVAR(infobox,1)			INITCVAR(entesp,1)			//INITCVAR(outlined,0)
	INITCVAR(spinhack,0)		INITCVAR(whitewalls,0)		INITCVAR(dlights,0)	
	INITCVAR(fullbright,0)		INITCVAR(blood,0)			INITCVAR(chasecam,0)
	INITCVAR(bunnyhop,0)		INITCVAR(avdraw,0)			INITCVAR(glowshells,0)
	// Just some standard Values.
}
//=======================================================================================
bool fileExists(const char* filename)
{
	WIN32_FIND_DATA finddata;
	HANDLE handle = FindFirstFile(filename,&finddata);
	return (handle!=INVALID_HANDLE_VALUE);
}
//=======================================================================================
extern inline std::string getHackDirFile(const char* basename);
void cCVARS::Save()
{
	ofstream ofs( getHackDirFile("init.ini").c_str() );
	cmd.save_cvars(ofs);
	ofs.close();
}

//=======================================================================================

void cCVARS::Load()
{
	if( !fileExists(getHackDirFile("init.ini").c_str()) ) return;
	cmd.load_cvars();
}

//=======================================================================================