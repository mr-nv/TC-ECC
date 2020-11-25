//==============================================================================
// timehandling.cpp
//==============================================================================
#include <windows.h>
#include <mmsystem.h>
#include "timehandling.h"
//==============================================================================
//TimeHandling =================================================================
//==============================================================================
float  ClientTime::mapTime    = 1;
double ClientTime::current    = 1;
unsigned int ClientTime::current_ms = 1;
//==============================================================================
void ClientTime::reportMapTime(float mapTimeNew)
{
	// map change detection
	/*if( mapTimeNew < mapTime-10 ) // time decreased (looks like a mapchange)
	{
		AtMapChange();
	}*/
	mapTime = mapTimeNew;
	updateCurrentTime();
}
//==============================================================================
void ClientTime::updateCurrentTime()
{
	// timer update
	current_ms = timeGetTime();

	current = (double)current_ms / 1000.0;
}
//==============================================================================