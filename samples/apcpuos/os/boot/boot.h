/*******************************************************************************
* This declares symbols that are defined in boot.asm
*******************************************************************************/

#ifndef _APCPU_BOOT_H_
#define _APCPU_BOOT_H_

/*!
 * Ram amount, in bytes
 */
extern unsigned int ramAmount;

/*!
 */
typedef struct ROMProcessInfo
{
	unsigned int readOnlyAddr; // address
	unsigned int readOnlySize; // size in bytes
	unsigned int readWriteAddr; // address
	unsigned int readWriteSize; // size in bytes
	unsigned int sharedReadWriteAddr;
	unsigned int sharedReadWriteSize;
} ROMProcessInfo;
extern ROMProcessInfo processInfo;

#define NO_INTERRUPT -1

#endif



