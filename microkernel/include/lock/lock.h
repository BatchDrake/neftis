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
    
#ifndef _LOCK_H
#define _LOCK_H

#include <types.h>

#define SPINLOCK_UNLOCKED 1
#define SPINLOCK_LOCKED   0

#define CS_NAME(name) JOIN (__critical_, name)

#define DECLARE_CRITICAL_SECTION(name) critsect_t CS_NAME (name) = {SPINLOCK_UNLOCKED, 0}

#define CRITICAL_ENTER(name) critical_enter (&CS_NAME (name).lock, &CS_NAME (name).flags)
#define CRITICAL_LEAVE(name) critical_leave (&CS_NAME (name).lock, CS_NAME (name).flags)
#define CRITICAL_IS_INSIDE(name) critical_is_inside (CS_NAME (name).flags)

#define CRITICAL_ABORT() critical_abort ()
/*
 * To make this "task atomic", we first save the scheduler state.
 * Now, some interrupts may happen, but it's ok if they change
 * the scheduler state as they're always restoring it.
 *
 * Then, we stop the scheduler. Now, task switches can't happen unless
 * we explicitly call switch_to from interrupt context. But that's an
 * error. Calls to wake_up or to schedule will only make scheduling
 * pending.
 *
 * Finally, save processor state & disable interrupts. Now this is
 * task-atomic. Nobody can borrow our CPU.
 */

/*
 * To leave everything as we found it:
 *
 * 1. CRITICAL_LEAVE ()
 * 2. sched_restore ()
 *
 * The only problem arises when an interrupt handler performs a call
 * to switch_to. However, this is an error: a raw call to switch_to
 * bypasses the scheduler and can't never be called but from the
 * scheduler.
 */
#define TASK_ATOMIC_ENTER(name)                 \
  do                                            \
  {                                             \
    sched_save (&CS_NAME (name).sched_state);   \
    pause ();                                   \
    CRITICAL_ENTER (name);                      \
  }                                             \
  while (0)

#define TASK_ATOMIC_LEAVE(name)                 \
  do                                            \
  {                                             \
    CRITICAL_LEAVE (name);                      \
    sched_restore (CS_NAME (name).sched_state); \
  }                                             \
  while (0)

#define TASK_ATOMIC_ABORT()             \
  do                                    \
  {                                     \
    CRITICAL_ABORT ();                  \
    resume ();                          \
  }                                     \
  while (0)


typedef volatile int spin_t;

typedef struct
{
  spin_t    lock;
  busword_t flags;
  int       sched_state;
}
critsect_t;

void spin_lock (spin_t *lock);
void spin_unlock (spin_t *lock);

void critical_enter (spin_t *, busword_t *);
void critical_leave (spin_t *, busword_t);
void critical_abort (void);
int  critical_is_inside (busword_t);

#endif /* _LOCK_H */

