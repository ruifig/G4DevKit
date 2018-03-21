#include "hwscreen.h"
#include "hwclock.h"
#include "coroutine.h"
#include <stdlib.h>
#include <string.h>

extern Ctx interruptedCtx;
void* interruptHandler(u32 p0, u32 p1, u32 p2, u32 p3, u32 reason)
{
	return &interruptedCtx;
}

Coroutine co1, co2;
char stack1[1024], stack2[1024];

// We use "yielded" as the row to print at
// "cookie" is the other coroutine we want to switch to
int run(int yielded, int cookie)
{
	int x;
	for(x=0; x<30; x++)
	{
		if (x==20)
			co_destroy((Coroutine*)cookie, 255);
		scr_printfAtXY(x, yielded, " Coroutine %d", yielded);
		clk_pauseMS(100);
		co_yield((Coroutine*)cookie, yielded==1 ? 2 : 1);
	}
	scr_printfAtXY(x, yielded, " Finished ");
	return yielded;
}

int main(void)
{
	scr_init();
	scr_printfAtXY(0,0,"Coroutines example");
	
	co_create(&co1, stack1, sizeof(stack1), &run, (int)&co2); // Cookie is co2
	co_create(&co2, stack2, sizeof(stack2), &run, (int)&co1); // Cookie is co1

	// switch execution to first coroutine in the chain
	int yielded = 1;
	// Wait until all coroutines finish
	while(co_hasAlive())
	{
		yielded = co_yield(NULL, yielded);
	}
	
	scr_printfAtXY(0,4, "Result=%d", yielded);
	return 0;
}
