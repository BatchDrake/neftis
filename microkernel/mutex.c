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
#include <task/sched.h>
#include <task/waitqueue.h>

#include <lock/lock.h>
#include <lock/mutex.h>

#include <kctx.h>

void
init_mutex (mutex_t *mutex, int val)
{
  mutex->wq.wq_lock  = SPINLOCK_UNLOCKED;
  mutex->wq.wq_queue = NULL;
  mutex->value       = val;
  mutex->owner       = get_current_task ();
}

void
up (mutex_t *mutex)
{
  DECLARE_CRITICAL_SECTION (mutex_up);

  ASSERT (get_current_context () == KERNEL_CONTEXT_TASK);
  
  TASK_ATOMIC_ENTER (mutex_up);
  
  if (mutex->value == MUTEX_LOCKED)
  {
    if (mutex->owner != get_current_task ())
      FAIL ("Mutex not ours (owner: %d, me: %d)\n",
            mutex->owner->ts_tid, gettid ());
    
    mutex->value = MUTEX_UNLOCKED;

    /* We signal only one, as only one can actually acquire it */
    signal (&mutex->wq, WAKEUP_REASON_MUTEX);
  }
  else
    FAIL ("Releasing already-released mutex\n");
  
  TASK_ATOMIC_LEAVE (mutex_up);
}

void
down (mutex_t *mutex)
{
  int locked = 0;

  ASSERT (get_current_context () == KERNEL_CONTEXT_TASK);
          
  DECLARE_CRITICAL_SECTION (mutex_down);

  while (!locked)
  {
    TASK_ATOMIC_ENTER (mutex_down);

    if (mutex->value == MUTEX_UNLOCKED)
    {
      mutex->value = MUTEX_LOCKED;
      mutex->owner = get_current_task ();

      ++locked;
    }
    else
      __sleep (&mutex->wq); /* Let's wait until it's free */

    TASK_ATOMIC_LEAVE (mutex_down);
  }
}

int
try_down (mutex_t *mutex)
{
  int result = -1;

  ASSERT (get_current_context () == KERNEL_CONTEXT_TASK);
  
  DECLARE_CRITICAL_SECTION (mutex_down);

  TASK_ATOMIC_ENTER (mutex_down);

  if (mutex->value == MUTEX_UNLOCKED)
  {
    mutex->value = MUTEX_LOCKED;
    mutex->owner = get_current_task ();
    
    result = 0;
  }
  
  TASK_ATOMIC_LEAVE (mutex_down);

  return result;
}

