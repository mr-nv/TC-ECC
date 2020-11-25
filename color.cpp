#include "color.h"

ColorManager colorList;
#pragma warning(disable:4786)
#pragma warning(disable:4800)
#pragma warning(disable:4244)
#pragma warning(disable:4101)
#pragma warning(disable:4715)
#pragma warning(disable:4018)

void ColorManager::Init()
{
	add(23,129,247,180);	 // Menu Title - 0
	add(225,225,225,180);	 // Menu Content - 1
	add(255, 0, 0,255);		 // Terrorist - 2
	add(0,100,255,255);		 // Counter-Terrorist - 3
	add( 0,255, 0,255);		 // Aimbot Target - 4
	add(255,255,  0,0);		 // Entity Esp Weapon - 5
	add(255,150,  0,0);		 // Entity Esp Hostage - 6
	add(0,0,0,0);			 // Menu - Deselected - 7
	add(255,255,255,0);		 // Menu - Selected - 8
	add(255,255,255,200);    // Crosshair - 9
	add(255,  0,  0,200);    // Crosshair - 10
}

ColorEntry * ColorManager::get(int index)
{
	if(index<entrys.size())
		return &entrys[index];
	else
		return &entrys[0];
}

void ColorManager::add(int r, int g, int b, int a)
{
	ColorEntry tmp;
	tmp.r = r;
	tmp.g = g;
	tmp.b = b;
	tmp.a = a;
	tmp.onebased_r = r/255;
	tmp.onebased_g = g/255;
	tmp.onebased_b = b/255;
	tmp.onebased_a = a/255;
	entrys.push_back(tmp);
	currentIndex++;
}