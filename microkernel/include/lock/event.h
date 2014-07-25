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
    
#ifndef _LOCK_EVENT_H
#define _LOCK_EVENT_H

#include <types.h>

#include "lock.h"

#define EMPTY_EVENT {1, 0, 0, {0}}
#define AUTO_RESET_EVENT {1, 0, 1, {0}}

#define DECLARE_EVENT(name) event_t name = EMPTY_EVENT
#define DECLARE_AUTO_RESET_EVENT(name) event_t name = AUTO_RESET_EVENT

typedef struct event
{
  spin_t lock;
  int signaled;
  int auto_reset;
  struct wait_queue wq; /* We define it statically, so we can declare it globally with no intializer */
}
event_t;

void init_event (event_t *);

event_t *event_new (void);
void event_destroy (event_t *);

void event_wait (event_t *);
void event_signal (event_t *);
void event_clear (event_t *);
void event_set_auto_reset (event_t *, int);

#endif /* _LOCK_H */

