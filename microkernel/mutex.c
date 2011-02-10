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

#include <kctx.h>

int init_mutex (mutex_t *);
void free_mutex (mutex_t *);

void acquire_mutex (mutex_t *);
void release_mutex (mutex_t *);
int  try_mutex (mutex_t *);


void
mutex_wake_up_all (mutex_t *mutex)
{
  struct waiting_task_info *this, *next;
  struct wait_queue *wq;
  
  wq = mutex->wq;
  
  if ((this = (struct waiting_task_info *)
    LIST_HEAD (*((void **) &wq->wq_queue))) == NULL)
    return;
  
  while (this)
  {
    next = (struct waiting_task_info *) LIST_NEXT (this);
    wake_up (this->wt_task, TASK_STATE_RUNNING, WAKEUP_MUTEX);
    wait_queue_remove_task (wq, this->wt_task);
    this = next;
  }
}


int
init_mutex (mutex_t *mutex)
{
  RETURN_ON_PTR_FAILURE (mutex->wq = wait_queue_new ());
  
  mutex->lock  = SPINLOCK_UNLOCKED;
  mutex->value = 1;
  
  return KERNEL_SUCCESS_VALUE;
}

void
acquire_mutex (mutex_t *mutex)
{
  if (!scheduler_present ())
    return;
    
  spin_lock (&mutex->lock);
  
  while (!mutex->value)
  {
    if (UNLIKELY_TO_FAIL (wait_queue_put_task (mutex->wq,
                          get_current_task ())))
      FAIL ("oh noes\n");
    spin_unlock (&mutex->lock);
    set_current_state (TASK_STATE_SOFT_WAIT);
    schedule ();
    spin_lock (&mutex->lock);
  }
  
  mutex->value--;
  spin_unlock (&mutex->lock);
}

void
release_mutex (mutex_t *mutex)
{
  if (!scheduler_present ())
    return;
    
  mutex->value = 1;
  mutex_wake_up_all (mutex);
}


