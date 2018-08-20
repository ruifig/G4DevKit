.text
.word _startup ; RESET handler

_ivt: ; Interrupt vector table
.zero 128 ; 4*32

_intrHCtx:
.zero 232

extern _main
_startup:

	; Setup interrupt handler stack
	lea r0, [_intrHStackH]
	str [_intrHCtx+ 13*4], r0 ; set SP
	
	; Set interrupt context crtsk register
	lea r0, [_intrHCtx]
	str [_intrHCtx+ 16*4 + 2*4 + 16*8 + 2*4], r0
	
	; Setup interrupt handlers
	lea r0, [_intrH]
	str [_ivt + 4*3], r0 ; Keyboard
	
	; Set interrupt context crtsk register
	lea r0, [_appCtx]
	msr crtsk, r0
	
	bl _main
	loop:
	b loop
	
extern _interruptHandler
_intrH:
	mov r4, lr ; save the interrupted context
	mov r0, ip
	bl _interruptHandler

	lea r0, [_intrHCtx]
	fullctxswitch [r4], [r0]
	dbgbrk ; we should never get here
	
public _hwf;
_hwf:
	sll ip, r0, 26
	or ip, ip, r1
	mov r0, r2
	mov r1, r3
	hwf
	mov pc, lr
	
.data
	_intrHStackL:
	.zero 4096
	_intrHStackH:

	_appCtx:
	.zero 232