
public _debugBreak
_debugBreak:
	dbgbrk
	mov pc, lr
	
public _getScreenAddr
_getScreenAddr:
	; Get the address of the screen device buffer
	; Screen device is always on device bus 2
	; Device function 0 gets the screen buffer address
	mov ip, (0x2<<26) | 0 ;
	hwf
	mov pc, lr

