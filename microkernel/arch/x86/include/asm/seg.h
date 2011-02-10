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
    
#ifndef _ASM_SEG_H
#define _ASM_SEG_H

#ifndef ASM
#  include <types.h>
#endif /* !ASM */

#define GDT_SEGMENT_KERNEL_CODE 0x8
#define GDT_SEGMENT_KERNEL_DATA 0x10
#define GDT_SEGMENT_USER_CODE   0x18
#define GDT_SEGMENT_USER_DATA   0x20
#define GDT_SEGMENT_TSS         0x28

#define GDT_ENTRY(x) (&gdt[(x) >> 3])

#define GDT_MAX_SEGMENTS        6

#define GDT_GRANULARITY_PAGES   0x08
#define GDT_SELECTOR_32_BIT     0x04

#define GDT_ACCESS_ACCESED      0x01
#define GDT_ACCESS_READWRITE    0x02
#define GDT_ACCESS_GROW_DOWN    0x04
#define GDT_ACCESS_CONFORMING   0x04 /* Far jumps from lower privs allowed */
#define GDT_ACCESS_EXECUTABLE   0x08
#define GDT_ACCESS_SEGMENT      0x10
#define GDT_ACCESS_RING(x)      ((x) << 5)
#define GDT_ACCESS_PRESENT      0x80

#ifndef ASM

struct tss
{
  WORD  link;
  WORD  reserved_1;
  DWORD esp0;
  WORD  ss0;
  WORD  reserved_2; 
  DWORD esp1;
  WORD  ss1;
  WORD  reserved_3;
  DWORD esp2;
  WORD  ss2;
  WORD  reserved_4;
  
  DWORD pagedir;
  
  DWORD eip, eflags;
  
  DWORD eax, ecx, edx, ebx, esp, ebp, esi, edi;
  
  WORD  es, reserv_es;
  WORD  cs, reserv_cs;
  WORD  ss, reserv_ss;
  WORD  ds, reserv_ds;
  WORD  fs, reserv_fs;
  WORD  gs, reserv_gs;
  WORD  ldtr, reserv_ldtr;
  
  WORD  reserv_iopb;
  
  WORD  iopb; 
} PACKED;

struct gdt_entry
{
  WORD limit_low;
  WORD base_low;
  BYTE base_mid;
  BYTE access;
  BYTE limit_high:4;
  BYTE flags:4;
  BYTE base_high;
} PACKED;

struct gdt_ptr
{
  WORD limit;
  struct gdt_entry *base;
} PACKED;

void gdt_init (void);
void x86_get_current_gdt (struct gdt_ptr *);
void x86_flush_gdt (struct gdt_ptr *);
void x86_refresh_segments (void);

#endif /* !ASM */

#endif /* _ASM_SEG_H */

