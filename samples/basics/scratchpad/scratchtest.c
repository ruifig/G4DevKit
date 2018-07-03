typedef signed char s8;
typedef unsigned char u8;
typedef signed int s32;
typedef unsigned int u32;

void fooFunc(void)
{
}

#if 1
s32 b;
s8 s8_s32(s32 a)
{
    return a;
}

s8 s8_u32(u32 a)
{
    return a;
}

u8 u8_s32(s32 a)
{
    return a;
}

u8 u8_u32(u32 a)
{
    return a;
}

s8 test(s8 a)
{
    return b==(int)a ? 10 : 20;
}

#else

s8 a;
u8 aa;
s32 s32_s8_()
{
    return a;
}
u32 u32_s8_()
{
    return a;
}

s32 s32_u8_()
{
    return aa;
}

u32 u32_u8_()
{
    return aa;
}

s32 s32_s8(s8 a)
{
    return a;
}

s32 s32_u8(u8 a)
{
    return a;
}

u32 u32_s8(s8 a)
{
    return a;
}

u32 u32_u8(u8 a)
{
    return a;
}

#endif
 
