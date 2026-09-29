#ifndef LE32_H
#define LE32_H

#include <exec/types.h>
#include <SDI_compiler.h>

ULONG ASM LE32(REG(d0, ULONG a));
VOID  ASM Swap32(REG(a0, APTR buffer), REG(d0, ULONG size));

#endif
