/*
 *    Event implementation
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

#include <task/task.h>
#include <task/waitqueue.h>
#include <task/sched.h>

#include <lock/lock.h>
#include <lock/event.h>

#include <mm/salloc.h>

void
init_event (event_t *ev)
{
  memset (&ev, 0, sizeof (event_t));
  
  ev->lock = 1;
  ev->wq.wq_lock = 1; 
}

event_t *
event_new (void)
{
  event_t *new;

  if ((new = salloc (sizeof (event_t))) == NULL)
    return NULL;

  init_event (new);

  return new;
}

void
event_destroy (event_t *ev)
{
  sfree (ev);
}

void
event_wait (event_t *ev)
{
  DECLARE_CRITICAL_SECTION (section);

  TASK_ATOMIC_ENTER (section);
  
  spin_lock (&ev->lock);

  if (!ev->signaled)
    __sleep (&ev->wq);
  
  spin_unlock (&ev->lock);

  TASK_ATOMIC_LEAVE (section);   /* Actual wait will happen here */
}

void
event_signal (event_t *ev)
{
  DECLARE_CRITICAL_SECTION (section);

  TASK_ATOMIC_ENTER (section);
  
  spin_lock (&ev->lock);
  
  if (!ev->auto_reset)
  {
    if (!ev->signaled)
    {
      ev->signaled = 1;
      __signal_all (&ev->wq, WAKEUP_REASON_EVENT);
    }
  }
  else
    __signal_all (&ev->wq, WAKEUP_REASON_EVENT);

  spin_unlock (&ev->lock);

  TASK_ATOMIC_LEAVE (section);
}

void
event_clear (event_t *ev)
{
  DECLARE_CRITICAL_SECTION (section);

  TASK_ATOMIC_ENTER (section);

  spin_lock (&ev->lock);
  
  ev->signaled = 0;
  
  spin_unlock (&ev->lock);

  TASK_ATOMIC_LEAVE (section);
}

void
event_set_auto_reset (event_t *ev, int flag)
{
  DECLARE_CRITICAL_SECTION (section);

  TASK_ATOMIC_ENTER (section);

  spin_lock (&ev->lock);
  
  ev->auto_reset = flag;
  
  spin_unlock (&ev->lock);

  TASK_ATOMIC_LEAVE (section);
}

