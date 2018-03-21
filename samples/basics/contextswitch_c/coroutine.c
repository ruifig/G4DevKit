#include "coroutine.h"
#include "hwcpu.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct Coroutines
{
	// To save the main state
	Coroutine main;
	Coroutine* curr;
} Coroutines;
Coroutines gCo;

static int ctxswitch(Ctx* new, Ctx* curr)
INLINEASM("\t\
ctxswitch [r0], [r1]");

static int mrs(void)
INLINEASM("\t\
mrs r0");

//
// Internal functions
//
static void setState(Coroutine* co, u32 state)
{
	co->flags = (co->flags & ~CO_STATE_MASK) | state;
}

void finish(Coroutine* co, int yielded)
{
	setState(co, CO_STATE_FINISHED);
	co->result = yielded;
	
	// Remember the next in the chain 
	Coroutine* next = co->next;
	
	// Remove from chain
	co->next->previous = co->previous;
	co->previous->next = co->next;
	// nullify next/previous, to help catch any bugs
	co->next = NULL;
	co->previous = NULL;
	
	// If we it's the currently executing coroutine, then we need to move switch
	// execution to the next one
	if (co==gCo.curr)
		co_yield(next, co->result);	
}

// This is generating incorrect assembly.
// The "entry" parameter in r2, but not saved across functionc alls
static void runCoroutine(int yielded , int cookie, CoroutineEntry entry)
{
	Coroutine* co = gCo.curr;
	setState(co, CO_STATE_ACTIVE);
	finish(co, entry(yielded, cookie));
}

//
// USER API
//
void co_create(Coroutine* co, void* stack, int stackSize, CoroutineEntry entry, int cookie)
{
	// We we create the first coroutine, we initialize what we need to save
	// the state of the main program
	if (co_getState(&gCo.main)!=CO_STATE_ACTIVE)
	{
		setState(&gCo.main, CO_STATE_ACTIVE);
		gCo.main.flags |= CO_ISMAIN;
		gCo.main.next = &gCo.main;
		gCo.main.previous = &gCo.main;
		gCo.curr = &gCo.main;
	}
	
	memset(co, 0, sizeof(Coroutine));
	
	setState(co, CO_STATE_STARTING);
	// setup stack
	co->cpuState.gregs[CPU_REG_SP] = (u32)stack + stackSize;
	// setup where to start executing, and parameters to the execution function
	co->cpuState.gregs[CPU_REG_PC] = (u32)&runCoroutine;
	// We set flag register to match the creator
	co->cpuState.flags[0] = mrs() & 0x0FFFFFFF;
	// Setup parameters for runCoroutine
	// NOTE: We don't need to set r0, because that's set when running the
	// coroutine, as part of co_yield
	co->cpuState.gregs[1] = cookie;
	co->cpuState.gregs[2] = (u32)entry;
	co->cookie = cookie;
	
	//
	// Insert the new coroutine at the end of the chain
	//
	// Main "previous" points to the last coroutine
	Coroutine* last = gCo.main.previous;
	// setup next direction
	last->next = co;
	co->next = &gCo.main;
	// setup previous direction
	gCo.main.previous = co;
	co->previous = last;
}

void co_destroy(Coroutine* co, int yielded)
{
	// Nothing to do if already finished, and we can't touch anything
	// if its the main program
	if (co_getState(co)==CO_STATE_FINISHED || co->flags&CO_ISMAIN)
		return;
	finish(co, yielded);
}

Coroutine* co_this(void)
{
	return gCo.curr;
}

bool co_hasAlive(void)
{
	// If "main" points to itself, then it's the only one in the chain
	return gCo.main.next==&gCo.main ? false : true;
}

int co_yield(Coroutine* co, int yielded)
{ 
	// By design, if a coroutine is still in the chain, then it can't be
	// finished, so the FINISHED check is not needed in that case.
	// That check is only needed if using the coroutine passed by the caller
	if (co==NULL) {
		co = gCo.curr->next;
	} else if (co_getState(co)==CO_STATE_FINISHED) {
		return 0;
	}
			
	// Set r0 of the context to resume, so that it gets our yield value
	co->cpuState.gregs[0] = yielded;
	Coroutine* curr = gCo.curr;
	gCo.curr = co;
	// This saves our state in "curr", and resumes "co"
	// When some other context resumes us, we get a yield value from that
	// context
	return ctxswitch(&co->cpuState, &curr->cpuState);
}
