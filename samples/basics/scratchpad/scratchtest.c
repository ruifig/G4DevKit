#include <stddef.h>

typedef signed char s8;
typedef unsigned char u8;
typedef signed int s32;
typedef unsigned int u32;

/*
 * Load control register
 */
u32 cpu_get_crirqmsk(void)
INLINEASM("\t\
mrs r0, crirqmsk");

void cpu_set_crirqmsk(u32 val)
INLINEASM("\t\
msr crirqmsk, r0");

void cpu_(u32 val)
INLINEASM("\t\
msr crirqmsk, r0");

u32 hwf(u32 busId, u32 func, u32 r0, u32 r1);

void* interruptHandler(u32 busId)
{
	// Renable IRQs
	//cpu_set_crirqmsk(1<<3);
	hwf(3, 0, 1, 2);
	return NULL;
}

int main(void)
{
	u32 v = cpu_get_crirqmsk();
	cpu_set_crirqmsk(1<<3);
	
	return 0;
}
