#include "hwcpu.h"
#include <assert.h>

#define HWCPUFUNC_GET_RAM_AMOUNT 0
#define HWCPUFUNC_GET_IRQ_QUEUE_SIZE 1
#define HWCPUFUNC_SET_MMU_TABLE 2
#define HWCPUFUNC_SET_INTR_SAVEADDR 3
#define HWCPUFUNC_SET_INTR_LOADADDR 4

u32 cpu_getRamAmount(void)
{
	HwiData data;
	int res = hwiCall(HWBUS_CPU, HWCPUFUNC_GET_RAM_AMOUNT, &data);
	always_assert(res==HWIERR_SUCCESS);
	return data.regs[0];
}

int cpu_getIRQQueueSize(void)
{
	HwiData data;
	int res = hwiCall(HWBUS_CPU, HWCPUFUNC_GET_IRQ_QUEUE_SIZE, &data);
	always_assert(res==HWIERR_SUCCESS);
	return data.regs[0];
}

void cpu_setMMUTableAddress(void* tbl, u32 numpages)
{
	HwiData data;
	data.regs[0] = (u32)tbl;
	data.regs[1] = numpages;
	int res = hwiCall(HWBUS_CPU, HWCPUFUNC_SET_MMU_TABLE, &data);
	always_assert(res==HWIERR_SUCCESS);
}

void cpu_setInterruptContexts(Ctx* save, Ctx* load)
{
	HwiData data;
	data.regs[0] = (u32)save;
	int res = hwiCall(HWBUS_CPU, HWCPUFUNC_SET_INTR_SAVEADDR, &data);
	always_assert(res==HWIERR_SUCCESS);
	data.regs[0] = (u32)load;
	res = hwiCall(HWBUS_CPU, HWCPUFUNC_SET_INTR_LOADADDR, &data);
	always_assert(res==HWIERR_SUCCESS);
}

