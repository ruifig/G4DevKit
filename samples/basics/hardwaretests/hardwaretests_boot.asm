
; Functions in the C file
extern _handleReset
extern _handleInterrupt

; Variables in the C file
extern _intrCtx
extern _intrBus
extern _intrReason


.text
_boot:
	bl _handleReset ; _handleReset returns the context to load
	
	; switch to context at [r0], and save current at [_intrCtx]
	lea r4, [_intrCtx]
	ctxswitch [r0], [r4]
	
	_boot1:
	; Save the bus and reason that caused the interrupt
	; NOTE: r0..r3 should not be changed, since they are parameters for the C
	; interrupt handler function
	srl r5, ip, 24
	str [_intrBus], r5;
	and r5, ip, 0x80FFFFFF
	str [_intrReason], r5;
	
	bl _handleInterrupt ; _handleInterrupt returns the context to load
	ctxswitch [r0], [r4]
	b _boot1;
