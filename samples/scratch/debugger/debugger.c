#include "misc.h"
#include "func.h"

struct Foo
{
	int a;
	int b;
};

void func1(struct Foo* foo)
{
	static int gVar3 = 1000;
	foo->a = foo->b;
	foo->b = gVar3;
}

int gVar1=10;
struct Foo gVar2;
float gVar3 = 0.15;

#define NULL 0x0

#if 0


int func1(void)
{
	return 1;
}

int func2(void)
{
	return 2;
}
#endif

int main(int argc, const char** argv)
{
	struct Foo foo;
	foo.a = 1;
	foo.b = 2;
	gVar1 = 20;
	gVar2.a=100;
	gVar2.b=200;
	func1(&foo);
	return 0;
}

#if 0
int g1,g2,g3,g4;

struct InnerStruct
{
	const char *aa;
	const char *bb;
};

struct Struct
{
	int a;
	const char* bz;
	struct InnerStruct inner1;
	struct InnerStruct* inner2;
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
	const int* ptr1 = 0x2; // constant object
	int * const ptr2 = 0x2; // constant pointer
	return 0;
}

struct InnerStruct inner2;

int testEnum(enum TestEnum1 a, TestEnum2 b)
{
	struct Struct s;
	
	int* array4[2] = {0x100, 0x200};
	int array1[1];
	int array2[10] = {0,1,2,3,4,5,6,7,8,9};
	struct Struct array3[2];

	array3[1].bz = "array3 aa string";
	array3[1].inner1.bb = "array3 inner 1 bb string";
	
	inner2.aa = "Inner2 aa";
	inner2.bb = "Inner2 bb";
	
	s.a = 10;
	s.bz = "Struct string";
	s.inner1.aa = "Inner aa";
	s.inner1.bb = "Inner bb";
	array4[0] = &s.a;
	s.inner2 = &inner2;
	testStruct(&s);
	return 0;
}

int testPointers(
	void* voidptr,
	unsigned char* a, signed char* b,
	unsigned short* c, signed short* d,
	unsigned int* e, signed int* f)
{

	struct Node nodes[5];
	for(int i=0; i<5; i++)
	{
		nodes[i].val = i;
		nodes[i].previous = i==0 ? NULL: &nodes[i-1];
		nodes[i].next= i==4 ? NULL: &nodes[i+1];
	}
	
	int dummy = 0;
	dummy = 1;
	dummy = 2;
	dummy = 3;
	dummy = 4;
	dummy = 5;
	dummy = 6;
	dummy = 7;
	
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

#endif
