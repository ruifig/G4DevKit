
#if 0
int g1;
extern int g2;
int foo(int a, int b);
void getOther(int* other);
void unused(int* other);

int main(int argc, const char** argv)
{
	int other1;
	int other2;
	getOther(&other1);
	getOther(&other2);
	return foo(g1, g2);
}

#else

int func1(int a, int b)
{
	return a + b;
}

static
int func2(int a, int b)
{
	return a * b;
}

int main(int argc, const char** argv)
{
	return
		func1((int)argv[0], (int)argv[1]) +
		func2((int)argv[2], (int)argv[3]);
}

#endif