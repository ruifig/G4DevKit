#include "common.h"
#include "hwcpu.h"
#include "hwscreen.h"
#include "string.h"
#include "assert.h"
#include "hwkeyboard.h"
#include "mmu_test_utils.h"

// The assembly interrupt handler sets this whenever an IRQ interrupt happens
u32 intrData[4];
u32 ramAmount;
bool mmuEnabled = false;

void doPause(void)
{
	scr_printf("Press any key to continue...\n");
	kyb_pause();
}

u32 exceptionCounter[Cpu_MAX];
extern Ctx interruptedCtx;

#define MMU_ACCESS_R (1 << 9)
#define MMU_ACCESS_W (1 << 8)
#define MMU_ACCESS_X (1 << 7)
#define MMU_ACCESS_ALL (MMU_ACCESS_R | MMU_ACCESS_W | MMU_ACCESS_X)

#define MMU_TABLE_NUMPAGES 1024
u32 mmuTable[MMU_TABLE_NUMPAGES];

void mmu_setPage(u32 vpage, u32 ppage, u32 access, u32 key)
{
	mmuTable[vpage] = ( ppage * MMU_PAGE_SIZE) | access | key;
}

//
// The first virtual pages up to the number of physical pages are mapped 1 to 1
//
void initMMUTable(void)
{
	u32 numppages = SIZE_TO_PAGES(ramAmount);
	
	// This sample requires that the total memory be below our allocated mmu
	// table, by a couple of pages.
	always_assert(numppages < MMU_TABLE_NUMPAGES-10);
	
	//
	// The physical range, is 1 to 1, with access to all by default
	for(u32 vp = 0; vp < numppages; vp++)
		mmu_setPage(vp, vp, MMU_ACCESS_ALL, 0);
	
	// The rest of the virtual range nas no access at all by default
	for(u32 vp = numppages; vp < MMU_TABLE_NUMPAGES; vp++)
		mmu_setPage(vp, vp, 0, 0x7F);
		
	u32 codePages = SIZE_TO_PAGES(processInfo.readOnlySize);
	u32 dataPages = SIZE_TO_PAGES(processInfo.readWriteSize);
	u32 sharedPages = SIZE_TO_PAGES(processInfo.sharedReadWriteSize);
	u32 totalPages = codePages + dataPages + sharedPages;
	
	u32 key = 1;
	//
	// Code + read only data
	//
	u32 vp= ADDR_TO_PAGE(processInfo.readOnlyAddr);
	u32 count= SIZE_TO_PAGES(processInfo.readOnlySize);
	while(count--)
	{
		mmu_setPage(vp, vp, MMU_ACCESS_X | MMU_ACCESS_R, key);
		vp++;
	}
	
	//
	// read/write data
	//
	vp = ADDR_TO_PAGE(processInfo.readWriteAddr);
	count = SIZE_TO_PAGES(processInfo.readWriteSize);
	while(count--)
	{
		mmu_setPage(vp, vp, MMU_ACCESS_R | MMU_ACCESS_W, key);
		vp++;
	}
	
	//
	// shared read/write data
	//
	vp = ADDR_TO_PAGE(processInfo.sharedReadWriteAddr);
	count = SIZE_TO_PAGES(processInfo.sharedReadWriteSize);
	while(count--)
	{
		mmu_setPage(vp, vp, MMU_ACCESS_R | MMU_ACCESS_W, key);
		vp++;
	}
	
	cpu_setKey(1);
	cpu_setMMUTableAddress(mmuTable, MMU_TABLE_NUMPAGES);
	mmuEnabled = true;
}

extern u32 intrBus;
extern u32 intrCount;
extern u32 intrReason;

void interruptHandler(u32 data0, u32 data1, u32 data2, u32 data3)
{
	cpu_setKey(0);
#if 0
	scr_printf("bus=%u, reason=%u, data=%u,%u,%u,%u\n",
		intrBus, intrReason, data0, data1, data2, data3);
#endif
	if (intrBus==0)
	{
		exceptionCounter[intrReason]++;
		intrData[0] = data0;
		intrData[1] = data1;
		intrData[2] = data2;
		intrData[3] = data3;
		
		// For this test, if it was an "execute" abort, the test used the BL
		// instruction, and the LR register has a valid resume pointer.
		// We therefore set PC to that valid value before trying to resume
		if (data1==Execute)
		{
			interruptedCtx.gregs[CPU_REG_PC] = interruptedCtx.gregs[CPU_REG_LR];
		}
	}
	intrCount++;	
}

