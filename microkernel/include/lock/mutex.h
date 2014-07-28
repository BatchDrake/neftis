/*
 *    Mutex implementation
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
    
#ifndef _LOCK_MUTEX_H
#define _LOCK_MUTEX_H

#include <types.h>

#include "lock.h"

#define WAKEUP_REASON_MUTEX 20

#define MUTEX_LOCKED   0
#define MUTEX_UNLOCKED 1

#define MUTEX_INITIALIZER {MUTEX_UNLOCKED, {SPINLOCK_UNLOCKED, NULL}, NULL}

#define DECLARE_MUTEX(name) mutex_t name = MUTEX_INITIALIZER

typedef struct
{
  int                value;
  struct wait_queue  wq;
  struct task       *owner;
}
mutex_t;

void init_mutex (mutex_t *, int);
void up (mutex_t *);
void down (mutex_t *);
int  try_down (mutex_t *);

#endif /* _LOCK_MUTEX_H */
