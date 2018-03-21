
typedef int(*FuncPtr0)(void);
void foo0(void);



#if 1
void func1(int a, FuncPtr0 b)
{
	foo0();
	foo0();
	b();
}

#endif

#if 0
typedef int(*FuncPtr1)(int);
void func2(int a, FuncPtr1 b, FuncPtr0 c, FuncPtr0 d)
{
	foo0(); // << r0-r3 should be saved before calling this.
	b(a); // << b was in r1, but not saved, which generate the wrong assembly
	c();
	d();
}
#endif

#if 0
void foo1(int*);
void foo1_1(FuncPtr0);
void func3(int a, FuncPtr0 b)
{
	foo0();
	foo1_1(b);
}
#endif
