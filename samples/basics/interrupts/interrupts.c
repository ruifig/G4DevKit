
#include "common.h"
#include <string.h>
#include "hwkeyboard.h"
#include "hwscreen.h"
#include "hwcpu.h"
#include "hwclock.h"

extern Ctx interruptedCtx;

// The assembly interrupt handler sets this whenever an IRQ interrupt happens
u32 intrBus;
u32 intrReason;

// Counts the total number of interrupts
int intrCount;

//
// Whenever there is a system call, we set this variable with what the system
// call returned. It shows how a system call can pass a result back to the
// application
int lastSystemCallResult;

void launchApplication(void);
void redrawScreen(int doClear);
void showMenu(void);


/*!
Setups the application execution context.
This is used at startup, and whenever an unrecoverable interrupt occurs (in this
sample) to reset the application.
*/
void setupAppCtx(void)
{
	// We use this small memory block as a stack for the application context
	#define APPSTACKSIZE 4096
	static char appStack[APPSTACKSIZE];

	// Setup the stack (register SP)
	// Note that the stack grows downwards, so we point SP to the top address of
	// the memory block we are using for the stack	
	interruptedCtx.gregs[CPU_REG_SP] = (int) &appStack[APPSTACKSIZE];
	// Setup the PC register
	interruptedCtx.gregs[CPU_REG_PC] = (int) &launchApplication;
	// Setup the flags register
	// The value specified (0x04000000), sets Supervisor mode, and enables
	// IRQs	
	interruptedCtx.crregs[0] = 0x04000000;	
}

void printInterruptDetails(
	const char* title,
	u32 data0, u32 data1, u32 data2, u32 data3)
{
	redrawScreen(TRUE);
	Ctx* ctx = &interruptedCtx;
	
	int x = 4;
	int y = 1;
	scr_printfAtXY(x,++y , "Interrupt type: %s", title);
	scr_printfAtXY(x+40,y, "Bus %d, Reason %d", intrBus, intrReason);
	scr_printfAtXY(x,++y , "Num interrupts: %d", intrCount);
	
	int yy = y;
	scr_printfAtXY(x,++y, "Interrupt r0: 0x%X", data0);
	scr_printfAtXY(x,++y, "Interrupt r1: 0x%X", data1);
	scr_printfAtXY(x,++y, "Interrupt r2: 0x%X", data2);
	scr_printfAtXY(x,++y, "Interrupt r3: 0x%X", data3);
	
	y = yy;
	x = 40;
	scr_printfAtXY(x,++y, "App ctx r0: 0x%X", ctx->gregs[0]);
	scr_printfAtXY(x,++y, "App ctx r1: 0x%X", ctx->gregs[1]);
	scr_printfAtXY(x,++y, "App ctx r2: 0x%X", ctx->gregs[2]);
	scr_printfAtXY(x,++y, "App ctx r3: 0x%X", ctx->gregs[3]);
	scr_printfAtXY(x,++y, "Last SWI call result: %d", lastSystemCallResult);
}

/*******************************************************************************
*		Interrupt handlers
*******************************************************************************/

// How many drivers we support in the sample
#define NUM_DRIVERS 2

void handleReset(void)
{
	scr_init();
	setupAppCtx();
}

void cpu_handleGeneric(u32 data0, u32 data1, u32 data2, u32 data3)
{
	static const char* reasons[HWCPU_INTERRUPT_MAX] =
	{
		"ABORT",
		"DIVIDE BY ZERO",
		"UNDEFINED INSTRUCTION",
		"ILLEGAL INSTRUCTION",
		"SWI"
	};

	printInterruptDetails(reasons[intrReason], data0, data1, data2, data3);
	
	if (intrReason!=HWCPU_INTERRUPT_SWI) {
		// Anything other than a SWI interrupt is an unrecoverable interrupt
		// in this sample, so lets reset the application.
		setupAppCtx();
	} else {
		// If it's a system call, pass a return value back to the application
		interruptedCtx.gregs[0] = ++lastSystemCallResult;	
	}
}

