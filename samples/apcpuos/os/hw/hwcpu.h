/*******************************************************************************
* CPU driver
*******************************************************************************/

#ifndef _APCPU_HWCPU_H_
#define _APCPU_HWCPU_H_

#include "hwcommon.h"

#define HW_CPU_FUNC_GETRAMAMOUNT 0
#define HW_CPU_FUNC_GETIRQQUEUESIZE 1
#define HW_CPU_FUNC_SETMMUTABLE 2

typedef struct CpuCtx
{
	uint32_t gregs[CPU_NUM_GREGS]; // general purpose registers
	uint32_t rims[2];
	double fregs[CPU_NUM_FREGS]; // floating point registers
	uint32_t crregs[CPU_NUM_CRREGS];
} CpuCtx;


hw_Drv* hw_cpu_ctor(hw_BusId);
void hw_cpu_dtor(hw_Drv* drv);

/*
 * Returns the lowest 32 bits of the cpu tick counter
 */
uint32_t hw_cpu_getCycles32(void)
INLINEASM("\t\
rdtsc r0:r0");

/*
 * Disabled further IRQs
 */
void hw_cpu_disableIRQ(void)
INLINEASM("\t\
mrs r0, flags ; load flags register\n\t\
or r0, r0, 1<<27 ; set bit 27 \n\t\
msr flags, r0 ; set flags register");

uint32_t hw_cpu_getFlagsRegister(void)
INLINEASM("\t\
mrs r0, flags ; load flags register");

void hw_cpu_setFlagsRegister(__reg("r0") uint32_t flags)
INLINEASM("\t\
msr flags, r0 ; set flags register");

// TODO : Is this correct with the new cpu refactoring?
uint32_t hw_cpu_nextIRQ(
	__reg("r0") int busid,
	__reg("r4") u32* data0, __reg("r5") u32* data1)
INLINEASM("\t\
nextirq r0\n\t\
; ip will contain 0 if no IRQ was retrieved, (bus<<24)|reason if it was\n\t\
str [r4],r0\n\t\
str [r5],r1\n\t\
mov r0, ip\n\t\
");

/*
 * Enables IRQs
 */
void hw_cpu_enableIRQ(void)
INLINEASM("\t\
mrs r0, flags ; load flags register \n\t\
and r0, r0, ~(1<<27) ; clear bit 27 \n\t\
msr flags, r0 ; set flags register");

/*! Sets the process keys in the flags register
*/
void hw_cpu_setProcessKeys(__reg("r0") int val)
INLINEASM("\t\
mrs r1 ; load flags register \n\t\
and r1, r1, 0xFF000000 ; clear the key bits \n\t\
or r1, r1, r0 ; Set the bit keys \n\t\
msr r1 ; set flags register");

 
/*
 * Emits a dbgbrk instruction
 */
void hw_cpu_dbgbrk(void)
INLINEASM("\t\
dbgbrk");

/*
 * Emits a hlt instruction
 */
void hw_cpu_hlt(void)
INLINEASM("\t\
hlt");

/*
 * Emits a ctwswitch instruction
 */
int hw_cpu_ctxswitch(__reg("r0") CpuCtx* new, __reg("r1") CpuCtx* curr)
INLINEASM("\t\
ctxswitch [r0], [r1]");

const char* hw_cpu_getIntrReasonMsg(uint32_t reason);

/*
 * Returns how much ram the system has, in bytes
 */
#define hw_cpu_getRamAmount() \
	hw_hwiSimple0(HWBUS_CPU, HW_CPU_FUNC_GETRAMAMOUNT)
	
/*
 * Returns how many queued up IRQs we have
 */
#define hw_cpu_getIRQQueueSize() \
	hw_hwiSimple0(HWBUS_CPU, HW_CPU_FUNC_GETIRQQUEUESIZE)

/*
 * Checks if the specified key is pressed.
 * NOTE: Not all keys can be checked for this
 * \param key Key to check.
 * \return TRUE or FALSE
 */
#define hw_cpu_setMMUTable(mmuTablePtr, mmuNumEntries) \
	hw_hwiSimple2(HWBUS_CPU, HW_CPU_FUNC_SETMMUTABLE, mmuTablePtr, mmuNumEntries)

#endif
