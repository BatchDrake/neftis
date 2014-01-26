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

#include <misc/list.h>

#include <console/console.h>
#include <irq/irq.h>
#include <irq/timer.h>

#include <mm/kmalloc.h>
#include <mm/spalloc.h>
#include <mm/vm.h>

#include <task/task.h>
#include <task/sched.h>

#include <arch.h>
#include <kctx.h>

extern struct console *syscon;

void
kernel_thread_one (void)
{
  int k = 0;
  char spinner[] = "-\\|/";

  int line;

  line = syscon->pos_y;
  
  for (;;)
  {
    if ((k++ % 100) == 0)
    {
      //pause (); /* If we don't pause the scheduler, the context switch happens right in the middle of console_gotoxy or printk, making the output messy */

      disable_interrupts ();
      
      console_gotoxy (syscon, 0, line);

      printk ("task1: [%c] %d ", spinner[k / 100 & 3], k / 100);

      enable_interrupts ();
    }
  }
}

void
kernel_thread_two (void)
{
  int k = 0;
  char spinner[] = "-\\|/";
  int line;
  
  line = syscon->pos_y;

  for (;;)
  {
    if ((k++ % 100) == 0)
    {
      disable_interrupts ();
    
      console_gotoxy (syscon, 40, line);

      printk ("task2: [%c] %d ", spinner[k / 100 & 3], k / 100);

      enable_interrupts ();
    }
  }
}

void
test_kthreads (void)
{
  struct task *task1, *task2;
  int i;
  
  if ((task1 = kernel_task_new (kernel_thread_one)) == NULL)
    FAIL ("Cannot allocate task!\n");

  if ((task2 = kernel_task_new (kernel_thread_two)) == NULL)
    FAIL ("Cannot allocate task!\n");

  printk ("\nReady... ");

  for (i = 0; i < 10000000; ++i)
    do_nothing ();

  printk ("steady... ");
  for (i = 0; i < 10000000; ++i)
    do_nothing ();

  printk ("go!\n\n");
  
  wake_up (task1, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
  wake_up (task2, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
}

DEBUG_FUNC (test_kthreads);
DEBUG_FUNC (kernel_thread_one);
DEBUG_FUNC (kernel_thread_two);

void 
main (void)
{
  disable_interrupts ();
  
  boot_console_init ();
  
  puts (KERNEL_BOOT_STRING);
  
  kctx_init ();
  
  hw_memory_init ();
  
  vm_init ();
  
  setup_system_consoles ();
  
  hw_interrupt_init ();
  
  irq_interface_init ();
  
  hw_early_irq_init ();
  
  init_kernel_threads ();
  
  scheduler_init ();

  early_timers_init ();

  test_kthreads ();
  
  enable_interrupts ();
  
  kernel_halt ();
}

DEBUG_FUNC (main);


