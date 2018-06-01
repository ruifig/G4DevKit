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
; There are 2 contexts. The currently executing context, and another one saved
; in _ctx.
; _ctx keeps the sate of the currently suspended context.
; 
; Register use as follow:
;	r4 - Screen column where to display the character
;	r5 - Screen row where to display the character
;	r6 - Character to show
;
;*******************************************************************************

.text
.word _startup
.word _startup
.zero 208

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
	
	; Prepare for the second context initialization
	; The second context is initialized when the first context switches
	; execution
	str [_ctx + 0*4], 1 ; context number
	lea r0, [_runCtx]
	str [_ctx + 15*4], r0 ; entry point
	; set the second's context flags register to be in Supervisor mode,
	; otherwise it can't call hwf
	mrs r0 ; Get current flags register
	str [_ctx + 16*4], r0; Set the flags register
	
	; Run the first context
	mov r0, 0
	b _runCtx
	
; On entry, r0 has the context number
_runCtx:
	; Set a stack for this context
	; sp = &_stackTop - (ctxNumber * 1024)
	lea sp, [_stackTop]
	smul r1, r0, 1024
	sub sp, sp, r1
	
	mov r4, 0 ; set the screen column
	; set the screen row
	add r5, r0, 2
	; Set the character to show
	add r6, r0, 49
	
_runCtxLoop:
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
	lea r0, [_ctx]
	ctxswitch [r0], [r0]
	; loop to beginning when this context gets resumed
	b _runCtxLoop
	
;*******************************************************************************
;								Data
;*******************************************************************************
.data
	_ctx:
	.zero 208
	
	; Stacks for both contexts
	.zero 2048
	_stackTop:
	
	_sampleName:
	.string "Context Switching Sample: Shows two contexts sharing execution"
	
