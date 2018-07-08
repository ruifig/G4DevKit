/*
 * Manages memory pages on an high level
 */
 
#include "multitasking/process.h"
#include "mmu.h"
#include "boot/boot.h"
#include "extern/tlsf/tlsf.h"
#include "kernel/kerneldebug.h"
#include "hw/hwscreen.h"
#include "hw/hwcpu.h"
#include "kernel/kernel.h"

#include <stdlib_shared.h>
#include "utilshared/misc.h"

typedef struct MMUData
{
	uint32_t* table;
	uint32_t numPages;
} MMUData;
MMUData mmu;

/*
MMU NOTES

Each entry in the MMU table is 32 bits (1 word), with the bits having the 
following meaning:
--------------------------------------------------------------------------------
MMU Table entry
--------------------------------------------------------------------------------
\3\3\2\2\2\2\2\2\ \2\2\2\2\1\1\1\1\ \1\1\1\1\1\1\ \ \ \ \ \ \ \ \ \ \ \ 
 \1\0\9\8\7\6\5\4\ \3\2\1\0\9\8\7\6\ \5\4\3\2\1\0\9\8\ \7\6\5\4\3\2\1\0\
  [    Physical page                            ] R W   X [   key     ] 
       |                                          | |   |      |
	   |                                          | |   |      !----- Access key
	   |                                          | |   !------------ (0 : No executing allowed. 1 : Execute allowed)
	   |                                          | !---------------- (0 : No writing allowed. 1 : Writing allowed)
	   |                                          !------------------ (0 : No reading allowed. 1 : Reading allowed)
	   !------------------------------------------------------------- Physical page address

For a memory access to be successful, the access permissions R/W/X need to match
the request access type, and the "key" value in the entry needs to match the
"key" value from the flags registers.
A value of 0 for the "key" in both the MMU table entry and the flags register
has a special meaning:
- 0 in the MMU Table entry will allow access with any key from the flags
register, provided the R/W/X permissions are still satisfied.
- 0 in the flags register key will completely override MMU permission checks.
This is useful for a kernel, to quickly enable/disable complete access to the 
entire memory.

The Flags registers has the following format:
--------------------------------------------------------------------------------
Flags registers
--------------------------------------------------------------------------------
\3\3\2\2\2\2\2\2\ \2\2\2\2\1\1\1\1\ \1\1\1\1\1\1\ \ \ \ \ \ \ \ \ \ \ \ 
 \1\0\9\8\7\6\5\4\ \3\2\1\0\9\8\7\6\ \5\4\3\2\1\0\9\8\ \7\6\5\4\3\2\1\0\
  N Z C V I S                                             [  MMU KEY  ]
  | | | | | |                                              |
  | | | | | |                                              !-- Key to use to access memory
  | | | | | |                                                  Special values: 0 = master key. Memory operations will ignore the [key] specified in the MMU table
  | | | | | |
  | | | | | !- Supervisor mode
  | | | | !--- IRQ disabled 
  | | | !----- Overflow
  | | !------- Carry
  | !--------- Zero
  !----------- Negative


Now, with the considerations above, and considering the kernel process key is
0x01, the MMU code sets the memory permission as follow:

     SECTIONS                             MMU TABLE KEY
---------------------------------- |--------------------------------------------
| ... SCREEN BUFFER ...          | | 0x000000FF (free read/write)
|                                | | 
---------------------------------- | -------------------------------------------
|                                | | 0x00FFFFFF (No permissions at all)
| ...   N PAGES  ...             | | Once given to a process, page permissions
|                                | | are set according to that process, like:
|                                | | 0x00kkkkFF (read/write for that process)
---------------------------------- |--------------------------------------------
| ... kernel heap pages          | | 0x000101FF (Kernel can read/write)
|                                | |
|   |           .              | | |
|   |           .              | | |
|   |           .              | | |
|   |           .              | | |
|   | Kernel dynamic memory    | | |
|   |--------------------------| | |
|   | MMU table                | | |
|   |--------------------------| | |
|   | Kernel (data/bss)_shared | | |
|   |--------------------------| | |
|   | Kernel stack             | | | -> Stack is right a the bottom so we can
|   |                          | | | detect stack overflow, since it will crash
|   !--------------------------! | | trying to write to the pages bellow
---------------------------------- | -------------------------------------------
| .data_shared/.bss_shared pages | | 0x0001FFFF (Kernel can read)
---------------------------------- |--------------------------------------------
| .data/.bss pages               | | 0x000101FF (kernel can read/write)
---------------------------------- |--------------------------------------------
| .text/.rodata pages            | | 0x0000FF00 (no write at all)
---------------------------------- ---------------------------------------------


*/

