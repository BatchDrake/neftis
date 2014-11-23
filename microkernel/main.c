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

int tid_1;
int tid_2;

struct task *t1, *t2;

static int flag = 0;

void
task_1 (void)
{
  int id, ret;
  volatile int i;
  int counter = 0;
  busword_t addr;
  
  for (i = 0; i < 1000000; ++i);
  
  printk ("[S] Creating a message...\n");
  
  if ((id = sys_msg_request (0)) < 0)
    printk ("[S] Cannot create message (%d)\n", id);
  else if ((int) (addr = sys_msg_map (id)) & 0xfff)
    printk ("[S] Cannot map: %d\n", addr);
  else
  {
    sys_msg_write_micro (id, "Hello world", sizeof ("Hello world"));
    
    while (counter < 7500)
    {
      if ((ret = sys_msg_send (id, tid_2)) < 0)
      {
	printk ("[S] Cannot send message (%d)\n", id);
	break;
      }

      if ((++counter % 100) == 0)
	printk ("[S] Sent %d messages so far\n", counter);
    }

    if (!flag)
    {
      ++flag;
      printk ("[S] I won!\n");
    }
    else
      printk ("[S] I lost :(\n");
  }
  

  for (;;);
}

void
task_2 (void)
{
  int id;
  int count = 0;
  busword_t addr;
  char buf[256];
  int size;
  
  wake_up (t1, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
  
  printk ("[R] Waiting for a message...\n");

  while (count < 7500)
  {
    if ((id = sys_msg_recv (MSG_RECV_BLOCK)) >= 0)
    {
      if ((++count % 100) == 0)
      {
	printk ("[R]  Received %d messages so far\n", count);
        if ((size = sys_msg_read_micro (id, buf, sizeof (buf))) < 0)
          printk ("[R] Read micro failed (%d)!\n", size);
        else
          printk ("[R]   Received string: \"%s\" (%d)\n", buf, size);
      }
      
      sys_msg_release (id);
    }
    else
    {
      error ("[R] Error recv: %d\n", id);
      kernel_halt ();
    }
  }

  if (!flag)
  {
    ++flag;
    printk ("[R] I won!\n");
  }
  else
    printk ("[R] I lost :(\n");
  
  for (;;);
}

void
test_kthreads (void)
{
  t2 = kernel_task_new (task_2);

  tid_2 = t2->ts_tid;

  t1 = kernel_task_new (task_1);

  wake_up (t2, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
  
  schedule ();
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
  
  test_kthreads ();
  
  enable_interrupts ();
  
  kernel_halt ();
}

DEBUG_FUNC (main);
