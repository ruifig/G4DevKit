.text

.word _startup; boot
.zero 128 ; Interrupt handlers
.zero 232 ; Interrupt context

_startup:
extern _main
	
	.word -1
	mov r0, -1
	msr crirqmsk, r0
	hlt
	
	mrs r0, flags
	and r0, r0, ~(1<<27)
	msr flags, r0
	
	mov ip, (3<<24)|3
	mov r0, 1
	hwf
	loop:
	bl _main
	hlt
	mov r0,0
	mov r0,1
	b loop
	


