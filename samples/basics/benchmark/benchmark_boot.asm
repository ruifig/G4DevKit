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
_boot:
bl _appMain
_bootLoop:
hlt
b _bootLoop
