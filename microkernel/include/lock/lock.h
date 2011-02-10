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

typedef volatile int spin_t;

typedef struct
{
  spin_t             lock;
  volatile busword_t value;
  struct wait_queue *wq;
}
mutex_t;

void spin_lock (spin_t *lock);
void spin_unlock (spin_t *lock);

int  init_mutex (mutex_t *);
void free_mutex (mutex_t *);

void acquire_mutex (mutex_t *);
void release_mutex (mutex_t *);
int  try_mutex (mutex_t *);

#endif /* _LOCK_H */

