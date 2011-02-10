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
#include <types.h>

#include <task/task.h>

#include <util.h>
#include <arch.h>
#include <kctx.h>

/* TODO: Improve this as a per-cpu array. */
static struct task *kernel_threads[MAX_KERNEL_THREADS];

struct task *
get_kernel_thread (tid_t task)
{
  if (task >= MAX_KERNEL_THREADS)
    return NULL;
    
  return kernel_threads[task];
}

void
switch_to (struct task *task)
{
  struct task *old;
  
  switch (get_current_context ())
  {
    case KERNEL_CONTEXT_BOOT_TIME:
      set_current_task (task);
      set_current_context (KERNEL_CONTEXT_TASK);
      __task_perform_switch (task);
      break; /* Not necessary, but nice. */
      
    case KERNEL_CONTEXT_TASK:
      old = get_current_task ();
      
      if (old == task)
        break;
    
      set_current_task (task);
    
      __task_switch_from_current (old, task);
      break;
    
    case KERNEL_CONTEXT_INTERRUPT:
      old = get_current_task ();
      
      if (old == task)
        break;
        
      set_current_context (KERNEL_CONTEXT_TASK);
      set_current_task (task);
      
      __task_switch_from_interrupt (old, task);
      
      break;
      
    default:
      FAIL ("unknown kernel context %d\n", get_current_context ());
  }
}

void
idle_task (void)
{
  struct task *myself;
  
  myself = get_kernel_thread (KERNEL_THREAD_IDLE);
  
  for (;;)
  {
    if (get_current_state () != TASK_STATE_SOFT_WAIT)
      FAIL ("idle: awaken! awaken! awaken!\n");
  }
}

void
init_kernel_threads (void)
{
  struct task *new;
  
  MANDATORY (new = __alloc_task ());
    
  new->ts_state = TASK_STATE_SOFT_WAIT;
  new->ts_type  = TASK_TYPE_IDLE;
  
  __task_config_start (new, idle_task);
  
  kernel_threads[KERNEL_THREAD_IDLE] = new;
}

DEBUG_FUNC (get_kernel_thread);
DEBUG_FUNC (switch_to);
DEBUG_FUNC (idle_task);
DEBUG_FUNC (init_kernel_threads);