//
// Run some tests with MMU disabled.
// With the MMU disabled, there is no virtual memory or memory protection, but
// abort exceptions still happen if the ram falls out of RAM range
void testSimple(void)
{
	return;
	//
	// At the last address. These should cause aborts
	//
	u8* addr = (u8*)ramAmount;
	scr_printf("Testing Abort at end: X(%s), W(%s,%s,%s), R(%s,%s,%s)\n",
		checkFail(execute(addr), addr, Execute),
		checkFail(writeB(addr, 1), addr, Write),
		checkFail(writeH(addr-1, 1), addr-1, Write),
		checkFail(writeW(addr-3, 1), addr-3, Write),
		checkFail(tryReadB(addr), addr, Read),
		checkFail(tryReadH(addr-1), addr-1, Read),
		checkFail(tryReadW(addr-3), addr-3, Read)
	);
	
	
	//
	// At the very end of the virtual address range
	//
	addr = (u8*)0xFFFFFFFF;
	void* execAddr = (u8*)((u32)addr & (~3));
	scr_printf("Testing Abort at top: X(%s), W(%s,%s,%s), R(%s,%s,%s)\n",
		// NOTE : BL instructions mask the last two bits of a jump address
		// so account for that, so the check doesn't fail
		checkFail(execute(execAddr), execAddr, Execute),
		checkFail(writeB(addr, 1), addr, Write),
		checkFail(writeH(addr, 1), addr, Write),
		checkFail(writeW(addr, 1), addr, Write),
		checkFail(tryReadB(addr), addr, Read),
		checkFail(tryReadH(addr), addr, Read),
		checkFail(tryReadW(addr), addr, Read)
	);
}


#define FIRSTDATA_SIZE 1030
extern u8 firstData[FIRSTDATA_SIZE];
extern u8 firstDataEnd;

u32 dataOffset(void* ptr)
{
	return (u32)((u8*)ptr - processInfo.readWriteAddr);
}

