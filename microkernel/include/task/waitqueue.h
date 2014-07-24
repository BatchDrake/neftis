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
    
#ifndef _TASK_WAITQUEUE_H
#define _TASK_WAITQUEUE_H

#include <misc/list.h>
#include <lock/lock.h>

struct waiting_task_info
{
  LINKED_LIST;
  
  struct task *wt_task;
};

struct wait_queue
{
  spin_t wq_lock;
  
  struct waiting_task_info *wq_queue;
};

struct wait_queue *wait_queue_new (void);
struct waiting_task_info *waiting_task_info_new (void);
int wait_queue_destroy (struct wait_queue *);
int wait_queue_put_task (struct wait_queue *, struct task *);
struct waiting_task_info *
wait_queue_lookup_task (struct wait_queue *, struct task *);
void wait_queue_remove_task (struct wait_queue *, struct task *);
void wait_queue_remove_all (struct wait_queue *);
void signal (struct wait_queue *, int);
int  sleep (struct wait_queue *);

#endif /* _TASK_WAITQUEUE_H */

