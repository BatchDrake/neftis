/*
 *    x86-dependant task functions
 *    Copyright (C) 2013  Gonzalo J. Carracedo
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
 
#include <task/task.h>

#include <mm/regions.h>
#include <mm/vm.h>

#include <asm/task.h>
#include <asm/regs.h>
#include <asm/seg.h>
#include <asm/io.h>

#include <arch.h>
#include <kctx.h>

extern int text_start;

busword_t
__task_get_user_stack_bottom (const struct task *task)
{
  return (busword_t) &text_start;
}

struct task_ctx_data *
get_task_ctx_data (struct task *task)
{
  return (struct task_ctx_data *) task->ts_arch_ctx_data;
}

/* This function is CRITICAL: without it, we can't
   build contexts when switching tasks! */
INLINE void
x86_init_stack_frame_kernel (struct x86_stack_frame *frame)
{
  memset (frame, 0, sizeof (struct x86_stack_frame));
  
  frame->segs.cs = GDT_SEGMENT_KERNEL_CODE;
  frame->segs.ds = GDT_SEGMENT_KERNEL_DATA;
  frame->segs.es = GDT_SEGMENT_KERNEL_DATA;
  frame->segs.gs = GDT_SEGMENT_KERNEL_DATA;
  frame->segs.fs = GDT_SEGMENT_KERNEL_DATA;
  frame->segs.ss = GDT_SEGMENT_KERNEL_DATA;

  frame->regs.esp = 0; /* Ignored */
  
  frame->priv.cs = GDT_SEGMENT_KERNEL_CODE;
  frame->priv.eflags = EFLAGS_INTERRUPT;

  GET_REGISTER ("%cr0", frame->cr0); 
  GET_REGISTER ("%cr3", frame->cr3);
}

INLINE void
x86_init_stack_frame_user (struct x86_stack_frame *frame)
{
  memset (frame, 0, sizeof (struct x86_stack_frame));
  
  frame->segs.cs = GDT_SEGMENT_USER_CODE | 3;
  frame->segs.ds = GDT_SEGMENT_USER_DATA | 3;
  frame->segs.es = GDT_SEGMENT_USER_DATA | 3;
  frame->segs.gs = GDT_SEGMENT_USER_DATA | 3;
  frame->segs.fs = GDT_SEGMENT_USER_DATA | 3;
  frame->segs.ss = GDT_SEGMENT_USER_DATA | 3;

  frame->regs.esp = 0; /* Ignored */
  
  frame->unpriv.cs     = GDT_SEGMENT_USER_CODE | 3;
  frame->unpriv.eflags = EFLAGS_INTERRUPT;
  frame->unpriv.old_ss = GDT_SEGMENT_USER_DATA | 3;

  GET_REGISTER ("%cr0", frame->cr0);
  GET_REGISTER ("%cr3", frame->cr3);
}

busword_t
__task_get_kernel_stack_top (const struct task *task)
{
  return (busword_t) task;
}

busword_t
__task_get_kernel_stack_size (const struct task *task)
{
  return KERNEL_MODE_STACK_PAGES;
}

struct task *
__alloc_task (void)
{
  struct task *new_task;
  struct task_ctx_data *data;
  busword_t stack_vaddr;

  if ((new_task = page_alloc (KERNEL_MODE_STACK_PAGES)) == NULL)
    return NULL;

  if ((stack_vaddr = kernel_vremap_ensure (KERNEL_MODE_STACK_PAGES)) == -1)
  {
    page_free (new_task, KERNEL_MODE_STACK_PAGES);
    
    return NULL;
  }

  if (kernel_vremap_map_pages (stack_vaddr, (busword_t) new_task, KERNEL_MODE_STACK_PAGES, VM_PAGE_READABLE | VM_PAGE_WRITABLE) == -1)
  {
    page_free (new_task, KERNEL_MODE_STACK_PAGES);
    
    (void) kernel_vremap_release (stack_vaddr, KERNEL_MODE_STACK_PAGES);
    
    return NULL;
  }

  /* Update kernel space */
  kernel_vremap_update_kernel ();
  
  memset (new_task, 0, KERNEL_MODE_STACK_PAGES << (__PAGE_BITS));

  data = get_task_ctx_data (new_task);

  data->stack_info.stack_vaddr = stack_vaddr;
    
  data->stack_info.stack_bottom = 
    (DWORD) new_task + 
    (KERNEL_MODE_STACK_PAGES << (__PAGE_BITS)) - sizeof (DWORD);
    
  data->stack_info.esp =
    stack_vaddr +
    (KERNEL_MODE_STACK_PAGES << (__PAGE_BITS)) - sizeof (DWORD);

  data->stack_info.stack_bottom_virtual = data->stack_info.esp;
  
  return new_task;
}

