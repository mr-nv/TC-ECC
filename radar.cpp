//==============================================================================
// radar.cpp
//==============================================================================

#include "windows.h"
#include <gl/gl.h>
#include <windows.h>
#include <stdlib.h>
#include <math.h>
#undef NDEBUG
#include <assert.h>
#include <memory.h>
#include <map>
#include <vector>
#include <fstream>
#include "radar.h"
#include "cvar.h"
#include "client.h"
#include "color.h"
#include "drawing.h"
#include "players.h"
#include "gui.h"

extern SCREENINFO screeninfo;
int ov_radar_x, ov_radar_y;
float ov_zoom = 5.5f;
bool mapLoaded = false;

#pragma warning(disable:4786)
#pragma warning(disable:4800)
#pragma warning(disable:4244)
#pragma warning(disable:4101)
#pragma warning(disable:4715)
#pragma warning(disable:4305)

#define BOUND_VALUE(var,min,max) if((var)>(max)){(var)=(max);};if((var)<(min)){(var)=(min);}
#define PI 3.14159265358979f

using namespace std;

void overview_draw_me(float * origin, int radius, int r, int g, int b, int a);

void HUD_DrawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE ); 
	gEngfuncs.pTriAPI->Begin( TRI_LINES );

	gEngfuncs.pTriAPI->Color4f( r/255.0f, g/255.0f, b/255.0f, a/255.0f );

	gEngfuncs.pTriAPI->Brightness(1);
	gEngfuncs.pTriAPI->Vertex3f((float)x1, (float)y1, 0);
	gEngfuncs.pTriAPI->Vertex3f((float)x2, (float)y2, 0);
	gEngfuncs.pTriAPI->End();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

model_s * m_MapSprite = 0;
int xTiles = 1;
int yTiles = 1;

const float screenaspect = (float)(4.0/3.0);

struct overviewInfo_s
{
	float	zoom;		// zoom of map images
	int		layers;		// how may layers do we have
	float	origin[3];  // 
	float	layersHeights[1];
	char	layersImages[1][255];
	int		rotated;	// are map images rotated (90 degrees) ?
};

static overviewInfo_s m_OverviewData;

float m_OverviewZoom = 1.0f;

float ov_dx=3;
float ov_dy=4;

//-------------------------------------------

bool parse_overview(char* overview_txt)
{
	// defaults
	m_OverviewData.origin[0] = 0.0f;
	m_OverviewData.origin[1] = 0.0f;
	m_OverviewData.origin[2] = 0.0f;
	m_OverviewData.zoom	= 1.0f;
	m_OverviewData.layers = 0;
	m_OverviewData.layersHeights[0]   = 0.0f;
	m_OverviewData.layersImages[0][0] = 0;
	
	// parse file:
	char token[1024];
	char* pfile = (char *)gEngfuncs.COM_LoadFile( overview_txt, 5, NULL);

	if (!pfile)
	{
//		gConsole.echo("can't open file %s.", overview_txt );
		mapLoaded = false;
		return false;
	}

	while (true)
	{
		pfile = gEngfuncs.COM_ParseFile(pfile, token);
		if (!pfile) 
		{ 
			break; 
		}

		if ( !stricmp( token, "global" ) )
		{
			// parse the global data
			pfile = gEngfuncs.COM_ParseFile(pfile, token);
			if ( stricmp( token, "{" ) ) 
			{
//				gConsole.echo("parse error in %s", overview_txt );
				mapLoaded = false;
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile(pfile,token);
			if (!pfile) 
			{ 
				break; 
			}

			while (stricmp( token, "}") )
			{
				if ( !stricmp( token, "zoom" ) )
				{
					pfile = gEngfuncs.COM_ParseFile(pfile,token);
					m_OverviewData.zoom = (float)atof( token );
					m_OverviewZoom = m_OverviewData.zoom;
				} 
				else if ( !stricmp( token, "origin" ) )
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token); 
					m_OverviewData.origin[0] = (float)atof( token );
					pfile = gEngfuncs.COM_ParseFile(pfile,token); 
					m_OverviewData.origin[1] = (float)atof( token );
					pfile = gEngfuncs.COM_ParseFile(pfile, token); 
					m_OverviewData.origin[2] = (float)atof( token );
				}
				else if ( !stricmp( token, "rotated" ) )
				{
					pfile = gEngfuncs.COM_ParseFile(pfile,token); 
					m_OverviewData.rotated = atoi( token );
				}

				pfile = gEngfuncs.COM_ParseFile(pfile,token); // parse next token
			}
		}
		else if ( !stricmp( token, "layer" ) )
		{
			pfile = gEngfuncs.COM_ParseFile(pfile,token);
				
			if ( stricmp( token, "{" ) ) 
			{
//				gConsole.echo("parse error in %s", overview_txt );
				mapLoaded = false;
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile(pfile,token);

			while (stricmp( token, "}") )
			{
				if ( !stricmp( token, "image" ) )
				{
					pfile = gEngfuncs.COM_ParseFile(pfile,token);
					strcpy(m_OverviewData.layersImages[ m_OverviewData.layers ], token);
				} 
				else if ( !stricmp( token, "height" ) )
				{
					pfile = gEngfuncs.COM_ParseFile(pfile,token); 
					float height = (float)atof(token);
					m_OverviewData.layersHeights[ m_OverviewData.layers ] = height;
				}
				pfile = gEngfuncs.COM_ParseFile(pfile,token); // parse next token
			}

			m_OverviewData.layers++;
		}
	}
	return true;
}

