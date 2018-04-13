;*******************************************************************************
; 				Boot code
;*******************************************************************************

;
; declare functions from C files
extern _krn_preboot
extern _krn_getIntrCtx
extern _krn_init
extern _krn_handleInterrupt

;
; declare variables from C files
extern _krn_currIntrBusAndReason
extern _krn_prevIntrBusAndReason

;*******************************************************************************
;							BOOT
; On boot we setup just the minimum we need with assembly so we can call C code.
; The minimum required for this is setting up a stack frame
;*******************************************************************************
;
.text
_boot:
	str [_krn_currIntrBusAndReason], 0
	str [_krn_prevIntrBusAndReason], 0
	
	; Boot first pass to initialize basics
	; This is required, so we setup a stack where we want, exit the preboot
	; so we can abandon the temporary stack, then call the fullboot function
	; with a proper stack
	bl _krn_preboot
	mov sp, r0 ; _kernel_preboot returns the stacktop to use, so set the stack
		
	bl _krn_getIntrCtx;
	mov r4, r0; We keep intrCtx around in r4
	
	;
	; Fully initialize the rest of the system.
	; krn_init returns the context we should switch to
	;
	bl _krn_init
	str [_krn_currIntrBusAndReason], -1
	ctxswitch [r0], [r4] ; switch to ctx at [r0]
	
	_boot1:
		ldr r5, [_krn_currIntrBusAndReason]
		str [_krn_prevIntrBusAndReason], r5
		str [_krn_currIntrBusAndReason], ip
		bl _krn_handleInterrupt ; Returns the context to switch to
		str [_krn_currIntrBusAndReason], -1
		ctxswitch [r0], [r4] ; switch to ctx at [r0]
		b _boot1
	
;*******************************************************************************
;*******************************************************************************
;*******************************************************************************

;
; Read only data (after we set MMU)
;
.rodata

;
; Process runtime information
; This is patched by the linker when building a ROM file, 
; and information about the size of the program:
; Contents are:
; 	4 bytes - readOnlyAddress (where code and .rodata starts)
;	4 bytes - readOnlySize (size of the read only portion)
;	4 bytes - readWriteAddress ( where read/write data starts)
;	4 bytes - readWriteSize (size of the read/write data)
;	4 bytes - sharedReadWriteAddress ( where shared read/write data starts)
;	4 bytes - sharedReadWriteSize (size of the shared read/write data)
public _processInfo
_processInfo:
.zero 24

.section ".apcpudebug"
