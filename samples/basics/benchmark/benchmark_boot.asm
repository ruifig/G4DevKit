;*******************************************************************************
;
;*******************************************************************************

.text
.word _boot
.word _boot
.zero 208

;
; Things we need from the C file
extern _appMain
extern _mmuTable
extern _calcMMUTableSize

_boot:
	bl _calcMMUTableSize
	; Make room for the mmu table, in the stack
	sub sp, sp, r0
	str [_mmuTable], sp
	bl _appMain


_bootLoop:
	hlt
	b _bootLoop
