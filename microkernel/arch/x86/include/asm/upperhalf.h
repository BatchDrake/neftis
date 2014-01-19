#ifndef _ARCH_X86_ASM_UPPER_HALF_H
#define _ARCH_X86_ASM_UPPER_HALF_H

#include <asm/pagedir.h>

#define BOOTTIME_DEFAULT_ATTRIBUTE 0x1f

/* Functions in case we need to output something REALLY REALLY early */
BOOT_FUNCTION (void boot_halt (void));
BOOT_FUNCTION (void boot_screen_clear (BYTE));
BOOT_FUNCTION (void boot_puts (const char *));
BOOT_FUNCTION (void boot_print_hex (DWORD));
BOOT_FUNCTION (void boot_print_dec (DWORD));

BOOT_FUNCTION (void boot_entry (void));

#endif /* _ARCH_X86_ASM_UPPER_HALF_H */
