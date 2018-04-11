#ifndef _coroutine_h_
#define _coroutine_h_

#include "hwcommon.h"

#define CO_STATE_MASK 0x3

//! Coroutine was created, but waiting to be executed for the first time
#define CO_STATE_STARTING 0
//! Coroutine is active
#define CO_STATE_ACTIVE 1
//! Coroutine finished, either by returning from the entry function, or by
// an explict co_destroy or co_exit
#define CO_STATE_FINISHED 2

#define CO_ISMAIN 0x4
typedef struct Coroutine
{
	Ctx cpuState;
	unsigned int flags;
	int result;
	struct Coroutine* previous;
	struct Coroutine* next;
	int cookie;
} Coroutine;

//! Type for coroutine entry function
// \param yielded Value supplied in the "co_yield" call that switched execution
// to this coroutine.
// \param cookie Cookie supplied in co_create
typedef int (*CoroutineEntry)(int yielded, int cookie);

//! Creates a new coroutine
// \param co Coroutine to initialize
// \param stack Buffer to use for stack
// \param stack Size of the stack buffer
// \param entry Entry function.
// \param cookie "cookie" parameter for the entry function. This is also
// saved in the Coroutine's "cookie" field
//
// NOTE: Make sure the supplied stack is big enough for what the coroutine needs
// to do. There are no checks in place to avoid a stack overflow whatsoever.
void co_create(Coroutine* co, void* stack, int stackSize, CoroutineEntry entry,
	int cookie);

//! Destroys the specified coroutine.
// \param co
//    Coroutine to destroy.
// \param yielded
//    Value to finish the coroutine with.
//
// NOTE: If "co" is the currently executing coroutine, this function never
// returns, since the coroutine is destroyed and execution transfered to the
// next coroutine in the chain, with the yielded value specified.
void co_destroy(Coroutine* co, int yielded);

//! Switches execution to the specified coroutine
// \param co
//    Coroutine to switch to. If this is a coroutine which is no longer active,
//    or it's the main program
//    this function call returns immediately with a return value of 0
// \param yielded
//    Value to pass to the coroutine that will be resumed.
//    If the coroutine to execute is a new coroutine, this value will be passed
//    as the parameter to the entry function. If its a previously suspended
//    coroutine, it will be the return value of the function used to suspend it
//    (e.g: Return value of the co_yield call)
//
// \return
//    Value passed back from whatever coroutine return control back to the
//    caller. Note that this values might not necessarily come from the
//    coroutine that the called originally resumed, since that coroutine might
//    transfer execution to other coroutines on its own.
int co_yield(Coroutine* co, int yielded);

//! Gets the state of the specified coroutine
#define co_getState(co) ((co)->flags&CO_STATE_MASK)

//! Returns the currently executing coroutine, or NULL if no coroutines were
// created yet.
// \Note Note that the main program is also treated as a coroutine
Coroutine* co_this(void);

//! Returns true if there are any coroutines left alive (excluding the main 
// program), or false otherwise.
// A coroutine is considered alive if it's in any state other than
// CO_STATE_FINISHED
bool co_hasAlive(void);

#endif
