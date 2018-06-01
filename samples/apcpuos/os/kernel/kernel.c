/*******************************************************************************
 * Main Kernel code
 ******************************************************************************/

#include "kernel.h"
#include "kerneldebug.h"
#include "boot/boot.h"
#include "hw/hwscreen.h"
#include "hw/hwclock.h"
#include "hw/hwkeyboard.h"
#include "hw/hwdisk.h"
#include "kernel/mmu.h"
#include "multitasking/process.h"
#include "syscalls/syscalls.h"
#include "stdlib_shared.h"
#include "string_shared.h"
#include "appslist.h"
#include "handles.h"
#include "kernelTimedEvents.h"
#include "appsdk/kernel_shared/txtui_shared.h"

Kernel krn;
bool krn_countSwiTime;
uint32_t ramAmount;
int krn_currIntrBusAndReason;
int krn_prevIntrBusAndReason;

void krn_spin(unsigned int ms)
{
	unsigned int beg;
	double speedHz=0;
	//int tickspersecond;
	beg = hw_clk_getRunningTimeMs32();
	while(1) {
		unsigned v = hw_clk_getRunningTimeMs32();
		if (v<beg) // If we detect wrap around, just quit
			return;
		else if (v-beg>=ms)
			return;
	}
}

/*!
 * First boot step, to initialize a basic environment, so we can do the rest
 * This is called from the boot assembly code
 * \return stack top to use for the kernel
 */
void* krn_preboot(void)
{
	// Check if we left a full page before the code
	extern uint32_t boot;
	assert((uint32_t)&boot==1024);
	
	ramAmount = hw_cpu_getRamAmount();
	krn.kernelPcb = prc_initKernelPrc();
	
	// Set the stack to use if a kernel crash happens before we finish
	// initializing the kernel
	// This is necessary, since the CPU will load the context at intrCtx
	// to handle interrupts, so intrCtx[CPU_REG_SP] needs to point to a valid
	// stack
	intrCtx.gregs[CPU_REG_SP] = (uint32_t)krn.kernelPcb->mainthread->stackTop;
	
	return krn.kernelPcb->mainthread->stackTop;
}

//
// IDLE task
//
// The IDLE task just powers down the cpu until an interrupt happens
uint32_t krn_idleTask(uint32_t p1)
{
#if TEST_TASKBOOT_FAIL
	krn_forceCrash();
#endif

	while(TRUE) {
		hw_cpu_hlt();
	}
	
	return EXIT_SUCCESS;
}

#ifdef DEBUG
static void krn_logTcbReady(const char* title)
{
	int size = queue32_size(&krn.tcbReady);
	KERNEL_DEBUG("**** TCB READY QUEUE %d : %s ***", size, title);
	for(int i=0; i<size; i++) {
		TCB* tcb = (TCB*) queue32_getAtIndex(&krn.tcbReady, i);
		KERNEL_DEBUG("    %8s:%Xh:%d", tcb->pcb->info.name, tcb,
			tcb->handle);
	}
}
#endif

static bool krn_kybCallback(uint8_t eventtype, uint8_t key, int flags,
	void* cookie)
{
	int msgid = MSG_KEY_PRESSED + (eventtype - HW_KYB_EVENT_PRESSED);
	LINKEDLIST_FOREACH(PCB, krn.kernelPcb, {
		if (it->info.flags & APPFLAG_WANTSKEYS) {
			prc_postThreadMessage(it->mainthread, msgid, key, flags);
		}
	});		
				
	return FALSE;
}

bool krn_taskScheduler(void* userData);

/*
 * Starts the kernel.
 * \return The cpu context to switch to
 */
