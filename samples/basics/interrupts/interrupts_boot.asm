;*******************************************************************************
;
; This sample shows how to install an handler for every type of interrupt.
; What you do when an interrupt occurs is up to you.
;
; NOTE: Because when some types of interrupts cause the debugger to go into
; break mode, you should detach the debugger after launching the sample
;
;*******************************************************************************

; Functions in the C file, which have the real interrupt handlers
extern _handleReset
extern _handleInterrupt
extern _cpu_setInterruptContexts;

; Variables in the C file
extern _intrBus
extern _intrReason
public _interruptedCtx;

;
; Execution starts here when booting
; We pass execution to a C function, since it's easier to work with C
.text
.word _boot
.word _interruptHandler
_interruptedCtx:
.zero 208

_boot:
	bl _handleReset
	; Start the application by resuming the context that was setup in
	; _interruptedCtx
	lea r4, [_interruptedCtx]
	ctxswitch [r4], [r4]
	dbgbrk ; We should never get here
	
	_boot1:
	; Save the bus and reason that caused the interrupt
	; NOTE: r0..r3 should not be changed, since they are parameters for the C
	; interrupt handler function
	srl r5, ip, 24
	str [_intrBus], r5;
	and r5, ip, 0x80FFFFFF
	str [_intrReason], r5;
	
	bl _handleInterrupt
	ctxswitch [r0], [r4]
	b _boot1
	
_interruptHandler:
	; Save the bus and reason that caused the interrupt
	; NOTE: r0..r3 should not be changed, since they are parameters for the C
	; interrupt handler function
	srl r5, ip, 24
	str [_intrBus], r5;
	and r5, ip, 0x80FFFFFF
	str [_intrReason], r5;
	
	bl _handleInterrupt
	
	; Resume application context
	lea r4, [_interruptedCtx]
	ctxswitch [r4], [r4]
	dbgbrk ; We should never get here

;*******************************************************************************
; Utility functions called from the C file, to cause some interrupts for testing
;*******************************************************************************	

; Utility function to try to execute at an invalid address,
; therefore cusing an Abort interrupt
public _causeAbortExecute
_causeAbortExecute:
	mov pc, 0x0FFFFFFA
	
; Utility function to cause a "Undefined Instruction" interrupt
public _causeUndefinedInstruction
_causeUndefinedInstruction:
	.word 0xFFFFFFFF

; Utility function to cause a "Illegal Instruction" interrupt
; This is done by setting the application context to run in User Mode, then
; call a privileged instruction
public _causeIllegalInstruction
_causeIllegalInstruction:
	mov r0,0
	msr r0 ; Set the flags register 0, which disable the Supervisor Mode bit
	; This should now cause a "Illegal instruction" interrupt, as 'hwi' is a
	; privileged instruction
	hwf

public _causeSystemCall
_causeSystemCall:
	; For a system call, you would here set the registers with the parameters
	; you want to pass to the kernel
	mov r0, 0xF00D
	mov r1, 0xBEEF
	mov r2, 0
	mov r3, 0
	swi
	; NOTE: The SWI interrupt handler will set our r0 to the result
	mov pc, lr

public _causeIRQ
_causeIRQ:
	push {lr}
	; Clock is fixed at bus 1
	; Clock function 2 sets a timer
	; Read the Clock documentation to understand all the parameters	
	mov ip, (0x1 << 24) | 2;	
	mov r0, 0x40000007 ; Timer 7, IRQ mode, No auto reset
	mov r1, 100 ; Trigger the timer in 1 ms.
	hwf
	pop {pc}

;*******************************************************************************
;										DATA
;*******************************************************************************
.data
