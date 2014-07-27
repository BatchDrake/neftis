/*
 *    Scheduler API
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
#include <scheds/default/defsched.h>

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
  SCHED_TEST_FIELD (get_state);
  SCHED_TEST_FIELD (set_state);
  SCHED_TEST_FIELD (wake_up);
  SCHED_TEST_FIELD (sched);
  SCHED_TEST_FIELD (find_task);
  SCHED_TEST_FIELD (sys_timer);
  SCHED_TEST_FIELD (init);
  SCHED_TEST_FIELD (release);
  
  list_insert_head ((void *) &sys_sched_list, (void *) sched);
  
  return 0;
}

int
scheduler_present (void)
{
  return get_current_scheduler () != NULL;
}
  
int
setprio (struct task *task, prio_t *prio)
{
  SCHEDULER_CHECK ();
  
  return (get_current_scheduler ())->sc_setprio (task, prio);
}
  
int
getprio (struct task *task, prio_t *prio)
{
  SCHEDULER_CHECK ();
  
  return (get_current_scheduler ())->sc_getprio (task, prio);
}

void
sched_save (int *state)
{
  *state = (get_current_scheduler ())->sc_get_state ();
}

void
sched_restore (int state)
{
  (get_current_scheduler ())->sc_set_state (state);
}

int
pause (void)
{
  SCHEDULER_CHECK ();
  
  (get_current_scheduler ())->sc_set_state (0);
}

int
resume (void)
{
  SCHEDULER_CHECK ();

  (get_current_scheduler ())->sc_set_state (1);
}

int
wake_up (struct task *task, int state, int why)
{
  SCHEDULER_CHECK ();
  
  return (get_current_scheduler ())->sc_wake_up (task, state, why);
}

void
schedule (void)
{
  SCHEDULER_CHECK_VOID ();
  
  (get_current_scheduler ())->sc_sched ();
}

static int
schedule_on_tick (int unused, void *unused1, void *unused2)
{
  if (likely (scheduler_present ()))
    (get_current_scheduler ())->sc_sys_timer ();
}

/* Pause scheduler before doing this */
void
switch_scheduler (struct sched *new)
{
  if (likely (scheduler_present ()))
  {
    (get_current_scheduler ())->sc_release ();
    /* TODO: transfer runqueues */
  }
  
  set_current_scheduler (new);
  
  (get_current_scheduler ())->sc_init ();
}

void
scheduler_init (void)
{
  struct sched *defsched;
  
  if (hook_timer (schedule_on_tick) == KERNEL_ERROR_VALUE)
    FAIL ("couldn't hook scheduling subsystem to system timer\n");

  defsched_register ();
  
  resume ();
}

DEBUG_FUNC (sched_lookup);
DEBUG_FUNC (sched_register);
DEBUG_FUNC (get_current_scheduler);
DEBUG_FUNC (scheduler_present);
DEBUG_FUNC (setprio);
DEBUG_FUNC (getprio);
DEBUG_FUNC (pause);
DEBUG_FUNC (resume);
DEBUG_FUNC (wake_up);
DEBUG_FUNC (schedule);
DEBUG_FUNC (schedule_on_tick);
DEBUG_FUNC (switch_scheduler);
DEBUG_FUNC (scheduler_init);