void overview_load( char* levelname )
{
	// dont load same map again
	static char last_levelname[256]="";
	char overview_txt[256];

	if( !strcmp(last_levelname,levelname)) 	{ return;} 

	// parse file
	if(levelname[0] == NULL)::strcpy(levelname,"cs_miltia");
	sprintf(overview_txt, "overviews/%s.txt", levelname );
	bool parse_success = parse_overview(overview_txt);

	if(!parse_success) 
	{ 
//		gConsole.echo("couldnt parse %s",overview_txt); 
		strcpy(last_levelname,levelname);
		mapLoaded = false;
		return;
	}

	// map sprite
	m_MapSprite = gEngfuncs.LoadMapSprite( m_OverviewData.layersImages[0] );

	if( !m_MapSprite ) 
	{
//		gConsole.echo("couldnt load %s",m_OverviewData.layersImages[0]);
		strcpy(last_levelname,levelname);
		mapLoaded = false;
		return; 
	}

	mapLoaded = true;

	// set additional data	
	float i = m_MapSprite->numframes / (4*3);
	i = (int)sqrt(i); // .NET FIX
	xTiles = i*4;
	yTiles = i*3;
}

void overview_loadcurrent()
{
	if(!gEngfuncs.pfnGetLevelName)return;

	char levelname[256];

	strcpy(levelname, gEngfuncs.pfnGetLevelName()+5);

	if (strlen(levelname)<5)return;

	levelname[strlen(levelname)-4] = 0;

	overview_load( levelname );
}
bool initi = false;

void overview_calcRadarPoint(float* , int * , int *);

inline void get_player_tile_coords(float& x, float& y)
{
	if(m_OverviewData.rotated)
	{
		float origin_tilex = (float)(-ov_dy + m_OverviewData.zoom * (1.0/1024.0) * m_OverviewData.origin[0]);
		float origin_tiley = (float)( ov_dx + m_OverviewData.zoom * (1.0/1024.0) * m_OverviewData.origin[1]);

		y = (float)(origin_tilex - (1.0/1024) * m_OverviewData.zoom * me.pmEyePos[0]);
		x = (float)(origin_tiley - (1.0/1024) * m_OverviewData.zoom * me.pmEyePos[1]);
		y = -y;
	}
	else
	{
		float origin_tilex = (float)( ov_dx + m_OverviewData.zoom * (1.0/1024.0) * m_OverviewData.origin[0]);
		float origin_tiley = (float)( ov_dy + m_OverviewData.zoom * (1.0/1024.0) * m_OverviewData.origin[1]);

		x = (float)(origin_tilex - (1.0/1024) * m_OverviewData.zoom * me.pmEyePos[0]);
		y = (float)(origin_tiley - (1.0/1024) * m_OverviewData.zoom * me.pmEyePos[1]);
	}
}
//float mainViewAngles[3];

int clientopenglcalc(int number) // y axis correction :)
{
	return (screeninfo.iHeight - number);
}
void drawcross(int x,int y,int w,int h,int r, int g, int b, int a)
{
	gEngfuncs.pfnFillRGBA( x - w, y , w * 2 , 2 ,r,g,b,a);
	gEngfuncs.pfnFillRGBA( x, y - h , 2 , h ,r,g,b,a);

	gEngfuncs.pfnFillRGBA( x - w, y-1 , w , 1 ,0,0,0,a);
	gEngfuncs.pfnFillRGBA( x + 2, y-1 , w - 2 , 1 ,0,0,0,a);
	gEngfuncs.pfnFillRGBA( x - w, y+2 , w * 2 , 1 ,0,0,0,a);

	gEngfuncs.pfnFillRGBA( x - w - 1, y-1 , 1 , 4 ,0,0,0,a);
	gEngfuncs.pfnFillRGBA( x + w, y-1 , 1 , 4 ,0,0,0,a);

	gEngfuncs.pfnFillRGBA( x-1, y-h , 1 , h-1 ,0,0,0,a);
	gEngfuncs.pfnFillRGBA( x+2, y-h , 1 , h-1 ,0,0,0,a);

	gEngfuncs.pfnFillRGBA( x-1, y-h-1 , 4 , 1 ,0,0,0,a);
}

