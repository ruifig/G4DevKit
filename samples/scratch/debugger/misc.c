#include "misc.h"

#if 0
char* gScreenAddr;

void initMisc(void)
{
	gScreenAddr = getScreenAddr();
}

static unsigned short* scr_getXYPtr(int x, int y)
{
	return (unsigned short*)(gScreenAddr + (y*(80*2) + x*2));
}

void printAtXy(int x, int y, const char* str)
{
	unsigned short* ptr = scr_getXYPtr(x,y);
	while(*str) {
		*ptr = 0xF00|*str;
		ptr++; str++;
	}
}

#endif
