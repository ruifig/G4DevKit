#include "hardwaretest_keyboard.h"
#include "hwkeyboard.h"
#include <assert.h>
#include <string.h>
#include <hwclock.h>

typedef struct KeyboardDriver
{
	Driver base;
	u32 irqCount;
} KeyboardDriver;

static KeyboardDriver keyboardDriver;


typedef struct ControlKeyTest
{
	int code;
	const char* name;
	bool hasTyped;
} ControlKeyTest;


/*!
* Waits until the user releases any pressed keys
*/
static void waitNoPress(void)
{
	// Wait until we release all keys
	while(kyb_isPressed(0)){
	}
	kyb_clearBuffer();
}

// Interrupt handler
void keyboard_handleIRQ(u32 data0, u32 data1, u32 data2, u32 data3)
{
	keyboardDriver.irqCount++;
}

InterruptHandler keyboardHandlers[HWKYB_INTERRUPT_MAX] =
{
	&keyboard_handleIRQ
};

static ControlKeyTest controlKeyTest[] =
{
	{KEY_BACKSPACE, "Backspace", true},
	{KEY_RETURN, "Return", true},
	{KEY_INSERT, "Insert", true},
	{KEY_DELETE, "Delete", true},
	{KEY_UP, "Up", true},
	{KEY_DOWN, "Down", true},
	{KEY_LEFT, "Left", true},
	{KEY_RIGHT, "Right", true},
	{KEY_SHIFT, "Shift", false},
	{KEY_CONTROL, "Control", false},
	{KEY_TAB, "Tab", true},
	{ 0, NULL}
};

static void hardwareTest_keyboard(void);
void hardwareTest_keyboard_init(DeviceTest* data)
{
	keyboardDriver.base.handlers = keyboardHandlers;
	keyboardDriver.base.numHanders = 
		sizeof(keyboardHandlers)/sizeof(InterruptHandler);
	data->driver = &keyboardDriver.base;
	data->testFunc = &hardwareTest_keyboard;
}

static bool readString(char* buf, int bufsize)
{
	bool hasCtrl=false;
	int done = 0;
	char* ptr = buf;
	*ptr = 0;
	int cursorCh=32;
	
	kyb_clearBuffer();
	scr_printf(" ");
	do
	{
		int code;
		KeyEvent type = kyb_getNext(&code, false);
		switch(type)
		{
		
			// Key Pressed
			case kKeyEvent_Press:
				if (code==KEY_CONTROL) hasCtrl=true;
			break;
			
			// Key Typed
			case kKeyEvent_Typed:
				if (code==KEY_RETURN) {
					done = 1;
				} else if (code==KEY_BACKSPACE) {
					if ((ptr-buf)>0) {
						scr_printf("\b");
						ptr--;
						*ptr = 0;
					}
				} if (code>=KEY_ASCII_FIRST && code<=KEY_ASCII_LAST) {
					if ( (ptr-buf) < (bufsize-1)) {
						scr_printf("\b%c ",code);
						*ptr++ = (char)code;
						*ptr = 0;
					}
				}
			break;
			
			// Key released
			case kKeyEvent_Release:
				if (code==KEY_CONTROL) {
					hasCtrl=false;
				} else if (code==KEY_BACKSPACE) {
					if (hasCtrl) {
						*buf = 0;
						done = 2;
					}
				}
				
			break;
		}

		scr_printf("\b%c", (done==2) ? ' ' : cursorCh);
		cursorCh = cursorCh==32 ? 22 : 32;
	} while(!done);

	waitNoPress();
	
	return done==1 ? true : false;
}

static void hardwareTest_keyboard(void)
{
#if 0
	scr_printf("Keyboard Tests\n");
	int code;

	// Test string reading
	char buf[32];
	const char* expectedStr = "Hello World!";
	scr_printf("	Type '%s' and press Enter:\n	>", expectedStr);
	bool res = readString(buf, sizeof(buf));
	check_nl(strcmp(buf, expectedStr)==0);

	check(keyboardDriver.irqCount==0);
	kyb_setIRQMode(true);
	scr_printf("	Enabled Keyboard IRQ Mode. Press any key to test. ");
	// I'm waiting for any events, which in turn means that an interrupt must
	// occur
	while(!kyb_getNext(&code,true)) {
		// do nothing
	}
	check_nl(keyboardDriver.irqCount!=0);
	waitNoPress();

	//
	// Test all non-ASCII keys
	//
	scr_printf("	Testing special keys. Press and release them ONCE quickly.\n");
	ControlKeyTest* test = &controlKeyTest[0];
	while(test->name) {
		scr_printf("	Press & release %s : ", test->name);
		bool ok = true;
		if (ok)
			ok = (kyb_getNext(&code, true)==kKeyEvent_Press) && code==test->code;
		if (ok && test->hasTyped )
			ok = (kyb_getNext(&code, true)==kKeyEvent_Typed) && code==test->code;
		if (ok)
			ok = (kyb_getNext(&code, true)==kKeyEvent_Release) && code==test->code;
		scr_printf(" %s\n", ok ? "OK" : "FAILED" );
		test++;
	}
	
#endif
}


