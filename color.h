#ifndef _COLOR_
#define _COLOR_

#include <vector>
using namespace std;

struct ColorEntry
{
	int r, g, b, a;
	float onebased_r,onebased_g,onebased_b,onebased_a;
};

class ColorManager
{
public:
	ColorManager(){ Init(); }
	ColorEntry * get(int index);
private:
	int currentIndex;
	vector<ColorEntry> entrys;
	void add(int r, int g, int b, int a);
	void Init();
};
extern ColorManager colorList;

#endif