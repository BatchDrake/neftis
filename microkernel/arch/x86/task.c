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
 
#include <task/task.h>

#include <mm/regions.h>
#include <mm/vm.h>

#include <asm/task.h>
#include <asm/regs.h>
#include <asm/seg.h>

#include <arch.h>
#include <kctx.h>

INLINE DWORD
get_eflags (void)
{
  DWORD ret;
  
  __asm__ __volatile__ ("pushf");
  __asm__ __volatile__ ("pop %0" : "=g" (ret)); /* We use this instead of
   popping directly to %eax to avoid warnings. TODO: do it in assembly */
   
   return ret;
}

INLINE int
esp_is_sane (struct task *task, DWORD esp)
{
  return ((DWORD) task + sizeof (struct task)) > esp &&
         esp < ((DWORD) task + KERNEL_MODE_STACK_PAGES << __PAGE_BITS);
}

INLINE struct task_ctx_data *
get_task_ctx_data (struct task *task)
{
  return (struct task_ctx_data *) task->ts_arch_ctx_data;
}

/* This function is CRITICAL: without it, we can't
   build contexts when switching tasks! */
INLINE void
x86_init_stack_frame (struct x86_stack_frame *frame)
{
  memset (frame, 0, sizeof (struct x86_stack_frame));
  
  frame->segs.cs = GDT_SEGMENT_KERNEL_CODE;
  frame->segs.ds = GDT_SEGMENT_KERNEL_DATA;
  frame->segs.es = GDT_SEGMENT_KERNEL_DATA;
  frame->segs.gs = GDT_SEGMENT_KERNEL_DATA;
  frame->segs.fs = GDT_SEGMENT_KERNEL_DATA;
  frame->segs.ss = GDT_SEGMENT_KERNEL_DATA;

  frame->regs.esp = (DWORD) frame; /* Unnecesary, but cool */
  
  frame->priv.cs = GDT_SEGMENT_KERNEL_CODE;
  frame->priv.eflags = EFLAGS_INTERRUPT;

  GET_REGISTER ("%cr0", frame->cr0); /* You gave me a headache */
  GET_REGISTER ("%cr3", frame->cr3); /* You too motherfucker */
}

struct task *
__alloc_task (void)
{
  struct task *new_task;
  struct task_ctx_data *data;
  
  if ((new_task = page_alloc (KERNEL_MODE_STACK_PAGES)) == NULL)
    return NULL;
    
  memset (new_task, 0, KERNEL_MODE_STACK_PAGES << (__PAGE_BITS));

  data = get_task_ctx_data (new_task);
  
  data->stack_info.stack_bottom = 
    (DWORD) new_task + 
    (KERNEL_MODE_STACK_PAGES << (__PAGE_BITS)) - sizeof (DWORD);
    
  data->stack_info.esp     = data->stack_info.stack_bottom;
  data->stack_info.useresp = 0; /* Define later. */
  
  return new_task;
}

busword_t
__task_find_stack_bottom (struct task *task)
{
  struct vm_region *curr;

  if (task->ts_vm_space == NULL)
    return KERNEL_ERROR_VALUE;

  curr = task->ts_vm_space->vs_regions;

  while (curr)
  {
    if (curr->vr_type == VREGION_TYPE_STACK)
      return curr->vr_virt_end - sizeof (busword_t) + 1;

    curr = LIST_NEXT (curr);
  }

  return KERNEL_ERROR_VALUE;
}

