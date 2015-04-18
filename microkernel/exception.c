/*
 *    Basic exception handling.
 *    Copyright (c) 2014 Gonzalo J. Carracedo
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
#include <task/exception.h>

#include <util.h>
#include <arch.h>
#include <kctx.h>

void
task_set_exception_handler (struct task *task, int exception, void (*handler) (struct task *, int, busword_t, busword_t, int))
{
  if (exception < 0 || exception >= EX_MAX)
    FAIL ("exception code unrecognized\n");

  task->ts_ex_handlers[exception] = handler;
}

static char *exception_description[] =
{
  "FPE",
  "SEGMENT_VIOLATION",
  "PRIV_INSTRUCTION",
  "ILL_INSTRUCTION"
};

/* Intended to be called from interrupt context only */
void
task_trigger_exception (struct task *task, int exception, busword_t textaddr, busword_t data, int code)
{
  DECLARE_CRITICAL_SECTION (except);

  ASSERT (get_current_context () == KERNEL_CONTEXT_INTERRUPT);
  ASSERT (get_current_task () == task);
  
  if (exception < 0 || exception >= EX_MAX)
    FAIL ("exception code unrecognized\n");

  if (task->ts_ex_handlers[exception] == NULL)
  {
    /* Process shall be killed */

    TASK_ATOMIC_ENTER (except);

    if (CRITICAL_IS_INSIDE (except))
      panic ("exception while atomic!\n");

    debug ("task %d: killed by exception %s in %p (data: %p, code: %d)\n", task->ts_tid, exception_description[exception], textaddr, data, code);
    
    (void) wake_up (task, TASK_STATE_EXITED, 0);

    task_destroy (task);

    schedule ();

    /* Attention: if interrupt happened in atomic context, the following call
       
       TASK_ATOMIC_LEAVE (except);

       won't trigger a task switch, as the scheduler is paused. We need to
       force it to resume */

    TASK_ATOMIC_ABORT ();
  }
  else
    (task->ts_ex_handlers[exception]) (task, exception, textaddr, data, code);
}

DEBUG_FUNC (task_set_exception_handler);
DEBUG_FUNC (task_trigger_exception);
