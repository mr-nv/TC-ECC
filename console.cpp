//==============================================================================
// Console.cpp
//==============================================================================

#pragma warning(disable:4786)
#pragma warning(disable:4800)
#pragma warning(disable:4244)
#pragma warning(disable:4101)
#pragma warning(disable:4715)
#pragma warning(disable:4305)

#include <windows.h>
#include <vector>
#include "console.h"
#include "client.h"
#include "stringfinder.h"
#include "interpreter.h"
#include "color.h"
#include "cvar.h"
#include "drawing.h"
#include "gui.h"
#include "TableFont.h"


// global
Console gConsole;

//========================================================================================
void Console::draw(int con_x, int con_y, int con_w, int con_h)
{
	if( !active ) return;

	ColorEntry* color;
/*	ColorEntry* color1 = colorList.get(0); // "menu1"	
	
	dddBorder(con_x, con_y-16,con_w,15,254); //Title border
	tintArea( con_x, con_y-16,con_w,14,color1->r,color1->g,color1->b,color1->a); //Title background
	DrawConStringCenter(con_x + con_w/2, con_y-21,255,255,40,"Console"); //Title
	blackBorder(con_x,con_y-2,con_w,con_h+3);	//Big Border
	dddBorder2(con_x+3, con_y+con_h-13,con_w-6,12,254); //Edit Border
	tintArea(con_x,con_y-2,con_w,con_h+2,color); //Big BackGround*/
	gGui.window(con_x,con_y+2,con_w,con_h+2, 0.5, "Console");
	tintArea(con_x, con_y+con_h-13,con_w,1,255,255,255,255);

	int x = con_x+3;
	int y = con_y+con_h-14;

	// advance blink:
	if(blinkTimer.expired())
	{
		blink = !blink;
		if(blink) { blinkTimer.countdown(0.2); }
		else      { blinkTimer.countdown(0.2); }
	}
	
	// draw cursor - that shit doesnt work at all :P
/*	if(blink)
	{
		// get len of text to cursor.
		int length, height, ch_length;
		char  save;
		save = cursorpos[0]; cursorpos[0]=0;
		gEngfuncs.pfnDrawConsoleStringLen( editbuf, &length, &height );
		cursorpos[0]=save;

		// get cursor size:
		save = cursorpos[1]; cursorpos[1]=0;
		gEngfuncs.pfnDrawConsoleStringLen( cursorpos, &ch_length, &height );
		cursorpos[1]=save;
		if(!*cursorpos) ch_length=5;
        
        tintArea(con_x+3, con_y+con_h-13,ch_length,height-5, colorList.get(3)); // "con_text2"
	}*/
	
	color = colorList.get(3); // "con_edit"
	gFont.drawString(false,x,y+5,color->r, color->g, color->b, editbuf);

	lines.reset();
	for(;;)
	{
		y-=11;
		if(y<con_y) break;
		
		string& curLine = lines.read(); 
		lines.prev();

		drawConsoleLine(curLine,x,y);
	}
}
//========================================================================================
void Con_Echo(const char *fmt, ... )
{
	va_list va_alist;
	char buf[256];

	va_start (va_alist, fmt);
	_vsnprintf (buf, sizeof(buf), fmt, va_alist);
	va_end (va_alist);

	// echo + scroll up:
	gConsole.echo("%s",buf);
}
//========================================================================================
void Console::echo(const char *fmt, ... )
{
	va_list va_alist;
	char buf[384];

	va_start (va_alist, fmt);
	_vsnprintf (buf, sizeof(buf), fmt, va_alist);
	va_end (va_alist);

	lines.add( buf );
}

//========================================================================================
void Console::say(const char* text, const char* name, int team )
{
	if(team==1) { echo("&r%s :", name); echo(" %s",text); }
	else        { echo("&b%s :", name); echo(" %s",text); }
}

//========================================================================================
void Console::drawConsoleLine( const string& str, int x, int y )
{
	const char* line = str.c_str();
	char  buf[256];
	char* bufpos;

	for(;;)
	{
		// extract string part
		bufpos=buf;
		for(;;) { *bufpos=*line; if(!*line||*line=='&')break;  ++line; ++bufpos; };
		bufpos[0]=0;bufpos[1]=0;

		// draw
		int length, height;
		gEngfuncs.pfnDrawConsoleStringLen( buf, &length, &height );
		gFont.drawString(false,x,y,colorTags(2)->r, 
								   colorTags(2)->g, 
								   colorTags(2)->b,
								   buf);
		// advance
		x+=length;
		if(*line=='&')
		{
			unsigned char ch = *++line - 'a';
			if(ch<26) curColorTag=ch;
			else         break;
			if(!*++line) break;
		}
		else 
		{
			break;
		}
	}
	curColorTag=0;
}

//========================================================================================

void Console::key(int ch, bool shift)
{
	char* pos;
	if( GetAsyncKeyState(VK_SHIFT) && shift )
	{
		if (ch == 45)
		{
			key('_',false);
		}
		return;
	}
	else
	{
		switch(ch)
		{
		case 127: // backspace
			if(cursorpos==editline) return;
			pos = --cursorpos;
			while(pos[0]) { pos[0]=pos[1]; ++pos; }
			return;
			
		case 128: // uparrow
			if(hist_direction!=DIR_BACK) { history.prev(); history.prev(); hist_direction=DIR_BACK;}
			strcpy(editline, history.read().c_str());
			cursorpos = editline + strlen(editline);
			history.prev();
			return;

		case 129: // downarrow
			if(hist_direction==DIR_BACK) { history.next(); history.next(); hist_direction=DIR_FORWARD;}
			strcpy(editline, history.read().c_str());
			cursorpos = editline + strlen(editline);
			history.next();
			return;
		
		case 130: // leftarrow
			if(cursorpos!=editline) --cursorpos;
			return;

		case 131: // righttarrow
			if(cursorpos!=(editline+strlen(editline))) ++cursorpos;
			return;

		case 13:
			if( !strcmp(editline,"===") )
			{
				if(mode==MODE_EXECUTE) { mode=MODE_CHAT;    echo("&b*** &aCONSOLE: &wCHAT MODE &b***"); }
				else                   { mode=MODE_EXECUTE; echo("&b*** &aCONSOLE: &wEXEC MODE &b***"); }
			}
			else if(mode==MODE_EXECUTE)
			{
				echo    ( "&x%s",editbuf );
				if(editline[0])
				{
					cmd.exec( editline );
					history.add(editline);
					history.reset();
				}
			} 
			else if(mode==MODE_CHAT)
			{
				char* text = editline; while(*text==' ')++text;
				char buf[256];sprintf(buf,"say \"%s\"",text);
				gEngfuncs.pfnClientCmd(buf);
			}	
			editline[0]=0;
			cursorpos = editline;
			return;
		default:
			// insert character
			if(strlen(editbuf)>(EDIT_MAX-4)) return;
			if(!cursorpos[0]) { cursorpos[0]=ch; ++cursorpos; cursorpos[0]=0; return; }

			pos = editbuf+strlen(editbuf)+1; 
			while(pos>cursorpos) { pos[0]=pos[-1]; --pos; }

			*cursorpos = ch;
			++cursorpos;
			return;
		}
	}
}


