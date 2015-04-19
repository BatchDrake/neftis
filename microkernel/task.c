/*
 *    Basic task handling.
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
#include <types.h>

#include <layout.h>

#include <task/task.h>
#include <task/loader.h>
#include <task/msg.h>

#include <mm/anon.h>
#include <mm/vremap.h>

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

  set_task (tid, task);
  
  return KERNEL_SUCCESS_VALUE;
}

int
register_task (struct task *task)
{
  int errcode = 0;
  tid_t tid;
  
  DECLARE_CRITICAL_SECTION (task_register);

  CRITICAL_ENTER (task_register);

  if ((tid = __find_free_tid ()) == KERNEL_ERROR_VALUE)
  {
    errcode = -1;
    goto leave;
  }

  if (__ensure_tid (tid) == KERNEL_ERROR_VALUE)
  {
    errcode = -1;
    goto leave;
  }
  
  task->ts_tid = tid;
  
  if (set_task (tid, task) == KERNEL_ERROR_VALUE)
    errcode = -1;
  
leave:
  CRITICAL_LEAVE (task_register);

  return errcode;
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

/* There can only be one switch at a time */

DECLARE_CRITICAL_SECTION (switch_section);

/* Some interesting facts about context switches:
   A context switch cannot happen during a context switch.

   captainobvious.gif

   For correctly doing this, we need to save the CPU state and
   enter in a critical section. Then we use the CPU flags
   stored in switch_section to build a stack (if switching
   from TASK like in the case of schedule()) or the flags
   obtained from the interrupt frame (if switching from
   interrupt context) */

void
switch_lock (void)
{
  CRITICAL_ENTER (switch_section);
}

/* Note there's no critical leave. It's directly performed in assembly level */
void
switch_to (struct task *task)
{
  struct task *old;

  ASSERT (task->ts_state != TASK_STATE_EXITED);

  switch (get_current_context ())
  {
    case KERNEL_CONTEXT_BOOT_TIME:
      switch_lock ();

      /* Now, context switch becomes atomic */
      set_current_task (task);
      set_current_context (KERNEL_CONTEXT_TASK);

      ++task->ts_switch_count;
      __task_perform_switch (task);
      break; /* Not necessary, but nice. */
      
    case KERNEL_CONTEXT_TASK:
      old = get_current_task ();
      
      if (old == task)
        break;

      switch_lock ();

      set_current_task (task);

      ++task->ts_switch_count;
      __task_switch_from_current (old, task);
      break;
    
    case KERNEL_CONTEXT_INTERRUPT:
      old = get_current_task ();
      
      if (old == task)
        break;

      switch_lock ();

      set_current_context (KERNEL_CONTEXT_TASK);
      set_current_task (task);

      ++task->ts_switch_count;
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
      FAIL ("idle: awaken! (%d)\n", get_current_state ());
   
    kernel_pause (); /* Pause until next timer interrupt */
  }
}

/* Tasks must be in TASK_STATE_EXITED before being deleted */
void
task_destroy (struct task *task)
{
  if (task->ts_vm_space != NULL)
    kernel_object_ref_close (task->ts_vm_space);

  if (task->ts_msgq != NULL)
    msgq_destroy (task->ts_msgq);

  set_task (task->ts_tid, NULL);
  
  __free_task (task);
}

extern class_t vm_space_class;

void
preload_kernel_space (struct task *this_task)
{

}

struct task *
kernel_task_new (void (*entry) (void))
{
  struct task *task;
  
  if ((task = __alloc_task ()) == NULL)
    return NULL;

  task->ts_state = TASK_STATE_NEW;
  task->ts_type  = TASK_TYPE_KERNEL_THREAD;
  
  if ((task->ts_vm_space = kernel_object_open_task (current_kctx->kc_vm_space, task)) == NULL)
  {
    task_destroy (task);
    return NULL;
  }

  if ((task->ts_msgq = msgq_new (task, current_kctx->kc_msgq_vremap)) == NULL)
  {
    task_destroy (task);
    return NULL;
  }
  
  __task_config_start (task, entry, NULL);

  if (register_task (task) == -1)
  {
    task_destroy (task);
    return NULL;
  }

  return task;
}

