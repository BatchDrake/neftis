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

#include <misc/errno.h>

SYSPROTO (syscall_krn_exit)
{
  DECLARE_CRITICAL_SECTION (exit);

  TASK_ATOMIC_ENTER (exit);
  
  (void) wake_up (get_current_task (), TASK_STATE_EXITED, 0);
  
  task_destroy (get_current_task ());

  schedule ();

  TASK_ATOMIC_ABORT ();

  return 0;
}

SYSPROTO (syscall_krn_debug_int)
{
  printk ("%d", args[0]);

  return 0;
}

SYSPROTO (syscall_krn_debug_pointer)
{
  printk ("%p", args[0]);

  return 0;
}

/* TODO: API for userland interaction (u_strlen, etc) */
int
__u_strlen (struct task *task, busword_t addr)
{
  char *p;
  int len = 0;
  struct vm_space *space = REFCAST (struct vm_space, task->ts_vm_space);
  
  while ((p = (char *) virt2phys_irq (space, addr)) != NULL && *p != '\0')
  {
    do
    {
      ++p;
      ++len;
    }
    while ((busword_t) p & PAGE_MASK && *p != '\0');

    if (*p == '\0')
      break;
  }

  if (p == NULL)
    return -1; /* Invalid pointer */
  
  return len;
}

SYSPROTO (syscall_krn_declare_service)
{
  busword_t nameaddr = args[0];
  struct task *task = get_current_task ();
  char *name = NULL;
  int namelen;
  int ret = 0;
  
  DECLARE_CRITICAL_SECTION (serv);

  CRITICAL_ENTER (serv);

  /* You're not supposed to change the service identity twice */
  if (task->ts_service != NULL)
  {
    ret = -EINVAL;

    goto done;
  }
  
  if ((namelen = __u_strlen (task, nameaddr)) == -1)
  {
    ret = -EFAULT;

    goto done;
  }

  if ((name = salloc_irq (namelen + 1)) == NULL)
  {
    ret = -ENOMEM;

    goto done;
  }

  /* If __u_strlen worked here, this shouldn't fail */
  (void) copy2phys (REFCAST (struct vm_space, task->ts_vm_space), name, nameaddr, namelen + 1);

  ret = __service_register (task, name, &task->ts_service);
  
done:  
  CRITICAL_LEAVE (serv);

  /* This can wait */
  if (name != NULL)
    sfree_irq (name);

  return ret;
}

SYSPROTO (syscall_krn_query_service)
{
  struct service *serv;
  struct task *task = get_current_task ();
  char *name = NULL;
  busword_t nameaddr = args[0];
  int namelen;
  int ret;
  
  DECLARE_CRITICAL_SECTION (serv);

  CRITICAL_ENTER (serv);

  if ((namelen = __u_strlen (task, nameaddr)) == -1)
  {
    ret = -EFAULT;

    goto done;
  }

  if ((name = salloc_irq (namelen + 1)) == NULL)
  {
    ret = -ENOMEM;

    goto done;
  }

  /* If __u_strlen worked here, this shouldn't fail */
  (void) copy2phys (REFCAST (struct vm_space, task->ts_vm_space), name, nameaddr, namelen + 1);

  if ((serv = __service_lookup_by_name (name)) == NULL)
    ret = -ESRCH;
  else
    ret = serv->se_task->ts_tid;

done:
  CRITICAL_LEAVE (serv);

  /* No hurries */
  if (name != NULL)
    sfree_irq (name);

  return ret;
}

SYSPROTO (syscall_krn_debug_string)
{
  char *p;
  busword_t addr = args[0];
  struct task *task = get_current_task ();
  int i;

  while ((p = (char *) virt2phys_irq (REFCAST (struct vm_space, task->ts_vm_space), addr)) != NULL && *p != '\0')
  {
    do
      putchar (*p++);
    while ((busword_t) p & PAGE_MASK && *p != '\0');

    if (*p == '\0')
      break;
    
    addr = PAGE_START (addr) + PAGE_SIZE;
  }

  return 0;
}

SYSPROTO (syscall_krn_debug_buf)
{
  char *p;
  busword_t addr = args[0];
  unsigned int size = args[1];
  
  struct task *task = get_current_task ();
  int i = 0;

  while (i < size && (p = (char *) virt2phys_irq (REFCAST (struct vm_space, task->ts_vm_space), addr)) != NULL)
  {
    do
      putchar (*p++);
    while (((busword_t) p & PAGE_MASK) && i++ < size);

    addr = PAGE_START (addr) + PAGE_SIZE;
  }

  return 0;
}

SYSPROTO (syscall_krn_set_tls)
{
  int ret;
  
  DECLARE_CRITICAL_SECTION (set_tls);

  CRITICAL_ENTER (set_tls);

  if ((ret = __task_set_tls (get_current_task (), args[0])) == 0)
    __task_tls_update (get_current_task ());

  CRITICAL_LEAVE (set_tls);

  return ret ? -EFAULT : 0;
}

SYSPROTO (syscall_krn_brk)
{
  struct task *task = get_current_task ();
  struct vm_region *dataseg;
  busword_t ret;
  busword_t pages;
  busword_t prog_brk;
  
  DECLARE_CRITICAL_SECTION (grow);

  CRITICAL_ENTER (grow);

  if ((dataseg = vm_space_find_region_by_role (REFCAST (struct vm_space, task->ts_vm_space), VREGION_ROLE_DATASEG)) == NULL)
  {
    error ("No data segment found\n");
    return -ENOMEM;
  }

  prog_brk = dataseg->vr_virt_end + 1;
  
  if (args[0] == 0) /* Query program break */
    ret = prog_brk;
  else if (args[0] < prog_brk) /* Shrink is not allowed */
    ret = -EINVAL;
  else
  {
    pages = __UNITS (args[0] - dataseg->vr_virt_start, PAGE_SIZE);
    
    if (vm_region_resize (dataseg, dataseg->vr_virt_start, pages) != -1)
      vm_update_region (REFCAST (struct vm_space, task->ts_vm_space), dataseg); /* Update changes */
    
    ret = dataseg->vr_virt_end + 1;
  }
  
  CRITICAL_LEAVE (grow);

  return ret;
}
