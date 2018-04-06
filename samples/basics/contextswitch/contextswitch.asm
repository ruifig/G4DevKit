;*******************************************************************************
; This sample shows how to do context switching
; It seems long, but most of the code is setting up things, which if done in
; C would be mostly assignment statements.
;
;              - Overview how the sample works -
;
; - On boot the cpu starts executing code at address 0
; - What _startup does
;		- Initialize the screen device
;		- Setup the contexts
;		- Loop, switching execution between the contexts
;
; There are 2 contexts, whose state is saved in _ctx1 and _ctx2.
; Also, a small stack is provided for each contexts (_ctx1Stack and _ctx2Stack)
;
;
; Register use as follow:
;	r4 - Screen column where to display the character
;	r5 - Screen row where to display the character
;	r6 - Character to show
;	r7 - Context to switch to
;	r8 - Where to save the current context
;
;*******************************************************************************

.text

;
; Things we need from the common static library
extern _scr_init
extern _scr_printCharAtXY
extern _scr_printStringAtXY
extern _pause

;*******************************************************************************
; Booting execution, as specified by the RESET interrupt handler
;*******************************************************************************
.text
public _startup
_startup:

	; Initialize screen device
	bl _scr_init
	
	; Print sample name
	mov r0, 0
	mov r1, 0
	lea r2, [_sampleName]
	bl _scr_printStringAtXY
	
	;
	; Setup the contexts
	; NOTE: _ctx1 is not being setup explicitly because it will be where the
	; initial context is saved to.
	;
	; Set the screen columns the contexts will write to (kept in register r4)
	mov r4, 0
	str [_ctx2 + 4*4], 0
	; Set screen rows the contexts will write to (kept in the r5 register)
	mov r5, 1
	str [_ctx2+ 5*4], 2
	; Set character to show (kept in register r6)
	mov r6, 49
	str [_ctx2+ 6*4], 50
	
	; Set the addresses to save and load the contexts from
	; In r7, we keep the address of the context to load.
	; In r8, we keep the address to save the current context to
	
	lea r7, [_ctx2] ; current context (_ctx1), switches to _ctx2
	lea r8, [_ctx1] ; current context state is saved in _ctx1
	str [_ctx2+ 7*4], r8 ; _ctx2 switches to _ctx1
	str [_ctx2+ 8*4], r7 ; _ctx2 save address
	
	; Setup a small stack for each context, so we can call C functions
	lea r0, [_ctx1Stack+1024]
	mov sp, r0 ; Stack for current context
	lea r0, [_ctx2Stack+1024]
	str [_ctx2+ 13*4], r0 ; Stack for _ctx2

	; Setup PC for _ctx2
	lea r0, [_contextLoop]
	str [_ctx2+ 15*4], r0
	
	; Set the contexts flags registers, to make sure we have interrupts
	; disabled and we are in supervisor mode
	; In this case, we are just copying the current flags to _ctx2
	mrs r0 ; Get our flags register
	str [_ctx2+ 16*4], r0

	; Now.... we let the main context run the intended code once
	; and then at the end it switches contexts
	
_contextLoop:

	; Clear the previous screen character
	mov r0, r4 ; x
	mov r1, r5 ; y
	mov r2, 32; a space, to clear
	bl _scr_printCharAtXY
	
	; Increment screen column, wrapping around at the end
	; r4 = (r4+1) % 80
	add r4, r4, 1
	sdiv r4:ip, r4, 80
	
	; Print the character at the desired screen coordinates
	mov r0, r4 ; x
	mov r1, r5 ; y
	mov r2, r6 ; character
	bl _scr_printCharAtXY

	; Pause for 500 ms
	mov r0, 500
	bl _pause
	
	; Change execution to the other context
	; Change to context at [r7], and save current in [r8]
	ctxswitch [r7], [r8]
	
	; loop to beginning when this context gets resumed
	b _contextLoop 

;*******************************************************************************
;								Data
;*******************************************************************************
.data
	_ctx1:
	.zero 208
	_ctx2:
	.zero 208

	; These are used as small stacks for the test contexts
	_ctx1Stack:
	.zero 1024
	_ctx2Stack:
	.zero 1024
	
	_sampleName:
	.string "Context Switching Sample: Shows two contexts sharing execution"
	