struct task *
user_task_new_from_exec (const void *data, busword_t size)
{
  struct task *task, *old_task;
  struct vm_region *stack, *vremap;
  struct vm_space *space;
  loader_handle *handler;
  void (*entry) (void);
  void (*abi_entry) (void);
  void *tls;
  
  tid_t tid;

  if ((task = __alloc_task ()) == NULL)
    return NULL;

  if ((space = vm_space_load_from_exec (data, size, (busword_t *) &entry, (busword_t *) &abi_entry)) == KERNEL_INVALID_POINTER)
  {
    task_destroy (task);

    return KERNEL_INVALID_POINTER;
  }

  if ((vremap = vm_region_vremap_new (USER_MSGQ_VREMAP_START, __UNITS (USER_MSGQ_VREMAP_SIZE, PAGE_SIZE), VREGION_ACCESS_READ | VREGION_ACCESS_WRITE | VREGION_ACCESS_USER)) == NULL)
  {
    task_destroy (task);

    return KERNEL_INVALID_POINTER;
  }

  (void) vm_region_set_desc (vremap, "msgq");
  
  if (vm_space_add_region (space, vremap) != KERNEL_SUCCESS_VALUE)
  {
    error ("Executable overlaps vremap region\n");

    vm_region_destroy (vremap, NULL);

    task_destroy (task);
    
    return KERNEL_INVALID_POINTER;
  }
  
  if ((task->ts_vm_space = kernel_object_instance_task (&vm_space_class, space, task)) == NULL)
  {
    vm_space_destroy (space);

    task_destroy (task);

    return NULL;
  }

  if ((task->ts_msgq = msgq_new (task, vremap)) == NULL)
  {
    task_destroy (task);
    
    return NULL;
  }
  
  /* From now on, we can forget about task->ts_vm_space */
  /* Use space data to look for free space */
  if (PTR_UNLIKELY_TO_FAIL (stack = vm_region_stack (__task_get_user_stack_bottom (task), TASK_USR_STACK_PAGES)))
  {
    error ("couldn't allocate userspace stack!\n");

    task_destroy (task);

    return KERNEL_INVALID_POINTER;
  }

  if (UNLIKELY_TO_FAIL (vm_space_add_region (space, stack)))
  {
    error ("couldn't add stack to new space (bottom = %p)\n", stack->vr_virt_end);

    vm_region_destroy (stack, NULL);

    task_destroy (task);
    
    return KERNEL_INVALID_POINTER;
  }

  MANDATORY ((tls = virt2phys (space, USER_TLS_START)) != NULL);

  /* Clear page contents to ensure all tasks start with a clean TLS */
  memset (tls, 0, USER_TLS_PAGES * PAGE_SIZE);
  
  task->ts_tid   = tid;
  task->ts_state = TASK_STATE_NEW;
  task->ts_type  = TASK_TYPE_USER_THREAD;
  
  __task_config_start (task, entry, abi_entry);

  if (register_task (task) == -1)
  {
    task_destroy (task);

    return NULL;
  }

/*  printk ("Adding breakpoint...\n");
  
    (void) copy2virt (space, 0x8048e0d, "\xcc", 1);*/
  
  return task;
}

/* We don't need to make critical sections in this functions,
   they are run at boot time */
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

  preload_kernel_space (new);
  
  new->ts_state = TASK_STATE_SOFT_WAIT;
  new->ts_type  = TASK_TYPE_IDLE;

  MANDATORY (new->ts_vm_space = kernel_object_open_task (current_kctx->kc_vm_space, new));
  
  __task_config_start (new, idle_task, NULL);

  if (__register_task_with_tid (new, KERNEL_THREAD_IDLE) == KERNEL_ERROR_VALUE)
    FAIL ("Cannot allocate idle task!\n");
}

DEBUG_FUNC (__find_free_tid);
DEBUG_FUNC (__ensure_tid);
DEBUG_FUNC (get_task);
DEBUG_FUNC (set_task);
DEBUG_FUNC (__register_task_with_tid);
DEBUG_FUNC (register_task);
DEBUG_FUNC (get_kernel_thread);
DEBUG_FUNC (get_userspace_task);
DEBUG_FUNC (switch_lock);
DEBUG_FUNC (switch_to);
DEBUG_FUNC (idle_task);
DEBUG_FUNC (task_destroy);
DEBUG_FUNC (preload_kernel_space);
DEBUG_FUNC (kernel_task_new);
DEBUG_FUNC (user_task_new_from_exec);
DEBUG_FUNC (init_task_list_table);
DEBUG_FUNC (init_kernel_threads);
