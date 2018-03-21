	.text
	.align	1
	.global	_func1
_func1:
	push	r6-r6,lr
	mov	r6,r1
	bl	_foo0
	bl	_foo0
	bl	r6
	pop	r6-r6,pc
	.global	_foo0
