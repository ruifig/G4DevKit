#include "hwcommon.h"
#include "hwcpu.h"
#include "hwclock.h"
#include "hwkeyboard.h"
#include <string.h>
#include <hwscreen.h>
#include "common.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "hardwaretest_common.h"
#include "hardwaretest_cpu.h"
#include "hardwaretest_clock.h"
#include "hardwaretest_screen.h"
#include "hardwaretest_keyboard.h"
#include "hardwaretest_nic.h"

// Main context
static Ctx appCtx;
// When an interrupt happens, it switches to this context
Ctx intrCtx;

// The assembly interrupt handler sets this whenever an IRQ interrupt happens
u32 intrBus;
u32 intrReason;

#define NUM_DRIVERS 5

// Put all the the drivers together
DeviceTest deviceTests[NUM_DRIVERS];

Ctx* handleInterrupt(u32 data0, u32 data1, u32 data2, u32 data3)
{
	always_assert(intrBus<NUM_DRIVERS);
	Driver* driver = deviceTests[intrBus].driver;
	always_assert(intrReason < driver->numHanders);	
	// We just forward the handling to the specific driver
	driver->handlers[intrReason](data0, data1, data2, data3);	
	return &appCtx;
}

void hardwareTests(void);

/*
Initializes the application execution context
*/
Ctx* handleReset(void)
{
	cpu_setInterruptContexts(&appCtx, &intrCtx);
	
	// We use this small memory block as a stack for the application context
	#define APPSTACKSIZE 1024*10
	static char appStack[APPSTACKSIZE];
	memset(&appCtx, 0, sizeof(appCtx));
	appCtx.gregs[CPU_REG_SP] = (u32) &appStack[APPSTACKSIZE];
	appCtx.gregs[CPU_REG_PC] = (u32) &hardwareTests;
	appCtx.flags[0] = CPU_FLAGSREG_SUPERVISOR;
	
	// We return this to the assembly code, which specifies what execution
	// context to switch to
	return &appCtx;
}

void hardwareTests(void)
{
	scr_init();

	// Initialize drivers 
	hardwareTest_cpu_init(&deviceTests[HWBUS_CPU]);
	hardwareTest_clock_init(&deviceTests[HWBUS_CLK]);
	hardwareTest_screen_init(&deviceTests[HWBUS_SCR]);
	hardwareTest_keyboard_init(&deviceTests[HWBUS_KYB]);
	hardwareTest_nic_init(&deviceTests[HWBUS_NIC]);
	
	for(int i=0; i<NUM_DRIVERS; i++) {
		deviceTests[i].testFunc();
		doPause();
	}
	//deviceTests[HWBUS_NIC].testFunc();
	doPause();
	
	// We should never return from this function
	loopForever();
}

