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

#include <types.h>

#include <irq/timer.h>
#include <misc/hook.h>
#include <task/task.h>

#include <arch.h>
#include <kctx.h>

struct task *task1, *task2;

#define GET_REGISTER(reg, where)                \
  __asm__ __volatile__ ("mov %" reg ", %%eax\n" \
                        "mov %%eax, %0" : "=g" (where) :: "eax");
INLINE DWORD
get_eflags (void)
{
  DWORD ret;
  
  __asm__ __volatile__ ("pushf");
  __asm__ __volatile__ ("pop %0" : "=g" (ret)); /* We use this instead of
   popping directly to %eax to avoid warnings. TODO: do it in assembly */
   
   return ret;
}
                      
void
infinite_loop (void)
{
  for (;;)
  {
    pause ();
    puts ("\033[0;34m");
    resume ();
    puts ("\xdb\xdb\xdb\xdb\xdb LULZ \xdb\xdb\xdb\xdb\xdb");
  }
}

void
another_task (void)
{
  for (;;)
  {
    pause ();
    puts ("\033[1;33m");
    resume ();
    puts ("\xdb\xdb\xdb SPECTRUM \xdb\xdb\xdb");
  }
}

void
early_timers_init (void)
{
  hw_set_timer_interrupt_freq (HZ);
  
  hw_timer_enable ();
  
  /*
  task1 = __alloc_task ();
  task2 = __alloc_task ();
  
  __task_config_start (task1, infinite_loop);
  __task_config_start (task2, another_task);
  
  wake_up (task1, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
  wake_up (task2, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
  */
}

DEBUG_FUNC (infinite_loop);
DEBUG_FUNC (another_task);
DEBUG_FUNC (early_timers_init);

