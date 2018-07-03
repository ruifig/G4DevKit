#include "mmu_test_utils.h"
#include "hwcpu.h"
#include <string.h>

u32 intrCount;
u32 intrBus;
u32 intrReason;

static u32 lastIntrCount = 0;
extern u32 intrData[4];
extern bool mmuEnabled;

static u32 doWriteB(void* addr, u8 v)
INLINEASM("\t\
strb [r0], r1");
static u32 doWriteH(void* addr, u16 v)
INLINEASM("\t\
strh [r0], r1");
static u32 doWriteW(void* addr, u32 v)
INLINEASM("\t\
str [r0], r1");

static u8 doReadB(void* addr)
INLINEASM("\t\
mov r1, 0xFFFFFFFF\n\t\
ldrub r1, [r0]\n\t\
mov r0, r1");

static u16 doReadH(void* addr)
INLINEASM("\t\
mov r1, 0xFFFFFFFF\n\t\
ldruh r1, [r0]\n\t\
mov r0, r1");

static u32 doReadW(void* addr)
INLINEASM("\t\
mov r1, 0xFFFFFFFF\n\t\
ldr r1, [r0]\n\t\
mov r0, r1");

static u32 doExecute(void* addr)
INLINEASM("\t\
bl r0");

bool writeB(void* addr, u8 val)
{
	doWriteB(addr, val);
	return intrCount==lastIntrCount ? true : false;
}

bool writeH(void* addr, u16 val)
{
	doWriteH(addr, val);
	return intrCount==lastIntrCount ? true : false;
}

bool writeW(void* addr, u32 val)
{
	doWriteW(addr, val);
	return intrCount==lastIntrCount ? true : false;
}

bool readB(void* addr, u8 val)
{
	u8 v = doReadB(addr);
	return (intrCount==lastIntrCount && v==val) ? true : false;
}

bool readH(void* addr, u16 val)
{
	u16 v = doReadH(addr);
	return (intrCount==lastIntrCount && v==val) ? true : false;
}

bool readW(void* addr, u32 val)
{
	u32 v = doReadW(addr);
	return (intrCount==lastIntrCount && v==val) ? true : false;
}

bool tryReadB(void* addr)
{
	doReadB(addr);
	return intrCount==lastIntrCount ? true : false;
}

bool tryReadH(void* addr)
{
	doReadH(addr);
	return intrCount==lastIntrCount ? true : false;
}

bool tryReadW(void* addr)
{
	doReadW(addr);
	return intrCount==lastIntrCount ? true : false;
}

bool execute(void* addr)
{
	doExecute(addr);
	return intrCount==lastIntrCount ? true : false;
}

bool writeAndReadB(void* addr, void* readAddr, u8 val)
{
	return writeB(addr,val) && readB(readAddr ? readAddr : addr, val);
}

bool writeAndReadH(void* addr, void* readAddr, u16 val)
{
	return writeH(addr, val) && readH(readAddr ? readAddr : addr, val);
}

bool writeAndReadW(void* addr, void* readAddr, u32 val)
{
	return writeW(addr,val) && readW(readAddr ? readAddr : addr, val);
}

const char* checkOk(bool res)
{
	return res==true? "OK" : "FAIL";
}

const char* checkFail(bool res, void* addr, Access access)
{
	u32 nextPageAddr = (u32)PAGE_TO_ADDR(ADDR_TO_PAGE(addr)+1);
#if 0
	scr_printf("CHECK(%u, %u) bus=%u, reason=%u, data=%u,%u,%u,%u, next=%u\n",
		address, (u32)access,
		intrBus, intrReason, intrData[0], intrData[1], intrData[2], intrData[3], nextPageAddr);
#endif
	
	const char* ret;
	if (
		(intrCount==lastIntrCount+1) && intrReason==Cpu_Abort &&
		((u32)addr==intrData[0] || (mmuEnabled && (intrData[0]==nextPageAddr))) &&
		access==intrData[1] )
	{
		ret = "OK";
	}
	else
	{
		ret = "FAIL";
	}
	
	lastIntrCount = intrCount;
	intrReason = 0xFFFFFF;
	memset(intrData, 0, sizeof(intrData));
	return ret;
}

