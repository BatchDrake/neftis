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

void
debug_kmap (void)
{
  void *page;
  char *a, *b;
  struct vm_region *region;
  
  debug ("Gonna alloc something...\n");

  if ((page = page_alloc (1)) == NULL)
    FAIL ("Cannot allocate one page :(\n");

  debug ("We have page at %p\n", page);
  
  if ((region = vm_region_shared (0xa0000000, (busword_t) page, 1)) == NULL)
    FAIL ("Cannot create region\n");

  region->vr_access |= VREGION_ACCESS_READ | VREGION_ACCESS_WRITE;
  
  vm_space_add_region (current_kctx->kc_vm_space, region);
    
  vm_region_invalidate (region);

  debug ("Done. Pagedir is %p\n", current_kctx->kc_vm_space->vs_pagetable);
  /* vm_space_debug (current_kctx->kc_vm_space); */
  
  a = (char *) 0xa0000000;
  b = (char *) page;

  strcpy (a, "Hola mundo");

  debug ("@ %p: \"%s\"\n", a, a);
  debug ("@ %p: \"%s\"\n", b, b);
}

DEBUG_FUNC (debug_kmap);

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
  
  debug_kmap ();
  
  scheduler_init ();

  early_timers_init ();
  
  enable_interrupts ();
  
  kernel_halt ();
}

DEBUG_FUNC (main);


