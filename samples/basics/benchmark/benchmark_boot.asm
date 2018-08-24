;*******************************************************************************
;
;*******************************************************************************

.word _startup ; RESET handler

_ivt: ; Interrupt vector table
.zero 128 ; 4*32

_intrHCtx:
.zero 232

;
; Things we need from the C file
extern _appMain
extern _mmuTable
extern _calcMMUTableSize

_startup:
	bl _calcMMUTableSize
	; Make room for the mmu table, in the stack
	sub sp, sp, r0
	str [_mmuTable], sp
	bl _appMain

_bootLoop:
	hlt
	b _bootLoop
