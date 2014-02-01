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
#include <task/loader.h>

#include <arch.h>
#include <kctx.h>

extern struct console *syscon;

static char example_elf [] = {
 "\x7f"  "ELF"  "\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x03\x00"
  "\x01\x00\x00\x00\xd8\x80\x04\x08"  "4"  "\x00\x00\x00\x84\x01\x00\x00\x00\x00\x00"
  "\x00"  "4"  "\x00"  " "  "\x00\x04\x00"  "("  "\x00\x07\x00\x06\x00\x01\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x80\x04\x08\x00\x80\x04\x08"  "$"  "\x01\x00\x00"  "$"  "\x01"
  "\x00\x00\x05\x00\x00\x00\x00\x10\x00\x00\x01\x00\x00\x00"  "$"  "\x01\x00\x00"  "$"
  "\x91\x04\x08"  "$"  "\x91\x04\x08\x04\x00\x00\x00\x04\x00\x00\x00\x06\x00\x00\x00"
  "\x00\x10\x00\x00\x04\x00\x00\x00\xb4\x00\x00\x00\xb4\x80\x04\x08\xb4\x80\x04"
  "\x08"  "$"  "\x00\x00\x00"  "$"  "\x00\x00\x00\x04\x00\x00\x00\x04\x00\x00\x00"  "Q"
  "\xe5"  "td"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x06\x00\x00\x00\x10\x00\x00\x00\x04\x00\x00\x00\x14\x00\x00"
  "\x00\x03\x00\x00\x00"  "GNU"  "\x00\x07"  "0"  "\x92\xa2"  "V"  "\xc7"  ":c"  "\xaf\xd1"
  "W"  "\xaa\xdf"  "G"  "\x92\xdb"  "_B"  "\xaa\x07"  "U"  "\x89\xe5\xa1"  "$"  "\x91\x04"
  "\x08\x83\xc0\x09\xa3"  "$"  "\x91\x04\x08\xcc"  "]"  "\xc3\x00\x14\x00\x00\x00\x00"
  "\x00\x00\x00\x01"  "zR"  "\x00\x01"  "|"  "\x08\x01\x1b\x0c\x04\x04\x88\x01\x00\x00"
  "\x1c\x00\x00\x00\x1c\x00\x00\x00\xcc\xff\xff\xff\x13\x00\x00\x00\x00"  "A"  "\x0e"
  "\x08\x85\x02"  "B"  "\x0d\x05"  "O"  "\xc5\x0c\x04\x04\x00\x00\x07\x00\x00\x00"  "G"
  "CC: (Debian 4.8.2-14) 4.8.2"  "\x00\x00"  ".shstrtab"  "\x00"  ".note.gnu.build-"
  "id"  "\x00"  ".text"  "\x00"  ".eh_frame"  "\x00"  ".data"  "\x00"  ".comment"  "\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x0b\x00\x00\x00\x07\x00\x00\x00\x02\x00\x00\x00\xb4\x80\x04"
  "\x08\xb4\x00\x00\x00"  "$"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00"
  "\x00\x00\x00\x00\x00\x00\x1e\x00\x00\x00\x01\x00\x00\x00\x06\x00\x00\x00\xd8"
  "\x80\x04\x08\xd8\x00\x00\x00\x13\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x01\x00\x00\x00\x00\x00\x00\x00"  "$"  "\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00"
  "\x00\xec\x80\x04\x08\xec\x00\x00\x00"  "8"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"  "."  "\x00\x00\x00\x01\x00\x00\x00\x03"
  "\x00\x00\x00"  "$"  "\x91\x04\x08"  "$"  "\x01\x00\x00\x04\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"  "4"  "\x00\x00\x00\x01\x00"
  "\x00\x00"  "0"  "\x00\x00\x00\x00\x00\x00\x00"  "("  "\x01\x00\x00\x1d\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00"
  "\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"  "E"  "\x01\x00\x00"  "="  "\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
  };

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

static int
callback (struct vm_space *space, int type, int flags, busword_t virt, busword_t size, void *data, busword_t datasize)
{
  char sflags[4] = {0};

  sflags[0] = (flags & VREGION_ACCESS_READ)  ? 'r' : '-';
  sflags[1] = (flags & VREGION_ACCESS_WRITE) ? 'w' : '-';
  sflags[2] = (flags & VREGION_ACCESS_EXEC)  ? 'x' : '-';
  
  debug ("  ELF segment at @ %p [%s] <-- %p (%H)\n",
          virt, sflags, data, size);
}

void
test_kthreads (void)
{
  struct task *task1, *task2, *task3;
  loader_handle *handle;
  int i;


  task3 = system_process_new ();
  
  debug ("Performing ELF test...\n");
  
  if ((handle = loader_open_exec (NULL, example_elf, sizeof (example_elf))) == NULL)
    FAIL ("Cannot load sample ELF file\n");

  loader_walk_exec (handle, callback);

  loader_close_exec (handle);

  debug ("Test done\n");
  
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

  loader_init ();
  
  init_kernel_threads ();
  
  scheduler_init ();

  early_timers_init ();

  test_kthreads ();
  
  enable_interrupts ();
  
  kernel_halt ();
}

DEBUG_FUNC (main);


