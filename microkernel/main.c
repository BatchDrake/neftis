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
#include <mm/slab.h>
#include <mm/vm.h>

#include <task/task.h>
#include <task/sched.h>
#include <task/loader.h>

#include <misc/radix_tree.h>

#include <arch.h>
#include <kctx.h>

extern struct console *syscon;

void
kernel_thread_test_slab (void)
{
  struct kmem_cache *cache;
  void *ptr;
  
  int i;
  
  if ((cache = kmem_cache_create ("my-cache", 16, NULL, NULL)) == NULL)
    FAIL ("Cannot allocate cache\n");

  ptr = kmem_cache_alloc (cache);

  printk ("Allocated %p from SLAB\n", ptr);
  printk ("Alloc, state: %d\n", cache->state);
  
  kmem_cache_free (cache, ptr);

  printk ("Freed, state: %d\n", cache->state);
  
  for (i = 0; kmem_cache_alloc (cache) != NULL; ++i);

  printk ("%d allocations, state: %d\n", i, cache->state);

  printk ("Grow cache...\n");
  
  if (kmem_cache_grow (cache) == -1)
    FAIL ("Cannot grow cache!\n");

  printk ("Allocating again\n");
  
  for (i = 0; kmem_cache_alloc (cache) != NULL; ++i);

  printk ("%d allocations, state: %d\n", i, cache->state);
  
  for (;;);
}

void
test_kthreads (void)
{
  struct task *task1;

  if ((task1 = kernel_task_new (kernel_thread_test_slab)) == NULL)
    FAIL ("Cannot allocate task!\n");
  
  wake_up (task1, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
}

DEBUG_FUNC (test_kthreads);
DEBUG_FUNC (kernel_thread_test_slab);

void
kernel_banner (void)
{
  puts ("    ___   __                  _ __  \n");
  puts ("   /   | / /_____  ____ ___  (_/ /__\n");
  puts ("  / /| |/ __/ __ \\/ __ `__ \\/ / //_/\n");
  puts (" / ___ / /_/ /_/ / / / / / / / ,<   \n");
  puts ("/_/  |_\\__/\\____/_/ /_/ /_/_/_/|_|   \n");
  puts ("\n");
  puts ("Based on Neftis microkernel\n");
}

void 
main (void)
{
  disable_interrupts ();
  
  boot_console_init ();
  
  puts (KERNEL_BOOT_STRING);

  kernel_banner ();
  
  kctx_init ();
  
  hw_memory_init ();
  
  vm_init ();
  
  setup_system_consoles ();
  
  hw_interrupt_init ();
  
  irq_interface_init ();
  
  hw_early_irq_init ();

  loader_init ();
  
  init_kernel_threads ();
  
  scheduler_init ();

  early_timers_init ();

  test_kthreads ();
  
  enable_interrupts ();
  
  kernel_halt ();
}

DEBUG_FUNC (main);


