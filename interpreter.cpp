//==============================================================================
// interpreter.cpp
//==============================================================================
#pragma warning(disable:4786)
#pragma warning(disable:4800)
#pragma warning(disable:4244)
#pragma warning(disable:4101)
#pragma warning(disable:4715)
#pragma warning(disable:4305)
#include "interpreter.h"
#include "cvar.h"
#include <windows.h>
#include "client.h"
#include <fstream>
void Con_Echo(const char *fmt, ... );
//========================================================================================

CommandInterpreter cmd;
extern char hackdir[256];
//#define cmd yxvkjn yxcvkjna
//==============================================================================
inline std::string getHackDirFile(const char* basename)
{
	if(strstr(basename,"..")){ return ":*?\\/<>\""; }
	string ret = hackdir;
	return (ret+basename);
}
//==============================================================================

void HlEngineCommand(const char* command)
{
	if(!gEngfuncs.pfnClientCmd) { return; }
	gEngfuncs.pfnClientCmd( const_cast<char*>(command) );
}

//==============================================================================

void HandleCvarFloat(char* name, float* value)
{
	char* arg1 = cmd.argC(1);  
	if (!strcmp(arg1,"change")) 
	{
		if (*value) *value=0; else *value=1; 
		return;
	}
	if (!strcmp(arg1,"up"))     
	{ 
		*value += cmd.argF(2); 
		return;
	}
    if (!strcmp(arg1,"down"))   
	{ 
		*value -= cmd.argF(2); 
		return;
	}
	if (!strcmp(arg1,"hide")) 
    { 
        *value = cmd.argF(2);
        return;
    }
	if (!*arg1) 
	{ 
		Con_Echo( "CVAR %s = %f\n",name,*value); 
		return;
	}
    *value = cmd.argF(1);
}

//==============================================================================

bool isHlCvar(char* name)
{
	if(!gEngfuncs.pfnGetCvarPointer) { return false; }
	
	cvar_s* test = gEngfuncs.pfnGetCvarPointer(name);
	return (test!=NULL);
}

//==============================================================================

void HandleHlCvar(char* name)
{
	if(!gEngfuncs.pfnGetCvarPointer) { return; }

	cvar_s* ptr = gEngfuncs.pfnGetCvarPointer(name);
	if(!ptr) return;

	HandleCvarFloat(name,&ptr->value);
}
//========================================================================================
void CommandInterpreter::logExec(const char* command)
{
	Con_Echo("FUNCTION %s CALL ArgCount %d\n",command,preExecArgs.size() );

	string collect;
	for(int i=0;i<preExecArgs.size();i++)
	{
		collect += " [";
		collect += preExecArgs[i];
		collect += "] ";
	}
	Con_Echo("Arguments %s", collect.c_str() );
} 

//========================================================================================
void CommandInterpreter::exec(const char* cmdlist)
{
	string my_copy = cmdlist;
	// find end:
	char* from = const_cast<char*>(my_copy.c_str());
	char* to   = from;	
	
	while(*from==' '||*from=='\t'){ ++from;++to; } // skip ws
	while(*to>=' ' && *to!= ';'){ // find end
		if(*to=='\"')
		{
			do{ ++to; }while(*to && *to!='\"');
		}
		else
		{
			++to;
		}
	}
	
	do{
		// comments...
		if(from[0]=='/' && from[1]=='/') { return; }

		// split up and exec
		if(from<to)
		{
			char oldch = *to;
			*to = 0;
			exec_one(from);
			*to = oldch;
		}
		
		// advance
		if(!*to) { break; }
		++to; 
		from = to;
		while(*from==' '||*from=='\t'){ ++from;++to; }  // skip ws
		while(*to>=' ' && *to!= ';') ++to;              // find end
	} while(1);
}

//========================================================================================
void CommandInterpreter::extractArguments(const char* const_args)
{
	preExecArgs.clear();

	char* args = const_cast<char*>(const_args);

	while(*args)
	{
		while(*args && *args<=' ') args++;
		if(!*args) break;

		char* start;
		if(*args=='\"') { start = ++args; while(*args!='\"' && *args ) args++;  }
		else            { start = args;   while(*args>' ') args++; }

		char last = *args;
		*args = 0;
		preExecArgs.push_back(start);
		*args = last;
		if(*args=='\"') args++;
	}
}

//========================================================================================
void CommandInterpreter::collectArguments(string& dest,int from,int to)
{
	dest.erase();
	--from;--to;

	int nArgs = preExecArgs.size();
	to = (to<nArgs) ? to : (nArgs-1);
	
	while(from<=to){
		dest += preExecArgs[from];
		if(from!=to) dest += " ";
		++from;
	};
}