void
__free_task (struct task *task)
{
  struct task_ctx_data *data;

  data = get_task_ctx_data (task);

  kernel_vremap_release (data->stack_info.stack_vaddr, KERNEL_MODE_STACK_PAGES);
  
  page_free (task, KERNEL_MODE_STACK_PAGES);
}

busword_t
__task_find_stack_bottom (struct task *task)
{
  struct vm_region *curr;

  if (task->ts_vm_space == NULL)
    return KERNEL_ERROR_VALUE;

  curr = REFCAST (struct vm_space, task->ts_vm_space)->vs_regions;

  while (curr)
  {
    if (curr->vr_role == VREGION_ROLE_STACK)
      return curr->vr_virt_end - sizeof (busword_t) + 1;

    curr = LIST_NEXT (curr);
  }

  return KERNEL_ERROR_VALUE;
}

void
__task_config_start (struct task *task, void (*start) (), void (*abi_entry) ())
{
  struct x86_stack_frame *frameptr;
  struct task_ctx_data *data;

  data = get_task_ctx_data (task);

  /* Kernel threads (and the idle process) run at kernel stack */
  /* Forget about system processes. They don't make sense. For
     now, we have idle threads, kernel threads and userland threads.
    */

  /* Kernel stack is the same for all task types */
  data->stack_info.esp -= sizeof (struct x86_stack_frame);
  frameptr = (struct x86_stack_frame *) (data->stack_info.stack_bottom - sizeof (struct x86_stack_frame));
  
  if (task->ts_type == TASK_TYPE_KERNEL_THREAD || task->ts_type == TASK_TYPE_IDLE)
    x86_init_stack_frame_kernel (frameptr);
  else if (task->ts_type == TASK_TYPE_USER_THREAD)
  {
    x86_init_stack_frame_user (frameptr);
    
    if ((frameptr->unpriv.old_esp = __task_find_stack_bottom (task)) == KERNEL_ERROR_VALUE)
      FAIL ("Cannot find userspace stack\n");
  }
  else
    FAIL ("Don't know how to build start frame for this type of task!\n");

  /* Return address goes where? */

  if (abi_entry == NULL)
    frameptr->priv.eip = (busword_t) start;
  else
  {
    frameptr->regs.eax = (busword_t) start;
    frameptr->priv.eip = (busword_t) abi_entry;
  }
  
  frameptr->cr3 = (busword_t) (REFCAST (struct vm_space, task->ts_vm_space)->vs_pagetable);
}

void
__task_perform_switch (struct task *task)
{
  struct task_ctx_data *data;

  data = get_task_ctx_data (task);

  /* Just telling the CPU where to come back from user mode */
  
  x86_set_kernel_stack (data->stack_info.stack_bottom_virtual);

/*  hexdump (data->stack_info.esp, __ALIGN (data->stack_info.esp, PAGE_SIZE) - data->stack_info.esp); */
  
  __asm__ __volatile__ (".extern __restore_context\n"
                        "movl %0, %%esp           \n"
                        "jmp __restore_context    \n"
                        :: "g" (data->stack_info.esp));
}

void __task_switch_from_current_asm (struct task *current, struct task *next);

void
__task_switch_from_current (struct task *current, struct task *next)
{
  struct task_ctx_data *data;

  data = get_task_ctx_data (next);

  x86_set_kernel_stack (data->stack_info.stack_bottom_virtual);
  
  __task_switch_from_current_asm (current, next);
}

void
__task_switch_from_interrupt (struct task *current, struct task *next)
{
  struct task_ctx_data *data;

  /* TODO: use something like unlikely or shit */
  if (current != NULL)
  {
    data = get_task_ctx_data (current);

    data->stack_info.esp = (busword_t) get_interrupt_frame ();
  }
  
  __task_perform_switch (next);
}

void __intr_common (void);
void __restore_context (void);

DEBUG_FUNC (__task_get_user_stack_bottom);
DEBUG_FUNC (get_task_ctx_data);
DEBUG_FUNC (x86_init_stack_frame_kernel);
DEBUG_FUNC (x86_init_stack_frame_user);
DEBUG_FUNC (__task_get_kernel_stack_top);
DEBUG_FUNC (__task_get_kernel_stack_size);
DEBUG_FUNC (__alloc_task);
DEBUG_FUNC (__free_task);
DEBUG_FUNC (__task_find_stack_bottom);
DEBUG_FUNC (__task_config_start);
DEBUG_FUNC (__task_perform_switch);
DEBUG_FUNC (__task_switch_from_current);
DEBUG_FUNC (__task_switch_from_interrupt);
DEBUG_FUNC (__intr_common);
DEBUG_FUNC (__restore_context);
