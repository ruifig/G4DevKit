
.text._getString1
public _getString1
_getString1:
	lea r0, [_string1]
	mov pc, lr
	
.text._getString2
public _getString2
_getString2:
	b _getString1
	
.text._getString3
public _getString3
_getString3:
	mov r0, 10
	mov r1, 11
	mov pc, lr 
	
.text
_dummy:
	mov pc, lr

.rodata
	_getString3Addr:
	.word _getString3
	
.data
	_string1:
	.string "_getString1"
	.word _dummy

