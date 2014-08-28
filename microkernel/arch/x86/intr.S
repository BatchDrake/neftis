/*
 * intr.S: Manejo de interrupciones a bajo nivel.
 */

/*
 * This file is part of Eulex.
 *
 * Eulex is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Eulex is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Eulex.  If not, see <http://www.gnu.org/licenses/>.
 */

.globl x86_idt_flush
.globl x86_get_current_idt
.globl __restore_context

#include <asm/regs.h>

.text
.align   4
   
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_NOERRCODE 12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

/* Rutinas para el control de IRQs. */

ISR_IRQ 32
ISR_IRQ 33
ISR_IRQ 34
ISR_IRQ 35
ISR_IRQ 36
ISR_IRQ 37
ISR_IRQ 38
ISR_IRQ 39
ISR_IRQ 40
ISR_IRQ 41
ISR_IRQ 42
ISR_IRQ 43
ISR_IRQ 44
ISR_IRQ 45
ISR_IRQ 46
ISR_IRQ 47

/* IOAPIC interrupts */

ISR_IRQ 48
ISR_IRQ 49
ISR_IRQ 50
ISR_IRQ 51
ISR_IRQ 52
ISR_IRQ 53
ISR_IRQ 54
ISR_IRQ 55

ISR_NOERRCODE 113
ISR_NOERRCODE 114
ISR_NOERRCODE 115
ISR_NOERRCODE 116
ISR_NOERRCODE 117
ISR_NOERRCODE 118
ISR_NOERRCODE 119

/* Microkernel system calls */
        
ISR_NOERRCODE 160 /* Microkernel */
ISR_NOERRCODE 161 /* IPC */
ISR_NOERRCODE 162 /* VMO */

/* Bugcheck interrupt */
ISR_NOERRCODE 255
        
.extern x86_isr_handler
.extern get_task_ctx_data
.extern switch_section
        
.globl __intr_common
.globl __restore_context
.globl __task_restore_point
.globl __task_switch_from_current_asm
        
x86_idt_flush:
    movl 4(%esp), %eax
    lidt (%eax)
    ret
    
x86_get_current_idt:
    movl 4(%esp), %eax
    sidt (%eax)
    ret
  
__intr_common:
  cld
  SAVE_ALL
  
  pushl %esp
  
  call x86_isr_handler
  
  addl $4, %esp
  
__restore_context:
  RESTORE_ALL
  
  addl $8, %esp /* Omit interrupt number & error code */
  
  iret

/* __task_switch_from_current (struct task *, struct task *) */

        /*
        * SECOND_ARG
        * FIRST_ARG
        * RET
        */
.extern get_task_ctx_data
        
__task_switch_from_current_asm:
        pushl %eax

        /* Check task.c:switch_lock ()

        switch_lock has saved CPU flags
        and entered in a critical section.

        We get the flags prior to the
        call to CRITICAL_ENTER and use it
        to build the stack frame that will
        restore the previos state. Note
        that a call to sti() is not necessary,
        it's automatically enabled (if required)
        with iret
        */
        
        movl $switch_section, %eax
        movl 4(%eax), %eax
        
        pushl %eax
        pushl %cs
        pushl $__task_restore_point
        pushl $0
        pushl $0

        SAVE_ALL

        /*
           0x54(%esp): ret
           0x58(%esp): first arg
           0x5c(%esp): second arg
        */

        movl 0x5c(%esp), %eax
        pushl %eax
        call get_task_ctx_data
        addl $4, %esp

        /* Save ESP in struct task_ctx_data */
        movl %esp, 4(%eax)

        movl 0x60(%esp), %eax
        pushl %eax
        call __task_perform_switch
        /* This part will never be reached */

__task_restore_point:
        /* Here stack is already unwinded */
        addl $4, %esp
        ret
        