void
__task_config_start (struct task *task, void (*start) ())
{
  struct x86_stack_frame *frameptr;
  struct task_ctx_data *data;

  data = get_task_ctx_data (task);

  /* Kernel threads (and the idle process) run at kernel stack */
  /* Sys processes are exactly like usermode processes but with
     with a higher level of privileges (kernel mode, to be precise).

     The problem arrives when handling interrupts. As there is no
     privilege switch, virtual space stays the same and the stack
     should be shared with the kernel space. The only possible
     solution here is to allocate stack with 1:1 mapping. Which, by the way,
     is difficult its address space is big as collisions can occur.

     We can switch the ESP to physycal at context switch, but that's
     slow has hell.

     TODO: think about it.
    */
  
  if (task->ts_type == TASK_TYPE_KERNEL_THREAD ||
      task->ts_type == TASK_TYPE_IDLE)
  {
    /* Fix this, this should be an absolute assignation */
    data->stack_info.esp -= sizeof (struct x86_stack_frame);
    frameptr = (struct x86_stack_frame *) data->stack_info.esp;
  }
  else if (task->ts_type == TASK_TYPE_SYS_PROCESS)
  {
    if ((data->stack_info.esp = __task_find_stack_bottom (task)) == KERNEL_ERROR_VALUE)
      FAIL ("Something weird happened: cannot find stack bottom in a SYS_PROCESS task!\n");

    data->stack_info.esp -= sizeof (struct x86_stack_frame);

    if ((frameptr = (struct x86_stack_frame *) virt2phys (task->ts_vm_space, data->stack_info.esp)) == NULL)
      FAIL ("Cannot convert virtual address %p to physycal address\n", data->stack_info.esp);

    debug ("System process stack pointer:  %p\n", data->stack_info.esp);
    debug ("System process physical frame: %p\n", frameptr);
  }
  else
    FAIL ("Don't know how to build start frame for this type of task!\n");

  x86_init_stack_frame (frameptr);
  
  /* Return address goes where? */
  
  frameptr->priv.eip = (physptr_t) start;

  if (task->ts_vm_space != NULL)
    frameptr->cr3 = (physptr_t) task->ts_vm_space->vs_pagetable;
  else
  {
    FAIL ("Building task of type %d (entry: %p) with no vm_space!\n",
	  task->ts_type, start);
  }
}


void
__task_perform_switch (struct task *task)
{
  struct task_ctx_data *data;
  
  data = get_task_ctx_data (task);
  
  __asm__ __volatile__ (".extern __restore_context\n"
                        "movl %0, %%esp           \n"
                        "jmp __restore_context    \n"
                        :: "g" (data->stack_info.esp));
}


void
__task_switch_from_current (struct task *current, struct task *next)
{
  struct task_ctx_data *data;
  extern void __task_restore_point;

  data = get_task_ctx_data (current);
  
  __asm__ __volatile__ ("pushf\n"
                        "push %%cs\n"
                        "push %0\n"
                        "push $0\n" /* no error code */
                        "push $0\n" /* no interrupt number */
                        SAVE_ALL /* save all registers */
                        :: "g" (&__task_restore_point));
                        
  /* Update stack information */
  __asm__ __volatile__ ("movl %%esp, %0" : "=g" (data->stack_info.esp));
  
  
  /* That's where __task_perform_switch will look for esp */
  
  __task_perform_switch (next); /* Go, go, go */
  
  __asm__ __volatile__ (".globl __task_restore_point\n"
                        "__task_restore_point:\n");
                      
  /* Stack cleanup instructions are here */
}


void
__task_switch_from_interrupt (struct task *current, struct task *next)
{
  struct task_ctx_data *data;
  
  /* TODO: use something like unlikely or shit */
  if (current != NULL)
  {
    data = get_task_ctx_data (current);
  
    data->stack_info.esp = get_interrupt_frame ();
  }
  
  __task_perform_switch (next);
}

DEBUG_FUNC (get_eflags);
DEBUG_FUNC (esp_is_sane);
DEBUG_FUNC (get_task_ctx_data);
DEBUG_FUNC (x86_init_stack_frame);
DEBUG_FUNC (__alloc_task);
DEBUG_FUNC (__task_find_stack_bottom);
DEBUG_FUNC (__task_config_start);
DEBUG_FUNC (__task_perform_switch);
DEBUG_FUNC (__task_switch_from_current);
DEBUG_FUNC (__task_switch_from_interrupt);