static bool LoadOvRadar = false;
void overview_redraw()
{
	if(	/*!me.alive||*/ gEngfuncs.Con_IsVisible())return;

	if (!LoadOvRadar)
	{
		LoadOvRadar = true;
		overview_loadcurrent();
	}
	int size = (int)cvar.radars;
	
	ov_radar_x = (screeninfo.iWidth /2);
	ov_radar_y = (screeninfo.iHeight/2);
	gGui.window(cvar.radarx-size, cvar.radary-size, 2*size+2, 2*size+2, 1, "Radar");
	glViewport(cvar.radarx-size,clientopenglcalc(cvar.radary+size),(size*2),(size*2));	

	if (m_MapSprite)
	{
		float z  = ( 90.0f - mainViewAngles[0] ) / 90.0f;		
		z *= m_OverviewData.layersHeights[0]; // gOverviewData.z_min - 32;	

		float xStep =  (2*4096.0f /  ov_zoom ) / xTiles;
		float yStep = -(2*4096.0f / (ov_zoom*screenaspect) ) / yTiles;

		float vStepRight[2];
		float vStepUp[2];
		float angle = (float)((mainViewAngles[1]+90.0)* (PI/180.0));
		
		if(m_OverviewData.rotated)	{ angle -= float(PI/2); }

		vStepRight[0] = (float)cos( angle )*xStep;
		vStepRight[1] = (float)sin( angle )*xStep;
		vStepUp   [0] = (float)cos( angle + (PI/2))*yStep;
		vStepUp   [1] = (float)sin( angle + (PI/2))*yStep;

		float tile_x, tile_y;
		get_player_tile_coords( tile_x, tile_y );

		float xs = ov_radar_x - tile_x*vStepRight[0] - tile_y*vStepUp[0];
		float ys = ov_radar_y - tile_x*vStepRight[1] - tile_y*vStepUp[1];

		float inner[2];
		float outer[2];

		outer[0] = xs;
		outer[1] = ys;	

		int frame = 0;	

		gEngfuncs.pTriAPI->RenderMode( kRenderTransTexture );
		gEngfuncs.pTriAPI->CullFace( TRI_NONE );

		glEnable (GL_BLEND);

		glColor4f(1.0f, 1.0f, 1.0f,0.8f);//set alpha value here.000

		for (int ix = 0; ix < yTiles; ix++)
		{
			inner[0] = outer[0];
			inner[1] = outer[1];

			for (int iy = 0; iy < xTiles; iy++)	
			{
				gEngfuncs.pTriAPI->SpriteTexture( m_MapSprite, frame );
				gEngfuncs.pTriAPI->Begin( TRI_QUADS );
				gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
				gEngfuncs.pTriAPI->Vertex3f (inner[0], inner[1], z);

				gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
				gEngfuncs.pTriAPI->Vertex3f (inner[0]+vStepRight[0], inner[1]+vStepRight[1], z);

				gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
				gEngfuncs.pTriAPI->Vertex3f (inner[0]+vStepRight[0]+vStepUp[0], inner[1]+vStepRight[1]+vStepUp[1], z);

				gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
				gEngfuncs.pTriAPI->Vertex3f (inner[0]+vStepUp[0], inner[1]+vStepUp[1], z);
				gEngfuncs.pTriAPI->End();

				frame++;

				inner[0] += vStepUp[0];
				inner[1] += vStepUp[1];
			}
		
			outer[0] += vStepRight[0];
			outer[1] += vStepRight[1];
		}
		glDisable (GL_BLEND);
	}
	glViewport(0,0,screeninfo.iWidth,screeninfo.iHeight);

	oglSubtractive = true;
	drawcross(cvar.radarx,cvar.radary,size/8,size/8,255,255,255,254);			

	oglSubtractive = false;
}


///////////////////////////////////////////////////////////////////////////////////////////////

