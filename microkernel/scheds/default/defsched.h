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

#ifndef _SCHEDS_DEFAULT_DEFSCHED_H
#define _SCHEDS_DEFAULT_DEFSCHED_H

/* The following code will implement a dummy FCFS scheduler for managing early
   boot tasks. */

struct defsched_info
{
  int enabled;
  int sched_pending;
  struct task *idle;
  struct task *runqueue;
};

void defsched_register (void);

#endif /* _SCHEDS_DEFAULT_DEFSCHED_H */
