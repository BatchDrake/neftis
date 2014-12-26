/*
 *    System call interface to IPC
 *    Copyright (C) 2014  Gonzalo J. Carracedo
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
#include <task/syscall.h>
#include <task/msg.h>
#include <kctx.h>

SYSPROTO (syscall_krn_exit)
{
  DECLARE_CRITICAL_SECTION (exit);

  TASK_ATOMIC_ENTER (exit);
  
  (void) wake_up (get_current_task (), TASK_STATE_EXITED, 0);
  
  task_destroy (get_current_task ());

  schedule ();

  TASK_ATOMIC_ABORT ();
}

SYSPROTO (syscall_krn_debug_int)
{
  printk ("%d", args[0]);
}

SYSPROTO (syscall_krn_debug_pointer)
{
  printk ("%p", args[0]);
}

SYSPROTO (syscall_krn_debug_string)
{
  char *p;
  busword_t addr = args[0];
  struct task *task = get_current_task ();
  int i;

  while ((p = (char *) virt2phys (REFCAST (struct vm_space, task->ts_vm_space), addr)) != NULL && *p != '\0')
  {
    do
      putchar (*p++);
    while ((busword_t) p & PAGE_MASK && *p != '\0');

    if (*p == '\0')
      break;
    
    addr = PAGE_START (addr) + PAGE_SIZE;
  }
}

SYSPROTO (syscall_krn_brk)
{
  struct task *task = get_current_task ();
  struct vm_region *dataseg;
  busword_t ret;
  
  DECLARE_CRITICAL_SECTION (grow);

  CRITICAL_ENTER (grow);

  if ((dataseg = vm_space_find_region_by_role (REFCAST (struct vm_space, task->ts_vm_space), VREGION_ROLE_DATASEG)) == NULL)
  {
    error ("No data segment found\n");
    ret = -1;
  }
  else
    ret = dataseg->vr_virt_end;
  
  CRITICAL_LEAVE (grow);

  return ret;
}