void VecRotateZ(float * in, float angle, float * out)
{
	float a,c,s;

	a = (float) (angle * PI/180);
	c = (float) cos(a);
	s = (float) sin(a);
	out[0] = c*in[0] - s*in[1];
	out[1] = s*in[0] + c*in[1];
	out[2] = in[2];
}

void overview_calcRadarPoint(float* origin, int * screenx, int * screeny)
{
	if(	/*!me.alive ||*/	gEngfuncs.Con_IsVisible	())	return;

	float aim [3],newaim [3];

	aim[0] = origin[0] - me.pmEyePos[0];
	aim[1] = origin[1] - me.pmEyePos[1];
	aim[2] = origin[2] - me.pmEyePos[2];

	VecRotateZ(aim, -mainViewAngles[1], newaim);

	*screenx = (cvar.radarx) - int( newaim[1]/ov_zoom * m_OverviewData.zoom * 0.3f * cvar.radars / 160);
	*screeny = (cvar.radary) - int( newaim[0]/ov_zoom * m_OverviewData.zoom * 0.4f * cvar.radars / 160);
}

void overview_draw_entity(float * origin, int radius, int r, int g, int b, int a)
{
	if(	/*!me.alive ||*/	gEngfuncs.Con_IsVisible	())	return;

	int x, y, d;

	overview_calcRadarPoint(origin, &x, &y);

	int radius2 = radius<<1;
	
	d = -4;

	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(x,y,radius2-d,radius2-d-1, r, g, b, a);
	oglSubtractive = false;
	whiteBorder(x,y,radius2-d,radius2-d-1);

}

void overview_draw_me(float * origin, int radius, int r, int g, int b, int a)
{
	if(	/*!me.alive				||*/
		gEngfuncs.Con_IsVisible	()
	)	return;
	int x, y;
	overview_calcRadarPoint(origin, &x, &y);
	int radius2 = radius<<1;
	drawcross(x,y,11,11,255,255,255,100);
}

///////////////////////////////////////////////////////////////////////////////////////////////

void DrawRadar()
{
	int size    = (int)cvar.radars;
	gGui.window(cvar.radarx-size, cvar.radary-size, 2*size+2, 2*size+2, 1, "Radar");

	gEngfuncs.pfnFillRGBA(cvar.radarx,cvar.radary-size,1,2*size,255,255,255,180);
	gEngfuncs.pfnFillRGBA(cvar.radarx-size,cvar.radary,2*size,1,255,255,255,180);
}

void drawRadarPoint(float* origin,int r,int g,int b,int a,bool blink=false,int boxsize=3)
{
	int screenx;
	int screeny;

	if (cvar.radar == 2 && mapLoaded)
	{
		overview_calcRadarPoint(origin, &screenx, &screeny);
		if ( screenx>(cvar.radarx+cvar.radars-boxsize-1)) screenx = cvar.radarx+cvar.radars-boxsize-1;
		else if (screenx<(cvar.radarx-cvar.radars-1)) screenx = cvar.radarx-cvar.radars-1;

		if (screeny>(cvar.radary+cvar.radars-boxsize-1)) screeny = cvar.radary+cvar.radars-boxsize-1;
		else if (screeny<(cvar.radary-cvar.radars-1)) screeny = cvar.radary-cvar.radars-1;
			
		//if(	!me.alive ) return;
	}
	else
	{
		float dx  = origin[0] - me.pmEyePos[0];
		float dy  = origin[1] - me.pmEyePos[1];

		float yaw = mainViewAngles[1]*(3.1415/180.0);
		me.sin_yaw       =  sin(yaw); 
		me.minus_cos_yaw = -cos(yaw);

		// rotate
		float x =  dy*me.minus_cos_yaw + dx*me.sin_yaw;
		float y =  dx*me.minus_cos_yaw - dy*me.sin_yaw;

		float range = 2500.0f;
		if(fabs(x)>range || fabs(y)>range)
		{ 
			// clipping
			if(y>x)
			{
				if(y>-x) {
					x = range*x/y;
					y = range;
				}  else  {
					y = -range*y/x; 
					x = -range; 
				}
			} else {
				if(y>-x) {
					y = range*y/x; 
					x = range; 
				}  else  {
					x = -range*x/y;
					y = -range;
				}
			}
		}
		screenx = cvar.radarx+int(x/range*float(cvar.radars));
		screeny = cvar.radary+int(y/range*float(cvar.radars));		
	}
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(screenx-1,screeny-1,boxsize,boxsize,r,g,b,a);
	oglSubtractive = false;
	blackBorder(screenx-1,screeny-1,boxsize,boxsize+1);
}
