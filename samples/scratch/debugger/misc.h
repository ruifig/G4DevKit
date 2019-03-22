#ifndef _misc_h__
#define _misc_h__

extern char* gScreenAddr;
char* getScreenAddr(void);
void debugBreak(void);
void initMisc(void);
void printAtXy(int x, int y, const char* str);

#endif
