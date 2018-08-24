
#include "common.h"
#include "hwcommon.h"
#include "hwscreen.h"
#include "hwclock.h"
#include "hwcpu.h"
#include <stdlib.h>
#include <string.h>

int line=1;
u32* mmuTable;
u32 mmuTableEntries;

void doWork()
{
	for(int x=0; x<(SCR_XRES-30); x++)
		for(int y=0; y<SCR_YRES; y++)
		{
			scr_printfAtXY(x,y," Testing %06d, %02d:%02d %3.3f ", x*y, x,y, (float)x/(y+1));
		}
}

// Calculate the MMU table size in bytes
// Called from the assembly boot code
u32 calcMMUTableSize(void)
{
	u32 ramAmount = cpu_getRamAmount();
	mmuTableEntries = ramAmount / MMU_PAGE_SIZE;
	return mmuTableEntries * sizeof(u32);
}

#define MMU_ACCESS_R (1 << 9)
#define MMU_ACCESS_W (1 << 8)
#define MMU_ACCESS_X (1 << 7)
#define MMU_ACCESS_ALL (MMU_ACCESS_R | MMU_ACCESS_W | MMU_ACCESS_X)

void initMMUTable(void)
{
	u32 key = 1;
	cpu_setKey(key);
	for (u32 i = 0; i < mmuTableEntries; i++)
	{
		mmuTable[i] = (i * MMU_PAGE_SIZE) | MMU_ACCESS_ALL | key;
	}
}

void appMain(void)
{
	//initMMUTable();
	//cpu_setMMUTableAddress(mmuTable, mmuTableEntries);

	int x,y;
	scr_init();
	float f1;
	float f2;
	
	double startTime = clk_getRunningTimeSeconds();
	
	while(1)
	{
		doWork();
	}
	#if 0
	double endTime = clk_getRunningTimeSeconds();
	uint32_t cycles = cpu_getCycles32();
	
	scr_clear();
	scr_printfAtXY(0,0, "Duration: %3.4f", endTime-startTime);
	scr_printfAtXY(0,1, "Cycles: %u", cycles);
	scr_printfAtXY(0,2, "Mhgz  : %4.4f", ((double)cycles/(endTime-startTime))/(1000*1000));
		
	#endif
	
	loopForever();
}
