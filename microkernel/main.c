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

#include <misc/radix_tree.h>

#include <arch.h>
#include <kctx.h>

extern struct console *syscon;
char example_elf[] =
  "\x7f"  "ELF"  "\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x03\x00"
  "\x01\x00\x00\x00\xf8\x80\x04\x08"  "4"  "\x00\x00\x00\xe4\x01\x00\x00\x00\x00\x00"
  "\x00"  "4"  "\x00"  " "  "\x00\x05\x00"  "("  "\x00\x07\x00\x06\x00\x01\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x80\x04\x08\x00\x80\x04\x08"  "|"  "\x01\x00\x00"  "|"  "\x01"
  "\x00\x00\x05\x00\x00\x00\x00\x10\x00\x00\x01\x00\x00\x00\xf8\x0f\x00\x00\xf8"
  "\x9f\x04\x08\xf8\x9f\x04\x08\x00\x00\x00\x00\x08\x00\x00\x00\x06\x00\x00\x00"
  "\x00\x10\x00\x00\x04\x00\x00\x00\xd4\x00\x00\x00\xd4\x80\x04\x08\xd4\x80\x04"
  "\x08"  "$"  "\x00\x00\x00"  "$"  "\x00\x00\x00\x04\x00\x00\x00\x04\x00\x00\x00"  "Q"
  "\xe5"  "td"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x06\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x14\x00\x00\x00\x03\x00\x00\x00"  "G"
  "NU"  "\x00\x88\x8a"  "8"  "\x86"  "[aJ!b"  "\x9b\xae"  "r"  "\x1e\x06\xb3\x06\xe1"  "j"
  "\xe2\xa5"  "U"  "\x89\xe5"  "S"  "\x83\xec\x14\xc7"  "E"  "\xf4\x00\x00\x00\x00\xeb"
  "("  "\x8b"  "E"  "\xf4\x89\xc1\x89\xc3\xc1\xfb\x1f\xa1\xf8\x9f\x04\x08\x8b\x15\xfc"
  "\x9f\x04\x08\x01\xc8\x11\xda\xa3\xf8\x9f\x04\x08\x89\x15\xfc\x9f\x04\x08\x83"
  "E"  "\xf4\x01\x81"  "}"  "\xf4\x7f\x96\x98\x00"  "~"  "\xcf\xcc\x83\xc4\x14"  "[]"  "\xc3"
  "\x14\x00\x00\x00\x00\x00\x00\x00\x01"  "zR"  "\x00\x01"  "|"  "\x08\x01\x1b\x0c\x04"
  "\x04\x88\x01\x00\x00"  " "  "\x00\x00\x00\x1c\x00\x00\x00\x98\xff\xff\xff"  "H"  "\x00"
  "\x00\x00\x00"  "A"  "\x0e\x08\x85\x02"  "B"  "\x0d\x05"  "M"  "\x83\x03"  "v"  "\xc3"
  "A"  "\x0c\x04\x04\xc5\x00"  "GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"  "\x00\x00"
  ".shstrtab"  "\x00"  ".note.gnu.build-id"  "\x00"  ".text"  "\x00"  ".eh_frame"  "\x00"
  ".bss"  "\x00"  ".comment"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0b\x00\x00\x00\x07\x00\x00\x00"
  "\x02\x00\x00\x00\xd4\x80\x04\x08\xd4\x00\x00\x00"  "$"  "\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x1e\x00\x00\x00\x01\x00"
  "\x00\x00\x06\x00\x00\x00\xf8\x80\x04\x08\xf8\x00\x00\x00"  "H"  "\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"  "$"  "\x00\x00\x00"
  "\x01\x00\x00\x00\x02\x00\x00\x00"  "@"  "\x81\x04\x08"  "@"  "\x01\x00\x00"  "<"  "\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"  "."
  "\x00\x00\x00\x08\x00\x00\x00\x03\x00\x00\x00\xf8\x9f\x04\x08\xf8\x0f\x00\x00"
  "\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00"
  "\x00"  "3"  "\x00\x00\x00\x01\x00\x00\x00"  "0"  "\x00\x00\x00\x00\x00\x00\x00"  "|"
  "\x01\x00\x00"  "*"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
  "\x01\x00\x00\x00\x01\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\xa6\x01\x00\x00"  "<"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
  "\x00\x00\x00\x00\x00\x00";

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
test_radix_entry (struct radix_tree_node *node, radixkey_t key)
{
  void **slot;

  if ((slot = radix_tree_lookup_slot (node, key)) == NULL)
    printk ("  Key %p not found :(\n", (uint32_t) key);
  else
    printk ("  Key %p: %p\n", (uint32_t) key, *slot);
}

void
test_kthreads (void)
{
  struct task *task1, *task2, *task3;

  int i = 0;

#if 0
  void *p;
    
  while ((p = page_alloc (16)) != NULL)
  {
    printk ("Alloc'd: %p (%d)\n", p, ++i);
    page_free (p, 8);
  }

  printk ("Memory filled with a 8/8 pattern\n");

  while ((p = page_alloc (9)) != NULL)
  {
    printk ("Alloc'd: %p (%d)\n", p, ++i);
    page_free (p, 8);
  }

  printk ("Done\n");
  
  kernel_halt ();
#endif
  
  if ((task3 = sysproc_load (example_elf, sizeof (example_elf))) == KERNEL_INVALID_POINTER)
    FAIL ("cannot open elf??\n");

  vm_space_debug (task3->ts_vm_space);

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
  wake_up (task3, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
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