void testWithMMU(void)
{
	// Since at this moment the MMU still points 1 to 1, this needs to have
	// the same behaviour
	testSimple();
	
	//
	// Try a memset crossing pages
	//
	// First we set to something that varies from index to index, so we can
	// later check if the memset worked on the entire range
	scr_printf("Testing memset across page boundaries...");
	bool ok = true;
	for(u32 i = 0; i<FIRSTDATA_SIZE; i++)
		firstData[i] = (u8)i;
	for(u32 i = 0; i<FIRSTDATA_SIZE; i++)
	{
		if (firstData[i]!=(u8)i)
		{
			scr_printf("**FAILED at idx %u (wanted %u, found %u)\n", i, (u8)i, firstData[i]);
			ok = false;
			break;
		}
	}
	if (ok)
		scr_printf("OK\n");
		
	memset(firstData, 0, sizeof(firstData));
	for(u32 i = 0; i<FIRSTDATA_SIZE; i++)
	{
		if (firstData[i]!=0)
		{
			scr_printf("FAILED at idx %d\n", i);
			ok = false;
			break;
		}
	}
	
	if (ok)
		scr_printf("OK\n");
	
	u32 firstDataOffset = dataOffset(&firstData);
	always_assert(firstDataOffset==0);
	always_assert((dataOffset(&firstDataEnd) - firstDataOffset)==FIRSTDATA_SIZE);
	
	// Add a virtual page at the end, where we can write, and mapped to
	// the .data pages
	mmu_setPage(ADDR_TO_PAGE(ramAmount), ADDR_TO_PAGE(processInfo.readWriteAddr),
		MMU_ACCESS_W, 1);
	u8* addr = (u8*)ramAmount;
	
	scr_printf("VPage mapped to .data\n");
	// Test writing to a vpage that is write-only, and read that that data back
	// from a physical page.
	scr_printf("Write-only vpage: +0W(%s,%s,%s), +1W(%s,%s,%s) +0R(%s,%s,%s)\n",
		checkOk(writeAndReadB(addr, &firstData, 0x34)),
		checkOk(writeAndReadH(addr, &firstData, 0x4567)),
		checkOk(writeAndReadW(addr, &firstData, 0x12345678)),
		checkOk(writeAndReadB(addr+1, (u8*)&firstData+1, 0x34)),
		checkOk(writeAndReadH(addr+1, (u8*)&firstData+1, 0x4567)),
		checkOk(writeAndReadW(addr+1, (u8*)&firstData+1, 0x12345678)),
		checkFail(tryReadB(addr), addr, Read),
		checkFail(tryReadH(addr), addr, Read),
		checkFail(tryReadW(addr), addr, Read)
	);
	memset(firstData, 0, sizeof(firstData));
	
	// set the page to write/read
	mmu_setPage(ADDR_TO_PAGE(ramAmount), ADDR_TO_PAGE(processInfo.readWriteAddr),
		MMU_ACCESS_R | MMU_ACCESS_W, 1);
		
	scr_printf("Read/Write: WR(%s,%s,%s), R(%s,%s,%s)\n",
		checkOk(writeAndReadB(addr+6, NULL, 0x34)),
		checkOk(writeAndReadH(addr+4, NULL, 0x4567)),
		checkOk(writeAndReadW(addr, NULL, 0x12345678)),
		checkOk(readB(&firstData[6], 0x34)),
		checkOk(readH(&firstData[4], 0x4567)),
		checkOk(readW(&firstData, 0x12345678))
	);
	memset(firstData, 0, sizeof(firstData));
	
	u32 offset = MMU_PAGE_SIZE-1;
	scr_printf("Accross boundaries with fails: W(%s,%s,%s), R(%s,%s,%s)\n",
		checkOk(writeAndReadB(addr+offset, (u8*)&firstData[offset], 0x34)), // Success
		checkFail(writeH(addr+offset, 0x4567), addr+offset, Write), // Fail
		checkFail(writeW(addr+offset, 0x12345678), addr+offset, Write), // Fail
		checkOk(readB(addr+offset, 0x34)), // Success
		checkFail(tryReadH(addr+offset), addr+offset, Read), // Fail
		checkFail(tryReadW(addr+offset), addr+offset, Read) // Fail
	);
	
	// Add another vpage pointing to to after the previous one, so we can test
	// writing/reading successfully across page boundaries
	mmu_setPage(ADDR_TO_PAGE(ramAmount)+1, ADDR_TO_PAGE(processInfo.readWriteAddr)+1,
		MMU_ACCESS_R | MMU_ACCESS_W, 1);
		
	memset(firstData, 0, sizeof(firstData));
	scr_printf("Accross boundaries success: WR(%s,%s,%s), WR(%s,%s,%s)\n",
		checkOk(writeAndReadB(addr+offset, NULL, 0x34)),
		checkOk(writeAndReadH(addr+offset, NULL, 0x4567)),
		checkOk(writeAndReadW(addr+offset, NULL, 0x12345678)),
		checkOk(writeAndReadB(addr+offset, &firstData[offset], 0x34)),
		checkOk(writeAndReadH(addr+offset, &firstData[offset], 0x4567)),
		checkOk(writeAndReadW(addr+offset, &firstData[offset], 0x12345678))
	);
	
}

int main(void)
{
	ramAmount = cpu_getRamAmount();
	scr_init();
	
	u32 codePages = SIZE_TO_PAGES(processInfo.readOnlySize);
	u32 dataPages = SIZE_TO_PAGES(processInfo.readWriteSize);
	u32 sharedPages = SIZE_TO_PAGES(processInfo.sharedReadWriteSize);
	u32 totalPages = codePages + dataPages + sharedPages;
	
	scr_printf(
		"Total RAM = %u bytes (%u kbytes, %u pages)\n"\
		"ROM image info:\n"\
		"readOnly        = address=%u, size=%u (%u pages)\n"\
		"readWrite       = address=%u, size=%u (%u pages)\n"\
		"sharedReadWrite = address=%u, size=%u (%u pages)\n"\
		"TOTAL = %u bytes (%u pages)\n",
		ramAmount, ramAmount/1024,  SIZE_TO_PAGES(ramAmount),
		processInfo.readOnlyAddr, processInfo.readOnlySize, codePages,
		processInfo.readWriteAddr, processInfo.readWriteSize, dataPages,
		processInfo.sharedReadWriteAddr, processInfo.sharedReadWriteSize, sharedPages,
		totalPages * MMU_PAGE_SIZE, totalPages
	);
	
	scr_printf("\nTesting with MMU disabled...\n");
	testSimple();
	doPause();
	
	scr_printf("Initializing MMU...");
	initMMUTable();
	scr_printf("OK\n");
	scr_printf("Testing with MMU enabled but 1 to 1...\n");
	testWithMMU();
	
	doPause();
	scr_clear();
	return 0;
}
