/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) <year>  <name of author>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
    
#ifndef _ASM_PAGEDIR_H
#define _ASM_PAGEDIR_H

/* Note: Use global pages in order to make kernel jumps faster */

#define PAGE_FLAG_PRESENT       1
#define PAGE_FLAG_WRITABLE      2 /* see cr0's WP bit for details */
#define PAGE_FLAG_USERLAND      4
#define PAGE_FLAG_WRITE_THROUGH 8
#define PAGE_FLAG_CACHE_DISABLE 16
#define PAGE_FLAG_ACCESSED      32
#define PAGE_FLAG_DIRTY         64
#define PAGE_FLAG_4MIB_PAGES    128
#define PAGE_FLAG_GLOBAL        256

#define PAGE_TABLE(addr)        (((DWORD) addr) >> 22)
#define PAGE_ENTRY(addr)        ((((DWORD) addr) >> 12) & 1023)

#define PAGE_BITS               0xfffff000
#define CONTROL_BITS            (~PAGE_BITS)

#define BOOT_FUNCTION(expr)       expr __attribute__ ((section (".bootcode")))
#define BOOT_SYMBOL(expr)       expr __attribute__ ((section (".bootdata")))

#define PAGE_TABLE_DFL_FLAGS    \
  (PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE)
  
DWORD* x86_alloc_new_pagedir (void);
int  x86_pagedir_map_range (DWORD *, DWORD, DWORD, DWORD, DWORD);
void x86_debug_pagedir (DWORD *);
int x86_pagedir_set_flags (DWORD *, DWORD, DWORD, DWORD);
int x86_pagedir_unset_flags (DWORD *, DWORD, DWORD, DWORD);
void x86_free_pagedir (DWORD *);

#endif /* _ASM_PAGEDIR_H */

