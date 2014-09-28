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
#include <lock/event.h>
#include <lock/mutex.h>

#include <misc/radix_tree.h>

#include <arch.h>
#include <kctx.h>

extern struct console *syscon;

static char bin2c_test_data [] = {
  "\x7f"  "ELF"  "\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x03\x00"
  "\x01\x00\x00\x00\xb8\x80\x04\x08"  "4"  "\x00\x00\x00"  "T"  "\x01\x00\x00\x00\x00"
  "\x00\x00"  "4"  "\x00"  " "  "\x00\x03\x00"  "("  "\x00\x06\x00\x05\x00\x01\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x80\x04\x08\x00\x80\x04\x08\x00\x01\x00\x00\x00\x01"
  "\x00\x00\x05\x00\x00\x00\x00\x10\x00\x00\x04\x00\x00\x00\x94\x00\x00\x00\x94"
  "\x80\x04\x08\x94\x80\x04\x08"  "$"  "\x00\x00\x00"  "$"  "\x00\x00\x00\x04\x00\x00"
  "\x00\x04\x00\x00\x00"  "Q"  "\xe5"  "td"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x10\x00\x00\x00\x04"
  "\x00\x00\x00\x14\x00\x00\x00\x03\x00\x00\x00"  "GNU"  "\x00\xd4\xd3\x1a\xc4\x10"
  "\x8c\x1d\x98"  "$@"  "\xdc"  "Yr"  "\xcd\xc7\x09\xbd"  ")"  "\x03"  "@U"  "\x89\xe5\xb8"
  "\x01\x00\x00\x00\xcd\xa0\xcd\xff"  "]"  "\xc3\x00\x00\x14\x00\x00\x00\x00\x00\x00"
  "\x00\x01"  "zR"  "\x00\x01"  "|"  "\x08\x01\x1b\x0c\x04\x04\x88\x01\x00\x00\x1c\x00"
  "\x00\x00\x1c\x00\x00\x00\xd0\xff\xff\xff\x0e\x00\x00\x00\x00"  "A"  "\x0e\x08\x85"
  "\x02"  "B"  "\x0d\x05"  "J"  "\xc5\x0c\x04\x04\x00\x00"  "GCC: (Debian 4.8.2-14) "
  "4.8.2"  "\x00\x00"  ".shstrtab"  "\x00"  ".note.gnu.build-id"  "\x00"  ".text"  "\x00"
  ".eh_frame"  "\x00"  ".comment"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0b\x00\x00\x00\x07\x00\x00\x00\x02"
  "\x00\x00\x00\x94\x80\x04\x08\x94\x00\x00\x00"  "$"  "\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x1e\x00\x00\x00\x01\x00\x00"
  "\x00\x06\x00\x00\x00\xb8\x80\x04\x08\xb8\x00\x00\x00\x0e\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"  "$"  "\x00\x00\x00\x01"
  "\x00\x00\x00\x02\x00\x00\x00\xc8\x80\x04\x08\xc8\x00\x00\x00"  "8"  "\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"  "."  "\x00\x00"
  "\x00\x01\x00\x00\x00"  "0"  "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x1d\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01"
  "\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1d\x01\x00\x00"
  "7"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
  "\x00"};
static const long bin2c_test_size = 580;

void
test_kthreads (void)
{
  struct task *tasks[4];

  int i;

  for (i = 0; i < 4; ++i)
  {
    if ((tasks[i] = user_task_new_from_exec (bin2c_test_data, bin2c_test_size)) == NULL)
      FAIL ("Cannot allocate task!\n");
    
    wake_up (tasks[i], TASK_STATE_RUNNING | WAKEUP_DELAYED, 0);
  }

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

  test_kthreads ();
  
  enable_interrupts ();
  
  kernel_halt ();
}

DEBUG_FUNC (main);
