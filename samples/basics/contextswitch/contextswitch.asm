;*******************************************************************************
; This sample shows how to do context switching
; It seems long, but most of the code is setting up things, which if done in
; C would be mostly assignment statements.
;
;              - Overview how the sample works -
;
; - On boot the cpu changes to context at fixed location 8 (_mainCtx here),
;   and sets the PC register of that context to the RESET handler,
;	named here as label _startup
; - What _startup does
;		- Get and save screen buffer address so we can use it later
;		- Setup the contexts _ctx1 and _ctx2, including a small stack for each
;		- Change executiong to _ctx1
;		- Execution keeps switching between _ctx1 and _ctx2
;
; - _ctx1/_ctx2 execution goes like this:
;		- Clear the last character
;		- Increment the screen column
;		- Print character at the new screen position
;		- Pause for a bit
;		- Change execution to the other context
;		- Once the other context passes execution back, loop
;
; Some state is passed to the _ctx1 and _ctx2 in the following registers:
;	r4 - Screen column where to display the character
;	r5 - Screen row where to display the character
;	r6 - Character to show
;	r7 - context to switch to after 1 iteration
;
;*******************************************************************************

.text

; The two contexts we will be running

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

	bl _scr_init
	
	; Print sample name
	mov r0, 0
	mov r1, 0
	lea r2, [_sampleName]
	bl _scr_printStringAtXY
	
	;
	; Setup the contexts
	;
	; Set the screen columns the contexts will write to (kept in register r4)
	mov r4, 0
	str [_otherCtx + 4*4], 0
	
	; Set screen rows the contexts will write to (kept in the r5 register)
	mov r5, 1
	str [_otherCtx + 5*4], 2

	; Set character to show (kept in register r6)
	mov r6, 49
	str [_otherCtx + 6*4], 50
	
	
	; Set the addresses to save and load the contexts from
	; In r7, we keep the address of the context to load.
	; In r8, we keep the address to save the current context to
	lea r0, [_mainCtx]
	lea r1, [_otherCtx]

	mov r7, r1 ; mainCtx will switch to otherCtx
	mov r8, r0 ; where to save mainCtx
	
	str [_otherCtx + 7*4], r0 ; otherCtx will switch to mainCtx
	str [_otherCtx + 8*4], r1 ; where to save otherCtx
	
	; Setup a small stack for each context, so we can call C functions
	lea r0, [_mainStack+1024]
	mov sp, r0
	lea r0, [_otherStack+1024]
	str [_otherCtx + 13*4], r0

	; Setup PC for the other context
	lea r0, [_contextLoop]
	str [_otherCtx + 15*4], r0
	
	; Set the contexts flags registers, to make sure we have interrupts
	; disabled and we are in supervisor mode
	mrs r0 ; Get our flags register
	str [_otherCtx + 16*4], r0

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
	_mainCtx:
	.zero 208
	_otherCtx:
	.zero 208

	; These are used as small stacks for the test contexts
	_mainStack:
	.zero 1024
	_otherStack:
	.zero 1024
	
	_sampleName:
	.string "Context Switching Sample: Shows two contexts sharing execution"
	
