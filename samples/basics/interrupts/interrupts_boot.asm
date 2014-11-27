;*******************************************************************************
;
; This sample shows how to install an handler for every type of interrupt.
; What you do when an interrupt occurs is up to you.
;
; NOTE: Because when some types of interrupts cause the debugger to go into
; break mode, you should detach the debugger after launching the sample
;
; The code flow for this sample goes something like this:
;	- The Interrupt vector table specifies the handler for every interrupt type
;	- When an interrupt occurs, the handler passes control to the respective C
;	  function. It's easier to only have the bare minimum in assembly, and do
;	  most of the work in C
;	- At boot, a RESET interrupt occurs
;		- The handler passes control to the C function, which setups the
;		  application context.
;		- Execution is passed to the application context.
;	- The application waits for a key to be pressed, and they causes the 
;	  desired interrupt.
;	- If the interrupt that occurs is to be considered an error, like for
;	  example the application tried to read/write/execute an invalid address,
;	  some variables are updated, and the application is restarted, so we can
;	  keep running the sample.
;	- If the interrupt is an IRQ or a SWI (system call), the interrupt is
;	  handled, some variables udpated and then the application is resumed.
;*******************************************************************************

.text

; Interrupt vector table (32 bytes)
.word _interrupt_Reset
.word _interrupt_Abort
.word _interrupt_DivideByZero
.word _interrupt_UndefinedInstruction
.word _interrupt_IllegalInstruction
.word _interrupt_SystemCall
.word _interrupt_IRQ
.word _interrupt_RESERVED ; not used

;
; The default Execution context is fixed at address 32
; It's the context the cpu switches to a boot or for interrupts
;
; Note that that there is an extra word at the end which is a pointer to the
; context name. This is so that our interrupt context matches the Ctx struct
; defined in the C file
_interruptCtx:
.zero 196 ; registers (r0...pc), flags register, and floating point registers
.word _interruptCtxName

; Functions in the C file, which have the real interrupt handlers
extern _handleReset
extern _handleAbort
extern _handleDivideByZero
extern _handleUndefinedInstruction
extern _handleIllegalIntruction
extern _handleSystemCall
extern _handleIRQ
extern _interruptedCtx
extern _interruptReason
.text

;*******************************************************************************
; 		INTERRUPT HANDLERS
; These just pass control to a C function which does the real work
; It's easier to work with C than Assembly
;*******************************************************************************
public _interrupt_Reset
_interrupt_Reset:
	str [_interruptedCtx], lr ; save interrupted context
	str [_interruptReason], ip ; save interrupt reason
	bl _handleReset
	ctxswitch [r0] ; resume context the handler wants us to resume	
	; We should never get here...
	
_interrupt_Abort:
	str [_interruptedCtx], lr ; save interrupted context
	str [_interruptReason], ip ; save interrupt reason
	bl _handleAbort
	ctxswitch [r0] ; resume context the handler wants us to resume	
	
_interrupt_DivideByZero:
	str [_interruptedCtx], lr ; save interrupted context
	str [_interruptReason], ip ; save interrupt reason
	bl _handleDivideByZero
	ctxswitch [r0] ; resume context the handler wants us to resume	
	
_interrupt_UndefinedInstruction:
	str [_interruptedCtx], lr ; save interrupted context
	str [_interruptReason], ip ; save interrupt reason
	bl _handleUndefinedInstruction
	ctxswitch [r0] ; resume context the handler wants us to resume	
	
_interrupt_IllegalInstruction:
	str [_interruptedCtx], lr ; save interrupted context
	str [_interruptReason], ip ; save interrupt reason
	bl _handleIllegalIntruction
	ctxswitch [r0] ; resume context the handler wants us to resume	
	
_interrupt_SystemCall:
	str [_interruptedCtx], lr ; save interrupted context
	str [_interruptReason], ip ; save interrupt reason
	bl _handleSystemCall
	ctxswitch [r0] ; resume context the handler wants us to resume	
	
_interrupt_IRQ:
	str [_interruptedCtx], lr ; save interrupted context
	str [_interruptReason], ip ; save interrupt reason
	bl _handleIRQ
	ctxswitch [r0] ; resume context the handler wants us to resume	
	
_interrupt_RESERVED:
	; This is reserved for future use.
	b _interrupt_RESERVED
	
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
; call a previleged instruction
public _causeIllegalInstruction
_causeIllegalInstruction:
	mov r0,0
	msr r0 ; Set the flags register 0, which disable the Supervisor Mode bit
	; This should now cause a "Illegal instruction" interrupt, as 'hwi' is a
	; previleged instruction
	hwi

public _causeSystemCall
_causeSystemCall:
	; For a system call, you would here set the registers with the parameters
	; you want to pass to the kernel
	mov r0, 0xF00D
	mov r1, 0xBEEF
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
	hwi
	pop {pc}

;*******************************************************************************
;										DATA
;*******************************************************************************
.data
	_interruptCtxName:
	.string "Interrupt Context"
