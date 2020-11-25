#ifndef _CVARS_
#define _CVARS_

class cCVARS
{
public:
	void Init(void);
	void Load();
	void Save();

	
	float nospread;
	float recoil;
	float fov;
	float hadj_h;
	float hadj_f;
	float hadj_r;
	float autowall;
	float aimingmethod;
	float aimspot;
	float shoot;
	float aim;
	float weapon;
	float box, distance ,name;
	float radar;       
	float radarx; 
	float radary; 
	float radars;  
	float speed, aspeed, knivespeed;
	float aimkey, speedkey;
	float nosmash;
	float crosshair;
	float wall;
	float infobox;
	float entesp;
	float outlined;
	float spinhack;
	
	float fullbright;
	float whitewalls;
	float dlights;
	
	float blood;
	float chasecam;
	float bunnyhop;

	float avdraw;

	float glowshells;
};
extern cCVARS cvar;

#endif