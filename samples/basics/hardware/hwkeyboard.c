#include "hwkeyboard.h"
#include <assert.h>

#define HWKYBFUNC_CLEARBUFFER 0
#define HWKYBFUNC_GETNEXT 1
#define HWKYBFUNC_GETKEYSTATE 2

void kyb_clearBuffer(void)
{
	HwiData data;
	int res = hwiCall(HWBUS_KYB, HWKYBFUNC_CLEARBUFFER, &data);
	always_assert(res==HWIERR_SUCCESS);
}

KeyEvent kyb_getNext(int* keyCode, bool block)
{
	HwiData data;
	do {
		int res = hwiCall(HWBUS_KYB, HWKYBFUNC_GETNEXT, &data);
		always_assert(res==HWIERR_SUCCESS);
	} while(block && data.regs[0]==0);
	
	if (data.regs[0])
		*keyCode = data.regs[1];	
	return data.regs[0];
}

bool kyb_isPressed(int keyCode)
{
	HwiData data;
	data.regs[0] = keyCode;
	int res = hwiCall(HWBUS_KYB, HWKYBFUNC_GETKEYSTATE, &data);
	always_assert(res==HWIERR_SUCCESS);
	return data.regs[0] ? true : false;
}

int kyb_getNextTyped(void)
{
	int keyCode;
	while(kyb_getNext(&keyCode, true)!=kKeyEvent_Typed) {
	}
	return keyCode;
}

void kyb_pause(void)
{
	kyb_clearBuffer();
	kyb_getNextTyped();
	kyb_clearBuffer();
}

