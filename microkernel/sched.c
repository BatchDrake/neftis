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
#include <string.h>

#include <task/task.h>
#include <task/sched.h>

#include <arch.h>
#include <kctx.h>

struct sched *sys_sched_list = NULL; /* X-files */

#define SCHEDULER_CHECK()               \
  if (!likely (scheduler_present ()))   \
  {                                     \
    warning ("no scheduler present\n"); \
    return -1;                          \
  }

#define SCHEDULER_CHECK_VOID()          \
  if (!likely (scheduler_present ()))   \
  {                                     \
    warning ("no scheduler present\n"); \
    return;                             \
  }


#define SCHED_TEST_FIELD(x)                \
  if (sched->sc_##x == NULL)               \
  {                                        \
    warning ("unespecified " STRINGIFY (x) \
      ", scheduler not complete.\n");      \
                                           \
    return -1;                             \
  }
  

struct sched *
sched_lookup (const char *name)
{
  struct sched *this;
  
  this = sys_sched_list;
  
  while (this != NULL)
  {
    if (strcmp (this->sc_name, name) == 0)
      return this;
      
    this = (struct sched *) LIST_NEXT (this);
  }
  
  return KERNEL_INVALID_POINTER;
}

int
sched_register (struct sched *sched)
{
  ASSERT (sched != NULL);
  
  SCHED_TEST_FIELD (name);
  SCHED_TEST_FIELD (setprio);
  SCHED_TEST_FIELD (getprio);
  SCHED_TEST_FIELD (pause);
  SCHED_TEST_FIELD (resume);
  SCHED_TEST_FIELD (wake_up);
  SCHED_TEST_FIELD (sched);
  SCHED_TEST_FIELD (find_task);
  SCHED_TEST_FIELD (sys_timer);
  SCHED_TEST_FIELD (init);
  SCHED_TEST_FIELD (release);
  
  list_insert_head ((void *) &sys_sched_list, (void *) sched);
  
  return 0;
}


INLINE struct sched *
__curr_sched (void)
{
  return current_kctx->kc_scheduler;
}

int
scheduler_present (void)
{
  return __curr_sched () != NULL;
}
  
int
setprio (struct task *task, prio_t *prio)
{
  SCHEDULER_CHECK ();
  
  return (__curr_sched ())->sc_setprio (task, prio);
}
  
int
getprio (struct task *task, prio_t *prio)
{
  SCHEDULER_CHECK ();
  
  return (__curr_sched ())->sc_getprio (task, prio);
}

  
int
pause (void)
{
  SCHEDULER_CHECK ();
  
  return (__curr_sched ())->sc_pause ();
}

  
int
resume (void)
{
  SCHEDULER_CHECK ();
  
  return (__curr_sched ())->sc_resume ();
}

int
wake_up (struct task *task, int state, int why)
{
  SCHEDULER_CHECK ();
  
  return (__curr_sched ())->sc_wake_up (task, state, why);
}

void
schedule (void)
{
  SCHEDULER_CHECK_VOID ();
  
  (__curr_sched ())->sc_sched ();
}

static int
schedule_on_tick (int unused, void *unused1, void *unused2)
{
  if (likely (scheduler_present ()))
    (__curr_sched ())->sc_sys_timer ();
}

/* Pause scheduler before doing this */
void
switch_scheduler (struct sched *new)
{
  if (likely (scheduler_present ()))
  {
    (__curr_sched ())->sc_release ();
    /* TODO: transfer runqueues */
  }
  
  set_current_scheduler (new);
  
  (__curr_sched ())->sc_init ();
}

/* The following code will implement a dummy FCFS scheduler for managing early
   boot tasks. */

struct defsched_info
{
  int enabled;
  int sched_pending;
  struct task *idle;
  struct task *runqueue;
};

#define CURRENT_INFO ((struct defsched_info *) (__curr_sched ())->sc_private)

struct defsched_info *
defsched_info_new (void)
{
  CONSTRUCTOR_BODY_OF_STRUCT (defsched_info);
}

void 
defsched_info_destroy (struct defsched_info *info)
{
  spfree (info);
}

int
defsched_init (void)
{
  struct defsched_info *info;
  
  RETURN_ON_PTR_FAILURE (info = defsched_info_new ());
  
  (__curr_sched ())->sc_private = info;
  
  CURRENT_INFO->idle = get_kernel_thread (KERNEL_THREAD_IDLE);
  
  return 0;
}

void
defsched_release (void)
{
  spfree ((__curr_sched ())->sc_private);
  (__curr_sched ())->sc_private = NULL;
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
  ASSERT (task != NULL);

  if (task->ts_state == op)
    return 0;
    
  switch (op)
  {
    case TASK_STATE_RUNNING:
      if (defsched_put_in_runqueue (task) == -1)
        FAIL ("failed to put process in runqueue\n");
      
      task->ts_state = op;
      
      if (reason == WAKEUP_INTERRUPT ||
          reason == WAKEUP_MUTEX ||
          reason == WAKEUP_IPC)
            switch_to (task);
      break;
      
    default:
      if (task->ts_state == TASK_STATE_RUNNING)
        if (defsched_remove_from_runqueue (task) == -1)
          FAIL ("not in runqueue but running. how?\n");
      
      task->ts_state = op;
      
      break;
  }
  
  return 0;
}

int sched_pending;

int
defsched_pause (void)
{
  CURRENT_INFO->enabled = 0;
}

int
defsched_resume (void)
{
  CURRENT_INFO->enabled = 1;
  
  if (CURRENT_INFO->sched_pending)
    schedule ();
}

void
defsched_sched (void)
{
  struct task *new;
  
  CURRENT_INFO->sched_pending = 0;
  
  circular_list_scroll_next ((void **) &CURRENT_INFO->runqueue);
    
  new = circular_list_get_head ((void **) &CURRENT_INFO->runqueue);
  
  if (new == NULL)
    new = CURRENT_INFO->idle;
  
  if (get_current_context () == KERNEL_CONTEXT_INTERRUPT)
    switch_to (new);
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
  FIELD (pause),
  FIELD (resume),
  FIELD (wake_up),
  FIELD (sched),
  FIELD (find_task),
  FIELD (sys_timer),
  FIELD (init),
  FIELD (release)
};
#undef FIELD

/* TODO: standarize error values */
void
scheduler_init (void)
{
  struct sched *defsched;
  
  if (hook_timer (schedule_on_tick) == KERNEL_ERROR_VALUE)
    FAIL ("couldn't hook scheduling subsystem to system timer\n");
  
  if (sched_register (&defsched_info) == KERNEL_ERROR_VALUE)
    FAIL ("failed to register default scheduler\n");

  switch_scheduler (&defsched_info);
  
  resume ();
}

DEBUG_FUNC (sched_lookup);
DEBUG_FUNC (sched_register);
DEBUG_FUNC (__curr_sched);
DEBUG_FUNC (scheduler_present);
DEBUG_FUNC (setprio);
DEBUG_FUNC (getprio);
DEBUG_FUNC (pause);
DEBUG_FUNC (resume);
DEBUG_FUNC (wake_up);
DEBUG_FUNC (schedule);
DEBUG_FUNC (schedule_on_tick);
DEBUG_FUNC (switch_scheduler);
DEBUG_FUNC (defsched_info_new);
DEBUG_FUNC (defsched_info_destroy);
DEBUG_FUNC (defsched_init);
DEBUG_FUNC (defsched_release);
DEBUG_FUNC (defsched_setprio);
DEBUG_FUNC (defsched_getprio);
DEBUG_FUNC (defsched_put_in_runqueue);
DEBUG_FUNC (defsched_remove_from_runqueue);
DEBUG_FUNC (defsched_wake_up);
DEBUG_FUNC (defsched_pause);
DEBUG_FUNC (defsched_resume);
DEBUG_FUNC (defsched_sched);
DEBUG_FUNC (defsched_sys_timer);
DEBUG_FUNC (defsched_find_task);
DEBUG_FUNC (scheduler_init);

