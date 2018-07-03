#include "common.h"

typedef enum CpuException
{
	Cpu_Abort,
	Cpu_DivideByZero,
	Cpu_UndefinedInstruction,
	Cpu_IllegalInstruction,
	Cpu_SWI,
	Cpu_MAX
} CpuException;

typedef enum Access
{
	Execute = 1,
	Write = 2,
	Read = 4
} Access;

bool writeB(void* addr, u8 val);
bool writeH(void* addr, u16 val);
bool writeW(void* addr, u32 val);
bool readB(void* addr, u8 val);
bool readH(void* addr, u16 val);
bool readW(void* addr, u32 val);
bool tryReadB(void* addr);
bool tryReadH(void* addr);
bool tryReadW(void* addr);
bool execute(void* addr);

bool writeAndReadB(void* addr, void* readAddr, u8 val);
bool writeAndReadH(void* addr, void* readAddr, u16 val);
bool writeAndReadW(void* addr, void* readAddr, u32 val);

const char* checkOk(bool res);
const char* checkFail(bool res, void* addr, Access access);
