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

/* TODO: protect this! */
static struct task ***task_list_table;
uint32_t              task_last_tid;

#define TASK_TABLE_INDEX_BITS (__PAGE_BITS - BUSWORD_SIZE_LOG)
#define TASK_TABLE_MASK ((1 << __PAGE_BITS) - 1)
#define MAX_TASKS (1 << (TASK_TABLE_INDEX_BITS << 1))

tid_t
__find_free_tid (void)
{
  tid_t i, curr;
  tid_t table_idx, table_entry;
  struct task **task_list;
  
  for (i = 0; i < MAX_TASKS; ++i)
  {
    curr = i + task_last_tid;

    table_idx = (curr >> TASK_TABLE_INDEX_BITS) & TASK_TABLE_MASK;
    table_entry = curr & TASK_TABLE_MASK;

    /* Alloc a pointer table if it doesn't exist */
    if (task_list_table[table_idx] == NULL)
    {
      if ((task_list_table[table_idx] = page_alloc (1)) == NULL)
        return KERNEL_ERROR_VALUE;  /* Failed miserably */
      else
        memset (task_list_table[table_idx], 0, PAGE_SIZE);
    }
    
    task_list = task_list_table[table_idx];

    if (task_list[table_entry] == NULL)
      return curr;
  }

  return KERNEL_ERROR_VALUE;
}

int
__ensure_tid (tid_t tid)
{
  struct task **task_list;
  tid_t i, table_idx, table_entry;
  
  table_idx = (tid >> TASK_TABLE_INDEX_BITS) & TASK_TABLE_MASK;
  table_entry = tid & TASK_TABLE_MASK;

  /* Alloc a pointer table if it doesn't exist */
  if (task_list_table[table_idx] == NULL)
  {
    if ((task_list_table[table_idx] = page_alloc (1)) == NULL)
      return KERNEL_ERROR_VALUE; /* Failed miserably */
    else
      memset (task_list_table[table_idx], 0, PAGE_SIZE);
  }
  
  task_list = task_list_table[table_idx];

  return KERNEL_SUCCESS_VALUE;
}

struct task *
get_task (tid_t tid)
{
  struct task **task_list;
  tid_t i, table_idx, table_entry;

  if (tid >= MAX_TASKS)
    return NULL;
  
  table_idx = (tid >> TASK_TABLE_INDEX_BITS) & TASK_TABLE_MASK;
  table_entry = tid & TASK_TABLE_MASK;

  if (task_list_table[table_idx] == NULL)
    return NULL;

  task_list = task_list_table[table_idx];
  
  return task_list[table_entry];
}

int
set_task (tid_t tid, struct task *task)
{
  struct task **task_list;
  tid_t i, table_idx, table_entry;

  if (tid >= MAX_TASKS)
    return KERNEL_ERROR_VALUE;
  
  table_idx = (tid >> TASK_TABLE_INDEX_BITS) & TASK_TABLE_MASK;
  table_entry = tid & TASK_TABLE_MASK;

  if (task_list_table[table_idx] == NULL)
    return KERNEL_ERROR_VALUE;

  task_list = task_list_table[table_idx];
  
  task_list[table_entry] = task;

  return KERNEL_SUCCESS_VALUE;
}

int
__register_task_with_tid (struct task *task, tid_t tid)
{
  if (__ensure_tid (tid) == KERNEL_ERROR_VALUE)
    return KERNEL_ERROR_VALUE;

  if (get_task (tid) != NULL)
    return KERNEL_ERROR_VALUE;

  
  return KERNEL_SUCCESS_VALUE;
}

struct task *
get_kernel_thread (tid_t tid)
{
  if (tid >= MAX_KERNEL_THREADS)
    return NULL;
    
  return get_task (tid);
}

struct task *
get_userspace_task (tid_t pid)
{
  if (pid < MAX_KERNEL_THREADS)
    return NULL;
  
  return get_task (pid - MAX_KERNEL_THREADS);
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
task_destroy (struct task *task)
{
  debug ("stub!\n");
}

/* This should lock */
struct task *
kernel_task_new (void (*entry) (void))
{
  struct task *task;
  tid_t tid;
  
  if ((tid = __find_free_tid ()) == KERNEL_ERROR_VALUE)
    return NULL;

  if (__ensure_tid (tid) == KERNEL_ERROR_VALUE)
    return NULL;
  
  if ((task = __alloc_task ()) == NULL)
    return NULL;

  if (set_task (tid, task) == KERNEL_ERROR_VALUE)
  {
    task_destroy (task);
    return NULL;
  }

  task->ts_tid   = tid;
  task->ts_state = TASK_STATE_SOFT_WAIT;
  task->ts_type  = TASK_TYPE_KERNEL_THREAD;
  task->ts_vm_space = current_kctx->kc_vm_space;
  
  __task_config_start (task, entry);
  
  return task;
}

struct task *
system_process_new (void)
{
  struct task *task;
  struct vm_space *space;
  tid_t tid;

  PTR_RETURN_ON_PTR_FAILURE (space = vm_bare_process_space ());
  
  if ((tid = __find_free_tid ()) == KERNEL_ERROR_VALUE)
    return NULL;

  if (__ensure_tid (tid) == KERNEL_ERROR_VALUE)
    return NULL;
  
  if ((task = __alloc_task ()) == NULL)
    return NULL;

  if (set_task (tid, task) == KERNEL_ERROR_VALUE)
  {
    task_destroy (task);
    return NULL;
  }

  task->ts_tid   = tid;
  task->ts_state = TASK_STATE_SOFT_WAIT;
  task->ts_type  = TASK_TYPE_SYS_PROCESS;
  task->ts_vm_space = space;
  
  __task_config_start (task, NULL);
  
  return task;
}

void
init_task_list_table (void)
{
  if ((task_list_table = page_alloc (1)) == NULL)
    FAIL ("Couldn't allocate task-list table\n");

  memset (task_list_table, 0, PAGE_SIZE);

  debug ("This system allows up to %d tasks\n", MAX_TASKS);
}

void
init_kernel_threads (void)
{
  struct task *new;

  init_task_list_table ();
  
  MANDATORY (new = __alloc_task ());
    
  new->ts_state = TASK_STATE_NEW;
  new->ts_type  = TASK_TYPE_IDLE;
  
  __task_config_start (new, idle_task);

  if (__register_task_with_tid (new, KERNEL_THREAD_IDLE) == KERNEL_ERROR_VALUE)
    FAIL ("Cannot allocate iddle task!\n");
}

DEBUG_FUNC (__find_free_tid);
DEBUG_FUNC (__register_task_with_tid);
DEBUG_FUNC (get_task);
DEBUG_FUNC (get_userspace_task);
DEBUG_FUNC (get_kernel_thread);
DEBUG_FUNC (switch_to);
DEBUG_FUNC (idle_task);
DEBUG_FUNC (init_kernel_threads);
DEBUG_FUNC (system_process_new);
