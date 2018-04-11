.text
extern _main
	mrs r0
	and r0, r0, ~(1<<27)
	msr r0
	
	mov ip, (3<<24)|3
	mov r0, 1
	hwf
	loop:
	bl _main
	hlt
	mov r0,0
	mov r0,1
	b loop
	


