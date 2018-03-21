;*******************************************************************************
; Bare minimum to print a null terminted string to the screen
;*******************************************************************************

.text
public _startup
_startup:

	; Get the address of the screen device buffer
	; Screen device is always on device bus 2
	; Device function 0 gets the screen buffer address
	mov ip, (0x2<<24) | 0 ;
	hwf
	
	; The screen buffer address is returned in r0
	mov r9, r0 ; keep the screen address in r9

	;
	; Prints a "Hello World" null terminated string 
	; at the top left of the screen
	;

	; Pointer to the screen position to write to
	lea r1, [_hello] ; Pointer to string to print
	ldrub r2, [r1] ; Get the first character

	printCharacter:
		or r3, r2, 0x0F00 ; Add colour information
		; print character as half word (1 byte for colour, 1 for the character)
		strh [r9], r3
		add r9, r9, 2 ; move screen pointer to the next position
		add r1, r1, 1 ; advance to the net character
		ldrub r2, [r1] ; read character
		cmp r2, 0 ; check for end of string
		bne printCharacter
	
	infiniteLoop:
		b infiniteLoop

.data
	_hello:
	.string "Hello World!"