void clock_handleTimer(u32 data0, u32 data1, u32 data2, u32 data3)
{
	intrCount++;
	printInterruptDetails("IRQ", data0, data1, data2, data3);
}

//
// Setup the driver structs
//
InterruptHandler cpu_handlers[] =
{
	&cpu_handleGeneric, // Abort
	&cpu_handleGeneric, // Divide by zero
	&cpu_handleGeneric, // Undefined instruction
	&cpu_handleGeneric, // Illegal intruction
	&cpu_handleGeneric  // SWI (System call)
};
InterruptHandler clock_handlers[] =
{
	&clock_handleTimer
};
// Put all the the drivers together
Driver drivers[NUM_DRIVERS] =
{
	{ cpu_handlers, HWCPU_INTERRUPT_MAX },
	{ clock_handlers, HWCLOCK_INTERRUPT_MAX }
};

void handleInterrupt(u32 data0, u32 data1, u32 data2, u32 data3)
{
	intrCount++;	
	// We just forward the handling to the specific driver
	drivers[intrBus].handlers[intrReason](data0, data1, data2, data3);
}

void showMenu(void)
{
	int x = 4;
	int y = 12;
	scr_printfAtXY(x, y++, "1. Test ABORT (Execute)");
	scr_printfAtXY(x, y++, "2. Test ABORT (Write)");
	scr_printfAtXY(x, y++, "3. Test ABORT (Read)");
	scr_printfAtXY(x, y++, "4. Test DIVIDE BY ZERO");
	scr_printfAtXY(x, y++, "5. Test UNDEFINED INSTRUCTION");
	scr_printfAtXY(x, y++, "6. Test ILLEGAL INSTRUCTION");
	scr_printfAtXY(x, y++, "7. Test SWI (System Call) (will pass 0xf00d,0xbeef,0x0,0x0 to the handler)");
	scr_printfAtXY(x, y++, "8. Test IRQ (triggers a one-off timer)");
}

void redrawScreen(int doClear)
{
	if (doClear) {
		scr_clear();
	}
	scr_printfAtXY(0,0,
		"Interrupts example: Make sure you disconnect the debugger");
	showMenu();
}

/*
Causes an Abort due to an invalid execute permissions
This one is implemented in assembly, so I can easily change the PC register
to point to an invalid address
*/
void causeAbortExecute(void);
// Defined in the assembly file.
void causeUndefinedInstruction(void);
// Defined in the assembly file.
void causeIllegalInstruction(void);
// Defined in the assembly file.
int causeSystemCall(void);
// Defined in the assembly file.
void causeIRQ(void);


// Used just for forcing a divide by zero without the compiler detect it
static int zero = 0;
static int dummy;

/*******************************************************************************
*		Interrupt handlers
*******************************************************************************/
void appMain(void)
{
	redrawScreen(FALSE);

	while(1) {
		int key = kyb_getNextTyped();
		switch(key) {
			case '1':
				causeAbortExecute();
			break;
			case '2':
			{
				// Cause an abort by trying to write to an invalid address
				int* ptr = (int*)0x0FFFFFFB;
				*ptr = dummy;
			}
			break;
			case '3':
			{
				// Cause an abort by trying to read from an invalid address
				int* ptr = (int*)0x0FFFFFFC;
				dummy = *ptr;
			}
			break;
			case '4':
				dummy = 10/zero;
				break;
			case '5':
				causeUndefinedInstruction();
				break;
			case '6':
				causeIllegalInstruction();
				break;
			case '7':
				lastSystemCallResult = causeSystemCall();
				break;
			case '8':
				causeIRQ();
				break;
		}
	}
}

void launchApplication(void)
{
	appMain();
	
	// Normally, once the main function of an application finished, the OS
	// would do some cleanup. In this case, if the application ever finishes,
	// we are just blocking here
	loopForever();
}