CpuCtx* krn_init()
{
	// Initialize kernel arrays and queues
	queue32_create(&krn.tcbReady, 32);
	krn_initTimedEvents();

	hw_initAll();
	// Small pause so we can look at the devices
	krn_spin(2000 * KERNEL_BOOT_PAUSE);

	{
		uint32_t timer=0;
		uint32_t ms=THREAD_TIME_SLICE;
		krn_bootLog("Initializing interrupt %d, %d ms...", timer, ms);
		hw_clk_startTimer(timer, ms, TRUE, TRUE);
		hw_clk_addCallback(timer, krn_taskScheduler, NULL);
		krn_bootLog("Done\n");
		
		timer=1;
		ms=1000;
		krn_bootLog("Initializing interrupt %d, %d ms...", timer, ms);
		hw_clk_startTimer(timer, ms, TRUE, TRUE);
		//hw_clk_addCallback(timer, krn_displayStats, NULL);
		krn_bootLog("Done\n");
	}

	// Initialize the handles system before creating any apps
	handles_init();

	// Launch all apps
	for(int i=0; i<os_getNumApps(); i++)
	{
		KernelAppInfo* info = os_getAppInfo(i);
		krn_bootLog(
			"Creating task '%s', stack %u, memory %u...",
			info->name, info->stacksize, info->memsize);
		PCB* pcb = prc_create( info->name, info->startFunc, info->privileged,
			info->stacksize, info->memsize);
		pcb->info.flags = info->flags;
		pcb->info.cookie = info->cookie;
		krn_bootLog("Done\n");
	}
	
	krn.statsMode = kKrnStats_Cpu;
	hw_kyb_addEventCallback(krn_kybCallback, NULL);
	
	krn.idlethread = prc_find("idle")->mainthread;
	krn.idlethread->state = TCB_STATE_KERNEL;
	// Remove the idle from any queues, as we'll be using it explicitly
	tcb_enqueue(krn.idlethread, NULL);
	
	krn_bootLog("Done\n");	
	krn_bootLog("Starting OS...\n");
	
	krn_spin(1000 * KERNEL_BOOT_PAUSE);
	
	// Clear screen
	txtui_setBackgroundColour(&rootCanvas, kTXTCLR_BLACK);
	txtui_setForegroundColour(&rootCanvas, kTXTCLR_WHITE);
	txtui_clear(&rootCanvas);	
	
#if TEST_KERNEL_INIT_FAIL
	krn_forceCrash();
#endif

	krn.currTcb = krn.idlethread;
	
	return krn.currTcb->ctx;
}

void krn_pickNextTcb(void)
{
	// Grab a thread to run, if any is available
	if (queue32_pop(&krn.tcbReady, (int*)(&krn.currTcb))) {
		// put it back at the end of the ready queue
		queue32_push(&krn.tcbReady, (int)krn.currTcb);
	} else {
		// No threads ready to run, so run the idle process
		krn.currTcb = krn.idlethread;
	}
	
	// setup TLS
	if (krn.currTcb->tlsVarPtr) {
		prc_giveAccessToKernel(krn.currTcb->pcb, true);
		*krn.currTcb->tlsVarPtr = krn.currTcb->tlsVarValue;
		prc_giveAccessToKernel(krn.currTcb->pcb, false);
	}	
}

bool krn_taskScheduler(void* userData)
{
	krn_checkTimedEvents();	
	krn_pickNextTcb();
	return TRUE;
}

static void krn_leaveTcb(TCB* tcb, bool isSwiTime)
{
	static double lasttime=0;
	hw_clk_currSecs = hw_clk_getRunningTimeSeconds();
	
	double duration = hw_clk_currSecs-lasttime;
	
	// Note: krn.currTcb can be NULL if the current thread/process
	// was deleted
	if (isSwiTime && krn.currTcb)
		krn.currTcb->pcb->stats.cpuTimeswi += duration;
	else
		tcb->pcb->stats.cpuTime += duration;
		
	lasttime = hw_clk_currSecs;
}

CpuCtx* krn_handleInterrupt(
			uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3)
{
	// Check for double faults (kernel crashes)
	// This is detecting by checking if we were serving an interrupt before
	if (krn_prevIntrBusAndReason!=NO_INTERRUPT)
	{
		krn_panic(
			"DOUBLE FAULT: '%s' PREVIOUS %d, DATA 0x%X,0x%X,0x%X,0x%X",
			hw_cpu_getIntrReasonMsg(krn_currIntrBusAndReason & 0x80FFFFFF),
			krn_prevIntrBusAndReason, data0, data1, data2, data3);
		// note: panic never returns, so we never get here
	}

	krn_leaveTcb(krn.currTcb, false);	
	
	// Save the interrupted context back to the right TCB
	memcpy(krn.currTcb->ctx, &intrCtx, sizeof(CpuCtx)); 
	
	krn_countSwiTime = false;

	uint32_t busAndReason = krn_currIntrBusAndReason;
	do {
		uint8_t bus = busAndReason >> 24;
		uint32_t reason = busAndReason & 0x80FFFFFF;
		
		if (bus<HWBUS_DEFAULTDEVICES_MAX && hw_drivers[bus]) {
			hw_drivers[bus]->irqHandler(reason, data0, data1);
		} else {
			krn_panic(
				"BUS %d : Received IRQ for device without driver.",
				bus);
		}

		// If it's a cpu interrupt, we only serve 1, so we can correctly
		// calculate the the time spent in System Calls.
		if (bus==HWBUS_CPU)
		{
		break;
		} else {
			// Grab the next IRQ if any
			busAndReason = hw_cpu_nextIRQ(-1, &data0, &data1);
		}
		
	} while(busAndReason);	
	
	krn_leaveTcb(krn.kernelPcb->mainthread, krn_countSwiTime);
		
	return krn.currTcb->ctx;
}
