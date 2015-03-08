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
    
#ifndef _ASM_REGS_H
#define _ASM_REGS_H


#include <types.h>


#define GET_REGISTER(reg, where)                \
  __asm__ __volatile__ ("mov %" reg ", %%eax\n" \
                        "mov %%eax, %0" : "=g" (where) :: "eax");

#define SET_REGISTER(reg, where)                \
  __asm__ __volatile__ ("mov %0, %%eax\n"       \
                        "mov %%eax, %" reg :: "g" (where) : "eax");
  
#define CR0_PAGING_ENABLED 0x80000000

/* Extended processor flags to use with EFLAGS */

#define EFLAGS_CARRY     (1 <<  0)
#define EFLAGS_PARITY    (1 <<  2)
#define EFLAGS_ADJUST    (1 <<  4)
#define EFLAGS_ZERO      (1 <<  6)
#define EFLAGS_SIGN      (1 <<  7)
#define EFLAGS_TRAP      (1 <<  8)
#define EFLAGS_INTERRUPT (1 <<  9)
#define EFLAGS_DIRECTION (1 << 10)
#define EFLAGS_OVERFLOW  (1 << 11)
#define EFLAGS_NESTED    (1 << 14)
#define EFLAGS_RESUME    (1 << 16)
#define EFLAGS_VM8086    (1 << 17)
#define EFLAGS_ALIGNMENT (1 << 18)
#define EFLAGS_VIRT_INTR (1 << 19)
#define EFLAGS_INT_PENDG (1 << 20)
#define EFLAGS_IDENTIFY  (1 << 21)

# ifndef ASM
#  define SAVE_ALL                \
  ".extern kernel_pagedir\n"      \
  "pusha\n"                       \
  "movl %%cr3, %%eax\n"           \
  "pushl %%eax\n"                 \
  "movl %%cr0, %%eax\n"           \
  "pushl %%eax\n"                 \
  "movl kernel_pagedir, %%eax\n"  \
  "movl %%eax, %%cr3\n"           \
  "push %%ss\n"                   \
  "push %%fs\n"                   \
  "push %%gs\n"                   \
  "push %%es\n"                   \
  "push %%ds\n"                   \
  "push %%cs\n"                   \
   
# else

  .macro ISR_ERRCODE num
     .global isr\num
     .align   4
  isr\num:

     pushl $\num
     jmp __intr_common
  .endm

  .macro ISR_NOERRCODE num
     .global isr\num
     .align   4
  isr\num:

    pushl $0
    pushl $\num
    jmp __intr_common
  .endm

  .macro ISR_IRQ num
     .global isr\num
     .align   4
  isr\num:
    pushl $0
    pushl $\num
    jmp __intr_common
  .endm
    
  .macro SAVE_ALL
    .extern kernel_pagedir
    pusha
    movl %cr3, %eax
    pushl %eax
    movl %cr0, %eax
    pushl %eax
    movl kernel_pagedir, %eax
    movl %eax, %cr3
    push %ss
    push %fs
    push %gs
    push %es
    push %ds
    push %cs
  .endm
  
  .macro RESTORE_ALL
    addl $4, %esp
  
    pop %ds
    pop %es
    pop %gs
    pop %fs
      
    addl $4, %esp /* %ss can't be updated here */
      
    popl %eax
    movl %eax, %cr0
    popl %eax
    movl %eax, %cr3
    popa
    
  .endm
# endif
# ifndef ASM
struct x86_common_regs
{
  DWORD edi;
  DWORD esi;
  DWORD ebp;
  DWORD esp;
  DWORD ebx;
  DWORD edx;
  DWORD ecx;
  DWORD eax;
};

struct x86_segment_regs
{
  DWORD cs;
  DWORD ds;
  DWORD es;
  DWORD gs;
  DWORD fs;
  DWORD ss;
};

struct x86_int_frame
{
  DWORD     error;
  physptr_t eip;
  DWORD     cs;
  DWORD     eflags;
};

struct x86_int_frame_privchg
{
  DWORD     error;
  physptr_t eip;
  DWORD     cs;
  DWORD     eflags;
  DWORD     old_esp;
  DWORD     old_ss;
};

struct x86_stack_frame
{
  struct x86_segment_regs segs;
  DWORD                   cr0;
  DWORD                   cr3;
  struct x86_common_regs  regs;
  DWORD                   int_no;
  union
  {
    struct x86_int_frame         priv;
    struct x86_int_frame_privchg unpriv;
  };
};

void x86_regdump (struct x86_stack_frame *);

# endif /* ASM */
#endif /* _ASM_REGS_H */

