.text
;.text._getString
public _getString
_getString:
	lea r0, [_hello2]
	mov pc, lr

.data
	_hello2:
	.string "Hello World!!!!"
