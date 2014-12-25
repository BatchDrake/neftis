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

#include <dev/serial.h>

#include <console/console.h>

#include <irq/irq.h>
#include <irq/timer.h>

#include <mm/kmalloc.h>
#include <mm/slab.h>
#include <mm/salloc.h>
#include <mm/vm.h>
#include <mm/vremap.h>

#include <task/task.h>
#include <task/sched.h>
#include <task/loader.h>
#include <task/msg.h>

#include <lock/event.h>
#include <lock/mutex.h>

#include <misc/radix_tree.h>
#include <misc/errno.h>

#include <arch.h>
#include <kctx.h>

extern struct console *syscon;

void
test_kthreads (void)
{
  schedule ();

  resume ();
}

DEBUG_FUNC (test_kthreads);

static char banner[] =
  "                                                                        \n"
  "       db                                                88  88         \n"
  "      d88b        ,d                                     \"\"  88         \n"
  "     d8'`8b       88                                         88         \n"
  "    d8'  `8b    MM88MMM  ,adPPYba,   88,dPYba,,adPYba,   88  88   ,d8   \n"
  "   d8YaaaaY8b     88    a8\"     \"8a  88P'   \"88\"    \"8a  88  88 ,a8\"    \n"
  "  d8\"\"\"\"\"\"\"\"8b    88    8b       d8  88      88      88  88  8888[      \n"
  " d8'        `8b   88,   \"8a,   ,a8\"  88      88      88  88  88`\"Yba,   \n"
"d8'          `8b  \"Y888  `\"YbbdP\"'   88      88      88  88  88   `Y8a  \n\n";

void
kernel_banner (void)
{
  puts (banner);
}

void 
main (void)
{
  disable_interrupts ();

  serial_init ();
  
  boot_console_init ();

  puts (KERNEL_BOOT_STRING);

  kernel_banner ();
  
  kctx_init ();
  
  hw_memory_init ();
  
  vremap_init ();
  
  vm_init ();

  setup_system_consoles ();
  
  hw_interrupt_init ();
  
  irq_interface_init ();
  
  hw_early_irq_init ();

  loader_init ();
  
  init_kernel_threads ();
  
  scheduler_init ();

  early_timers_init ();

  init_msg_queues ();

  load_servers ();
  
  test_kthreads ();
  
  enable_interrupts ();
  
  kernel_halt ();
}

DEBUG_FUNC (main);
