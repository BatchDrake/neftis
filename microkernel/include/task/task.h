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
    
#ifndef _TASK_TASK_H
#define _TASK_TASK_H

#include <mm/vm.h>
#include <misc/list.h>

#include <arch.h>
#include <kctx.h>

#define TASK_STATE_NEW          0
#define TASK_STATE_READY        1
#define TASK_STATE_RUNNING      2
#define TASK_STATE_SOFT_WAIT    3
#define TASK_STATE_HARD_WAIT    4
#define TASK_STATE_CACHED       5
#define TASK_STATE_SWAPPED      6
#define TASK_STATE_EXITED       7
#define TASK_STATE_CUSTOM       8

#define TASK_TYPE_IDLE          0
#define TASK_TYPE_KERNEL_THREAD 1
#define TASK_TYPE_USERLAND      2

#define MAX_KERNEL_THREADS      16

#define KERNEL_THREAD_IDLE      0
#define KERNEL_THREAD_DFRIRQD   1

#define SCHED_BUFFER_PTRS 16

typedef busword_t               tid_t;

/* We're gonna design task structures this way, to avoid cache conflicts and
   profit from spatial locality:
   
   task_new () ---> __alloc_task () [arch dependant]
   __alloc_task () will allocate one (and posibly more) pages and put
   struct task somewhere inside it, among another arch-dependand control
   structures. The rest of the space there will be used as the kernel-mode
   stack. __alloc_task is also in charge of putting any structure in its correct
   position and making room for enough stack space. This internal layout
   is opaque to the high-level kernel API (the files inside microkernel/
   directory). */

struct sched_buffer
{
  CIRCULAR_LIST;
  
  void *sb_misc_data[SCHED_BUFFER_PTRS];
};
 
struct task
{
  struct sched_buffer ts_sched_info;

  tid_t               ts_tid;
  int                 ts_state;
  int                 ts_type;
  
  char                ts_arch_ctx_data[1];
};

# ifdef KERNEL_CONTEXT_TASK

INLINE int
get_current_state (void)
{
  ASSERT (get_current_context () == KERNEL_CONTEXT_TASK);
  
  return current_kctx->kc_current->ts_state;
}

INLINE void
set_current_state (int state)
{
  ASSERT (get_current_context () == KERNEL_CONTEXT_TASK);
  
  current_kctx->kc_current->ts_state = state;
}
# endif /* KERNEL_CONTEXT_TASK */

void switch_to (struct task *task);
struct task *get_kernel_thread (tid_t);
struct task *get_userspace_task (tid_t);
struct task *get_task (tid_t);

int   __register_task_with_tid (struct task *, tid_t);
tid_t __find_free_tid (void);
int   __ensure_tid (tid_t);

void init_kernel_threads (void);
int set_task (tid_t, struct task *);
struct task *kernel_task_new (void (*) (void));
void task_destroy (struct task *);
#endif /* _TASK_TASK_H */

