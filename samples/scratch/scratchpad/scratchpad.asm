.text
.word _startup
_ivt:
.zero 128 ; Interrupt vector table

_intrHCtx:
.zero 232

extern _main
_startup:

	; Setup stack
	lea sp, [_stackH]
	
	.L10:
	bl .L11
	.word .L10 - .L11
	
	b .L10
	bl _main
	infiniteLoop:
	hlt
	b infiniteLoop
	.L11:
	
.data
	_stackL:
	.zero 1024
	_stackH:

	_hello:
	.string "Hello World!"