bool mmu_check_user(struct PCB *pcb, u32 access, void* addr, size_t size)
{
	uint32_t page1 = ADDR_TO_PAGE(addr);
	uint32_t page2 = ADDR_TO_PAGE((uint8_t*)addr+size-1);
	if (page1>=mmu.numPages || page2>=mmu.numPages)
		return FALSE;

	uint32_t processKey= pcb->mainthread->ctx->crregs[CPU_CRREG_FLAGS] & MMU_KEYMASK;
	
	// If current ctx has master key, no need for checks
	if (processKey==0)
		return TRUE;
		
	for(int vpage = page1; vpage<=page2; vpage++)
	{
		u32 entry = mmu.table[vpage];
		u32 pageKey = entry & MMU_KEYMASK;
		if (!
			((pageKey==0 || pageKey==processKey) &&
			(entry & access)==access))
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

void mmu_setPages(int firstPage, int numPages, uint8_t pid, u32 access)
{
	uint32_t key = access | pid;
	for(int i=firstPage; i<firstPage+numPages; i++) {
		mmu.table[i] = (mmu.table[i] & MMU_PAGEMASK) | key;
	}	
}

int mmu_findFreePages(int numPages)
{
	int firstPage=0;
	int found=0;
	for(int i=0; i<mmu.numPages; i++) {
		if ((mmu.table[i] & MMU_KEYMASK) == PID_INVALID) {
			if (found==0)
				firstPage = i;
			found++;
			if (found==numPages)
				return firstPage;
		} else {
			found = 0;
		}
	}
	
	return -1;
}

int mmu_findFreeAndSet(int numPages, uint8_t pid, u32 access)
{
	int firstPage = mmu_findFreePages(numPages);
	if (firstPage==-1)
		return -1;
	mmu_setPages(firstPage, numPages, pid, access);
	return firstPage;
}

void mmu_freePages(uint8_t pid)
{
	uint32_t key = 0 | PID_INVALID;
	for(int i=0; i<mmu.numPages; i++) {
		if ((mmu.table[i] & MMU_KEYMASK) == pid)
			mmu.table[i] = (mmu.table[i] & MMU_PAGEMASK) | key;
	}
}

void mmu_freePagesRange(int firstPage, int numPages)
{
	uint32_t key = 0 | PID_INVALID;
	for(int i=firstPage; i<firstPage+numPages; i++) {
		mmu.table[i] = (mmu.table[i] & MMU_PAGEMASK) | key;
	}
}

int mmu_countPages(uint8_t pid)
{
	int count=0;
	for(int i=0; i<mmu.numPages; i++) {
		if ((mmu.table[i]&MMU_KEYMASK)==pid)
			count++;
	}
	return count;
}


/*
Sets permissions for pages in the memory range [startAddr ... startAddr+size[
*/
void mmu_setPagesInMemRange(
	void* startAddr, uint32_t size, uint8_t pid, uint32_t access)
{
	uint32_t firstPage = ADDR_TO_PAGE(startAddr);
	uint32_t numPages = SIZE_TO_PAGES(size);
	
	uint32_t key = access | pid;
	for(int i=firstPage; i<firstPage+numPages; i++) {
		mmu.table[i] = (mmu.table[i] & MMU_PAGEMASK) | key;
	}
}

void mmu_init(size_t krnFirstPage, size_t krnNumPages, void* mmutableaddr)
{
	KERNEL_DEBUG("%s (krnFirstPage=%u, krnNumPages=%u, mmutableaddr=%x",
		__func__, krnFirstPage, krnNumPages, mmutableaddr);
	mmu.numPages = SIZE_TO_PAGES(ramAmount);
	mmu.table = (uint32_t*)mmutableaddr;
	
	assert( isAligned(processInfo.readOnlyAddr,MMU_PAGE_SIZE) );
	assert( isAligned(processInfo.readWriteAddr,MMU_PAGE_SIZE) );
	assert( isAligned(processInfo.sharedReadWriteAddr,MMU_PAGE_SIZE) );
	
	
	// First, map the vpages to ppages
	for(int i=0; i<mmu.numPages; i++) {
		mmu.table[i] = i * MMU_PAGE_SIZE;
	}
	
	/* By default, no access at all */
	mmu_setPagesInMemRange(0, ramAmount, PID_INVALID, 0);

	/* setup .text/.rodata */
	mmu_setPagesInMemRange(
		(void*)processInfo.readOnlyAddr, processInfo.readOnlySize,
		PID_ANY, MMU_R|MMU_X);

	/* setup .data/.bss */
	// Kernel can read/write
	mmu_setPagesInMemRange(
		(void*)processInfo.readWriteAddr, processInfo.readWriteSize,
		PID_KERNEL, MMU_R|MMU_W);
	
	/* setup .data_shared/.bss_shared */
	// Kernel can read. No write at all (so we can use this data to initialize
	// other processes shared data)
	mmu_setPagesInMemRange(
		(void*)processInfo.sharedReadWriteAddr, processInfo.sharedReadWriteSize,
		PID_KERNEL, MMU_R);
		
	/* set kernel heap pages (read/write) */
	mmu_setPages( krnFirstPage, krnNumPages, PID_KERNEL, MMU_R|MMU_W);
	
	/* Set first page as read/write, so we can access the interrupted context */
	mmu_setPages( 0, 1, PID_KERNEL, MMU_R|MMU_W);

	/* Reserve the last few pages for the screen buffer */
	uint32_t screenBufferSize = hw_scr_getBufferSize();	
	mmu_setPagesInMemRange((void*)(ramAmount-screenBufferSize),screenBufferSize,
		PID_ANY, MMU_R|MMU_W);

	// set the mmu table
	hw_cpu_setMMUTable((uint32_t)(mmu.table), mmu.numPages);

	// Switch back to our own context, to trigger the mmu to update
	// TODO : Is something like this needed now that ctx_switch has changed?
	//hw_cpu_ctxswitch( (CpuCtx*)(&intrCtxStart) );
}
