;*******************************************************************************
;
;*******************************************************************************

;
; Things we need from the C file
extern _main

.text

	;
	; Setup interrupt handling
	;
	; Context to load...
	lea r0, [_interruptStack + 1024]
	str [_interruptCtx + 13*4], r0 ; set stack
	lea r0, [_interruptHandlerAux]
	str [_interruptCtx + 15*4], r0 ; set PC
	mov ip, (0x0 << 24) | 4
	lea r0, [_interruptCtx]
	hwf
	
	; Where to save the interrupted context
	mov ip, (0x0 << 24) | 3
	lea r0, [_interruptedCtx]
	hwf

	; Execute application
	bl _main
loop:
	b loop; // Endless loop

_interruptCtx:
.zero 208
_interruptStack:
.zero 1024

public _interruptedCtx
_interruptedCtx:
.zero 208

extern _interruptHandler;

_interruptHandlerAux:
	; r0..r3 : Interrupt data
	; ip : Bus and Reason
	sub sp, sp, 4
	lea r5, [_interruptCtx] ; Keep the address of the interrupt ctx
	_interruptHandlerAux_Loop:
		str [sp], ip
		bl _interruptHandler ; Call the handler, and it will return ctx to run
		ctxswitch [r0], [r5]
		b _interruptHandlerAux_Loop

