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
    
#ifndef _ASM_INTERRUPT_H
#define _ASM_INTERRUPT_H

#include <types.h>

#define X86_INT_DIVIDE_BY_ZERO               0
#define X86_INT_DEBUG                        1
#define X86_INT_NMI                          2
#define X86_INT_BREAKPOINT                   3
#define X86_INT_OVERFLOW                     4
#define X86_INT_BOUND_RANGE_EXCEEDED         5
#define X86_INT_INVALID_OPCODE               6
#define X86_INT_DEVICE_NOT_AVAILABLE         7
#define X86_INT_DOUBLE_FAULT                 8
#define X86_INT_COPROCESSOR_SEGMENT_OVERRUN  9
#define X86_INT_INVALID_TSS                  10
#define X86_INT_SEGMENT_NOT_PRESENT          11
#define X86_INT_STACK_SEGMENT_FAULT          12
#define X86_INT_GENERAL_PROTECTION_FAULT     13
#define X86_INT_PAGE_FAULT                   14
#define X86_INT_15                           15
#define X86_INT_X87_FLOATING_POINT_EXCEPTION 16
#define X86_INT_ALIGNMENT_CHECK              17
#define X86_INT_MACHINE_CHECK                18
#define X86_INT_SIMD_FPE                     19
#define X86_INT_SECURITY_EXCEPTION           30

#define I386_INTERRUPT_GATE                  0xE /* Disables interrupts */
#define I386_TRAP_GATE                       0xF  

#define IDT_SYSTEM                           0x10
#define IDT_PRIV(r)                          ((r) << 5)
#define IDT_PRESENT                          0x80

#define KERNEL_BUGCHECK_INTERRUPT            0xff

#define RAISE_INTERRUPT(x) __asm__ __volatile__ ("int $" STRINGIFY (x));

struct idt_entry
{
  WORD  base_lo;             
  WORD  sel;                 
  BYTE  always0;             
  BYTE  flags;               
  WORD  base_hi;             
} PACKED;

struct idt_ptr
{
  WORD   limit;
  struct idt_entry *base;                
} PACKED;

void x86_idt_flush (struct idt_ptr *);
void x86_get_current_idt (struct idt_ptr *);
void x86_init_all_gates (void);

#endif /* _ASM_INTERRUPT_H */

