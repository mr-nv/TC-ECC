#ifndef _MENU_
#define _MENU_

#include <vector>
using namespace std;

struct MenuEntry
{
	int iStep, iMax, iValue, iMin;
	float fStep, fMax, *fValue, fMin;
	char* cCVarName;
	bool bIsInt;
};

class cMenu
{
public:
	void Init();
	void Draw(int x, int y);
	int Key(int keynum);
	bool Activated;
private:
	void AddInt(int step, int max, int value, char* name, int min);
	void AddFloat(float step, float max, float *value, char* name, float min);
	vector<MenuEntry> Entrys;
	int iSelected;
};
extern cMenu gMenu;

#endif