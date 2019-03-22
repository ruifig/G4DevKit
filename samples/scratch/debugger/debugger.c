#include "misc.h"
#include "func.h"

int g1,g2,g3,g4;

struct Struct
{
	int a;
	const char* b;
};

void testTempQualifiers(const char* p1, char* const p2, const char** const p3)
{
}

void testFuncInHeader2(void)
{
}

void func3(void)
{
}

void func4(int a, int b)
{
}

void func2(void)
{
	g1 = 1;
	func3();
	g2 = 2;
	func3();
	g3 = 3;
	func3();
	g4 = 4;
	int local1 = 5;
	func3();
	{
		int local1 = 1;
		func4(g3*local1, g1*local1);
		local1 *= 2;
	}
	
	local1 = 2;
	func4(g3*local1, g1*local1);
}

void func1(void)
{
	g1 = 1;
	func2();
}

int add(int a, int b)
{
	return a + b;
}

enum TestEnum1
{
	E1Val1 = 1,
	E1Val2 = 2
};

typedef enum
{
	E2Val1 = 1,
	E2Val2 = 2
} TestEnum2;

int testFunctionPtr(float (*pf) (char*, int))
{
}

int testStruct(const struct Struct* a)
{
	return 0;
}

int testEnum(enum TestEnum1 a, TestEnum2 b)
{
	struct Struct s;
	s.a = 10;
	s.b = "Struct string";
	testStruct(&s);
	return 0;
}

int testPointers(
	void* voidptr,
	unsigned char* a, signed char* b,
	unsigned short* c, signed short* d,
	unsigned int* e, signed int* f)
{
	return testEnum(E1Val1, E2Val2);
}

int testFloats(float a, double b)
{
	unsigned char p1 = 255;
	signed char p2 = -128;
	unsigned short p3 = 65535;
	signed short p4 = -32768;
	unsigned int p5 = 0xFFFFFFFF;
	signed int p6 = -2147483648;
	
	return testPointers(0xDEADBEEF, &p1, "Hello World!", &p3, &p4, &p5, &p6);
	return 0;
}

int testIntegers(
	unsigned char a, signed char b,
	unsigned short c, signed short d,
	unsigned int e, signed int f)
{
	return testFloats(10.5f,11.5f);
}

int main(int argc, const char** argv)
{
	initMisc();
	testIntegers(1,2,3,4,5,6);
	g1 = add(1,2);
	g2 = add(3,3);
	
	printAtXy(1,10, "Hello there!");
	
	func1();
	
	return 0;
}