//========================================================================================
void CommandInterpreter::createRandomPrefix()
{
	static char characterBox[] = "0123456789abcdefghijklmnopqrstuvwxyz"
		                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&/()=?{}[]*#-.,<>~+_";
	static int len = sizeof(characterBox)-1;
	
	excludePrefixChar[0] = characterBox[rand()%len];
	excludePrefixChar[1] = characterBox[rand()%len];
	excludePrefixChar[2] = characterBox[rand()%len];
	excludePrefixChar[3] = characterBox[rand()%len];
	excludePrefixChar[4] = 0;
}

void CommandInterpreter::exec_one(const char* cur_cmd)
{
	if(*cur_cmd=='#' || *cur_cmd=='.')
	{
		if( false )
		{
			static string hlcommand;
			hlcommand.erase();
			hlcommand += excludePrefixChar;
			hlcommand += (cur_cmd+1);
			HlEngineCommand(hlcommand.c_str());
		} 
		else 
		{
			HlEngineCommand(cur_cmd+1);
		}
		return;
	}

	// extract command
	char  command[32];
	char* commandPos = command;
	int   commandCharsLeft = 31;
	while(*cur_cmd>' ' && commandCharsLeft)
	{
		*commandPos = tolower(*cur_cmd);
		commandPos++;
		cur_cmd++;
		commandCharsLeft--;
	}
	*commandPos = 0;
	while(*cur_cmd>' ') cur_cmd++; // skip parts bigger than 31 chars.

	if(names.find(command))
	{
		Entry& entry = entries[names.num];

		switch(entry.type)
		{
		case Entry::ALIAS:{
			string& content = *(string*)(entry.data);
			exec(const_cast<char*>(content.c_str()));
						  }break;
		case Entry::COMMAND:{
			typedef void (*CmdFunc)();
			CmdFunc function = (CmdFunc)(entry.data);
			extractArguments(cur_cmd);
			function();
							}break;
		case Entry::CVAR_FLOAT:
			extractArguments(cur_cmd);
			HandleCvarFloat( command, (float*)entry.data );
			break;
		case Entry::HL_CVAR:
			extractArguments(cur_cmd);
			HandleHlCvar(command);
			break;
		}
	} else {
		if(!isHlCvar(command))
		{
			Con_Echo("Unknown command &w%s",command);
		}
		else
		{
			Add(command,Entry::HL_CVAR,NULL);
			extractArguments(cur_cmd);
			HandleHlCvar(command);
		}
	}
}

//========================================================================================
void CommandInterpreter::execFile(const char* filename)
{
	if(strstr(filename,"..")) 
	{
		Con_Echo( "forbidden.\n" );
		return;
	}

	ifstream ifs(filename);
	if(!ifs)
	{
		return;
	}

	char buf[1024];
	while(ifs)
	{
		ifs.getline(buf,550,10);
		if(!ifs) { break; }
		exec(buf);
	}
}

//========================================================================================
void  CommandInterpreter::Add( const char *name, int type, void* data  )
{
	if(names.find(name))
	{
		Con_Echo("&w%s&a is already registered.\n",name);
		return;
	}

	int index = entries.size();
	Entry tmp = {type,data};
	entries.push_back(tmp);
	names.add(name,index);
}

//========================================================================================
inline static void lowercase(char* str)
{
	while(*str){ *str = tolower(*str); ++str; }
}

//========================================================================================
void  CommandInterpreter::AddAlias    ( const char *NamE, string& newcontent )
{
	// lowercase and cut
	char name[36];
	strncpy(name,NamE,31);
	name[31]=0;
	lowercase(name);
	
	if(names.find(name))
	{
		Entry& entry = entries[names.num];
		if(entry.type!=Entry::ALIAS)
		{
			Con_Echo("&w%s&a is already registered.\n",name);
			return;
		}

		*(string*)(entry.data) = newcontent;
		return;
	}
	
	if(isHlCvar(name))
	{
		Con_Echo("&w%s&a is a HL cvar.\n",name);
		return;
	}

	int index = entries.size();
	
	string* initial_content = new string;
	*initial_content = newcontent;

	Entry tmp = {Entry::ALIAS, initial_content};
	entries.push_back(tmp);
	names.add(name,index);
}

//========================================================================================

void CommandInterpreter::save_cvars(std::ofstream& ofs)
{	
	ofs<<"[CVAR]\n";
	
	for(names.it_start();names.it_running();names.it_next())
	{
		Entry& r = entries[names.num];
		if( r.type==Entry::CVAR_FLOAT )	ofs<<names.str<<"="<<*(float*)r.data<<"\n";
	}
}

//========================================================================================
inline std::string getHackDirFile(const char* basename);
void CommandInterpreter::load_cvars(void)
{	
	char cvar_f[100];
	DWORD size=500;

	for(names.it_start();names.it_running();names.it_next())
	{
		Entry& r = entries[names.num];

		if( r.type==Entry::CVAR_FLOAT )
		{
			GetPrivateProfileString("CVAR",names.str,"NULL",cvar_f,size,getHackDirFile("init.ini").c_str());
			*(float*)r.data = atof(cvar_f);
		}
	}
}
//========================================================================================