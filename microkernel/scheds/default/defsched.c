/*
 *    Default FIFO scheduler with no priority
 *    Copyright (C) 2014  Gonzalo J. Carracedo
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
#include <string.h>

#include <task/task.h>
#include <task/sched.h>

#include <kctx.h>
#include "defsched.h"

struct defsched_info *
defsched_info_new (void)
{
  CONSTRUCTOR_BODY_OF_STRUCT (defsched_info);
}

void 
defsched_info_destroy (struct defsched_info *info)
{
  sfree (info);
}

int
defsched_init (void)
{
  struct defsched_info *info;
  
  RETURN_ON_PTR_FAILURE (info = defsched_info_new ());
  
  (get_current_scheduler ())->sc_private = info;

  CURRENT_INFO->idle = get_kernel_thread (KERNEL_THREAD_IDLE);

  return 0;
}

void
defsched_release (void)
{
  sfree ((get_current_scheduler ())->sc_private);
  (get_current_scheduler ())->sc_private = NULL;
}

int
defsched_setprio (struct task *task, prio_t *ignored)
{
  return 0;
}


int
defsched_getprio (struct task *task, prio_t *new)
{
  *new = 0;
  return 0;
}

/* TODO: generalize this */
int
defsched_task_in_runqueue (struct task *task)
{
  struct task *this_task;
  
  this_task = CURRENT_INFO->runqueue;
  
  while (this_task)
  {
    if (this_task == task)
      return 1;
    
    this_task = (struct task *) LIST_NEXT (this_task);
    
    if (this_task == CURRENT_INFO->runqueue)
      break;
  }
  
  return 0;
}


/* TODO: use backpointers to store about the actual scheding info */

int
defsched_put_in_runqueue_hiprio (struct task *task)
{
  circular_list_insert_head ((void *) &CURRENT_INFO->runqueue, task);
  
  return 0;
}

int
defsched_put_in_runqueue (struct task *task)
{
  circular_list_insert_tail ((void *) &CURRENT_INFO->runqueue, task);
  
  return 0;
}

int
defsched_remove_from_runqueue (struct task *task)
{
  circular_list_remove_element ((void *) &CURRENT_INFO->runqueue, task);

  return 0;
}

/* TODO: make scheduler aware of waitqueues */
int
defsched_wake_up (struct task *task, int op, int reason)
{
  int delayed = op & WAKEUP_DELAYED;
  int again   = op & WAKEUP_AGAIN;

  op &= TASK_STATE_MASK;
  
  ASSERT (task != NULL);

  if (task->ts_state == op && !again)
  {
    FAIL ("task %d (%p) already in state %d \n", task->ts_tid, task, op);
    return 0;
  }
  
  switch (op)
  {
    case TASK_STATE_RUNNING:
      if (reason > 0)
      {
        if (defsched_put_in_runqueue_hiprio (task) == -1)
          FAIL ("failed to put process in runqueue\n");
      }
      else
      {
        if (defsched_put_in_runqueue (task) == -1)
          FAIL ("failed to put process in runqueue\n");
      }
      
      task->ts_state = op;
      task->ts_wakeup_reason = reason;

      if (!delayed)
        schedule ();
      
      break;
      
  case TASK_STATE_EXITED:
    if (task->ts_state == TASK_STATE_RUNNING)
      if (defsched_remove_from_runqueue (task) == -1)
        FAIL ("not in runqueue but running. how?\n");
    
    task->ts_state = op;

    break;
    
  default:
    if (task->ts_state == TASK_STATE_RUNNING)
    {
      if (defsched_remove_from_runqueue (task) == -1)
        FAIL ("not in runqueue but running. how?\n");
    }
    
    task->ts_state = op;
    
    break;
  }
  
  return 0;
}


void
defsched_set_state (int state)
{
  CURRENT_INFO->enabled = state;

  if (state && CURRENT_INFO->sched_pending)
    schedule ();
}

int
defsched_get_state (void)
{
  return CURRENT_INFO->enabled;
}

void
defsched_sched (void)
{
  DECLARE_CRITICAL_SECTION (section);
  
  struct task *new;

  if (CURRENT_INFO->enabled)
  {
    TASK_ATOMIC_ENTER (section);
    
    circular_list_scroll_next ((void **) &CURRENT_INFO->runqueue);
    
    new = circular_list_get_head ((void **) &CURRENT_INFO->runqueue);
  
    if (new == NULL)
      new = CURRENT_INFO->idle;

    CURRENT_INFO->sched_pending = 0;

    TASK_ATOMIC_LEAVE (section);
    
    switch_to (new);
  }
  else
    CURRENT_INFO->sched_pending = 1;
}

void
defsched_sys_timer (void)
{
  if (CURRENT_INFO->enabled)
    defsched_sched ();
  else
    CURRENT_INFO->sched_pending = 1;
}

struct task *
defsched_find_task (struct task *unused)
{
  warning ("stub\n");
  
  return NULL;
}

#define FIELD(fieldname) .sc_##fieldname = defsched_##fieldname
static struct sched defsched_info =
{
  .sc_name = "default",
  FIELD (setprio),
  FIELD (getprio),
  FIELD (set_state),
  FIELD (get_state),
  FIELD (wake_up),
  FIELD (sched),
  FIELD (find_task),
  FIELD (sys_timer),
  FIELD (init),
  FIELD (release)
};
#undef FIELD

void
defsched_register (void)
{
  if (sched_register (&defsched_info) == KERNEL_ERROR_VALUE)
    FAIL ("failed to register default scheduler\n");

  switch_scheduler (&defsched_info);
}

DEBUG_FUNC (defsched_register);
DEBUG_FUNC (defsched_info_new);
DEBUG_FUNC (defsched_info_destroy);
DEBUG_FUNC (defsched_init);
DEBUG_FUNC (defsched_release);
DEBUG_FUNC (defsched_setprio);
DEBUG_FUNC (defsched_getprio);
DEBUG_FUNC (defsched_put_in_runqueue);
DEBUG_FUNC (defsched_remove_from_runqueue);
DEBUG_FUNC (defsched_wake_up);
DEBUG_FUNC (defsched_set_state);
DEBUG_FUNC (defsched_get_state);
DEBUG_FUNC (defsched_sched);
DEBUG_FUNC (defsched_sys_timer);
DEBUG_FUNC (defsched_find_task);
