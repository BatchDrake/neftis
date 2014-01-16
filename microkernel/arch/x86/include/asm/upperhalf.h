#ifndef _ARCH_X86_ASM_UPPER_HALF_H
#define _ARCH_X86_ASM_UPPER_HALF_H

#include <asm/pagedir.h>

#define BOOTTIME_DEFAULT_ATTRIBUTE 0x1f

BOOT_FUNCTION (void btclear (void));
BOOT_FUNCTION (void btputs (const char *));
BOOT_FUNCTION (void btputx (DWORD));
BOOT_FUNCTION (void btputd (DWORD));
BOOT_FUNCTION (void boot_prepare_paging_early (DWORD *));
BOOT_FUNCTION (void boot_entry (void));

#endif /* _ARCH_X86_ASM_UPPER_HALF_H */
