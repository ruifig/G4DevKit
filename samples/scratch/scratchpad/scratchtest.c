#include "scratchtest.h"
#include "misc.h"
#include "dummy.h"

typedef struct
{
	int a;
	int b;
} Foo;

int derp(int a)
{
	return fromDummy(a, a);
}

void someFunc()
{
}

int fooFunc()
{
	Foo f;
	derp(1);
	derp2();
	derp3(1,2,3);
	someFunc();
	return f.b;
}

