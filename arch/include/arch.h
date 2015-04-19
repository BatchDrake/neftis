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
    
#ifndef _ARCH_H
#define _ARCH_H

#include <layout.h>
#include <task/syscall.h>

/* Misc functions */
void __halt (void);
void __pause (void);

/* Boot arch-dependant functions */
void boot_console_init (void);

void hw_memory_init (void);
void hw_interrupt_init (void);
void hw_early_irq_init (void);

void enable_interrupts (void);
void disable_interrupts (void);
void bugcheck (void);

/* VM arch-dependant functions */

# ifdef _MM_VM_H

int vm_kernel_space_map_io (struct vm_space *);
int vm_kernel_space_map_image (struct vm_space *);
int vm_user_space_map_image (struct vm_space *);
busword_t vm_get_prefered_stack_bottom (void);

# endif /* _MM_VM_H */

/* Low-level functions to manipulate page tables */
int   __vm_map_to (void *, busword_t, busword_t, busword_t, DWORD);
int   __vm_set_flags (void *, busword_t, busword_t, DWORD);
int   __vm_unset_flags (void *, busword_t, busword_t, DWORD);
void *__vm_alloc_page_table (void);
void  __vm_free_page_table (void *);
int   __vm_flush_pages (busword_t, busword_t);

/* Task management functions */
# ifdef _TASK_TASK_H

struct task *__alloc_task (void);
void         __free_task (struct task *);

void __task_config_start (struct task *, void (*) (), void (*) ());
void __task_perform_switch (struct task *);
void __task_switch_from_current (struct task *, struct task *);
void __task_switch_from_interrupt (struct task *, struct task *);
busword_t __task_get_user_stack_bottom (const struct task *);
busword_t __task_get_kernel_stack_top (const struct task *);
busword_t __task_get_kernel_stack_size (const struct task *);

# endif /* _TASK_TASK_H */

/* Timer functions */
# ifdef _IRQ_TIMER_H

void hw_set_timer_interrupt_freq (int);
void hw_timer_enable (void);
void hw_timer_disable (void);
int  hook_timer (int (*) (int, void *, void *));

/* Serial port functions */
void hw_serial_init (void);

/* Server functions */
void load_servers (void);

/* ABI functions */
int get_abi_vdso (const char *, const void **, uint32_t *);

# endif /* _IRQ_TIMER_H */
#endif /* _ARCH_H */
