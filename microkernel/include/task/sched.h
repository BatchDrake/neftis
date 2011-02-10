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
    
#ifndef _TASK_SCHED_H
#define _TASK_SCHED_H

#include <misc/list.h>

#include <task/task.h>
#include <task/waitqueue.h>

#define  WAKEUP_EXPLICIT  0 /* Process is awaken from another task */
#define  WAKEUP_INTERRUPT 1 /* Process is awaken from interrupt */
#define  WAKEUP_MUTEX     2 /* Process is awaken on mutex release */
#define  WAKEUP_IPC       3 /* Process is awaken to process a message */

/* We must note that the above "wake up reasons" exist just for making the
   scheduler aware of the reason of our wake up request, i.e. if a process
   is waiting for a I/O operation to be completed we want it to be rapidly
   executed, even before an higher priority user task. */
   
typedef int32_t prio_t;

struct sched
{
  LINKED_LIST;
  
  char *sc_name;
  
  int  (*sc_setprio)   (struct task *, prio_t *);
  int  (*sc_getprio)   (struct task *, prio_t *);
  
  int  (*sc_pause)     (void);
  int  (*sc_resume)    (void);
  
  int  (*sc_wake_up)   (struct task *, int, int);
  
  void (*sc_sched)     (void);
  void (*sc_sys_timer) (void);
  
  struct task *  
       (*sc_find_task) (struct task *);
       
  int (*sc_init)       (void);
  void (*sc_release)   (void);
  
  void *sc_private;
};

struct sched *sched_new (void);
struct sched *sched_lookup (const char *);

int  sched_register (struct sched *);

int  scheduler_present (void);

int  setprio (struct task *, prio_t *);
int  getprio (struct task *, prio_t *);

int  pause (void);
int  resume (void);

int  wake_up (struct task *, int, int);

void schedule (void);

void scheduler_init (void);

#endif /* _TASK_SCHED_H */

