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
 
#ifndef _KCTX_H
#define _KCTX_H

#include <mm/regions.h>
#include <mm/salloc.h>
#include <mm/vm.h>

#include <task/task.h>
#include <task/sched.h>

#define MAX_NUMA_REGIONS 16

#ifndef NR_MAXCPUS
# define NR_MAXCPUS 1
#endif

#if NR_MAXCPUS != 1
#  error NR_MAXCPUS exceeds 1 (SMP support not currently available)
#endif

#define KERNEL_CONTEXT_BOOT_TIME 0
#define KERNEL_CONTEXT_INTERRUPT 1
#define KERNEL_CONTEXT_TASK      2

struct kernel_context
{
  int                kc_context;
  struct mm_regions *kc_numa_regions[MAX_NUMA_REGIONS];
  struct vm_space   *kc_vm_space;
  struct task       *kc_current;
  struct sched      *kc_scheduler;
  void              *kc_intr_frame;
};

struct kernel_context *get_kctx (int);
int    get_cpu (void);
void   kctx_init (void);
struct mm_region *kctx_get_numa_friendly (int);

#define current_kctx (get_kctx (get_cpu ()))

INLINE int
get_current_context (void)
{
  return current_kctx->kc_context;
}

INLINE void
set_current_context (int ctx)
{
  current_kctx->kc_context = ctx;
}

INLINE struct task*
get_current_task (void)
{
  return current_kctx->kc_current;
}

INLINE tid_t
gettid (void)
{
  return (get_current_task ())->ts_tid;
}

INLINE void
set_current_task (struct task* task)
{
  current_kctx->kc_current = task;
}

INLINE void *
get_interrupt_frame (void)
{
  return current_kctx->kc_intr_frame;
}

INLINE void
set_interrupt_frame (void *frame)
{
  current_kctx->kc_intr_frame = frame;
}

INLINE struct sched *
get_current_scheduler (void)
{
  return current_kctx->kc_scheduler;
}

INLINE void
set_current_scheduler (struct sched *scheduler)
{
  current_kctx->kc_scheduler = scheduler; 
}

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

#endif /* _KCTX_H */

