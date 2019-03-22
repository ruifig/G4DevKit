;*******************************************************************************
;
;*******************************************************************************

.text
.word _startup
.word _startup
.zero 232

;
; Things we need from the C file
extern _main

.text
_startup:
	; Execute application
	bl _main
loop:
	hlt
	b loop; // Endless loop
