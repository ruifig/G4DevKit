#if 0
int g2;

int foo(int a, int b)
{
	//static int fooVar = 4;
	return a - b; // + g2 + g1 + fooVar;
}

void getOther(int* other)
{
	*other = 1;
}

int unused(int a, int b)
{
	return a * b;
}

#endif