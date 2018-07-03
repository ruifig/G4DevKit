
;
; External symbols (in C code) 
;
extern _intrBus
extern _intrReason
extern _main
extern _interruptHandler

.text
.word _boot
.word _intrHandlerASM

; Where interrupted contexts are saved
public _interruptedCtx
_interruptedCtx:
.zero 208

_intrHandlerStack:
.zero 1024

_boot:
	; setup interrupt handler stack
	lea r0, [_intrHandlerStack + 1024]
	str [_interruptedCtx + 13*4], r0 ; setup SP register
	
	bl _main
	_bootLoop:
		hlt
		b _bootLoop
	
_intrHandlerASM:
	; Save the bus and reason that caused the interrupt
	; NOTE: r0..r3 should not be changed, since they are parameters for the C
	; interrupt handler function
	srl r5, ip, 24
	str [_intrBus], r5;
	and r5, ip, 0x80FFFFFF
	str [_intrReason], r5;
	
	bl _interruptHandler
	
	; Switch back to interrupted context
	lea r0, [_interruptedCtx]
	ctxswitch [r0], [r0]

.data

public _firstData
public _firstDataEnd
_firstData:
.zero 1030
_firstDataEnd:
