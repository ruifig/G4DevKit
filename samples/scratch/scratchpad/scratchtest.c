

typedef struct
{
	int a;
	int b;
} Foo;

int gVar1;
int* gVar2 = &gVar1;
void testFoo(const Foo*);

int main(void)
{
	Foo f;
	f.a = 1;
	f.b = 2;
	testFoo(&f);
	testFoo(&f);
	return 0;
}
