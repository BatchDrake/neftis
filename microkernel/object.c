/*
 *    Implementation of the kernel object manager
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

#include <task/task.h>
#include <misc/object.h>

struct kernel_class *kernel_class_list;

struct kernel_class *
kernel_class_lookup_by_name (const char *name)
{
  return NULL;
}

struct kernel_object *
kernel_object_create (struct kernel_class *class, void *ptr)
{
  /* Create new object, put it in class object list */
  return NULL;
}

struct kernel_object *
kernel_object_open (struct kernel_object *object)
{
  /* Open existing object, increment instance count */
  return NULL;
}

struct kernel_object *
kernel_object_dup (struct kernel_object *object)
{
  /* Duplicate existing object */

  return NULL;
}

void
kernel_object_close (struct kernel_object *object)
{
  /* Decrement instances and destruct object if instance count drops to zero */
}

struct kernel_object_list *
kernel_object_list_dup (struct kernel_object_list *task_handle_list, struct task *task)
{
  /* Duplicate per-task object list, increment instance count of each object, and create proper kernel_object_users */
}

void
kernel_object_list_destroy (struct kernel_object_list *task_handle_list)
{
  /* Destroy per-task object list */
}

int
task_register_kernel_object (struct task *task, struct kernel_object *object)
{
  /* Register instance in task instance list (create kernel_object_list and link it) */
}

uint32_t
kernel_object_register (struct kernel_object *object)
{
  /* Give object a system-wide handle */
  return KERNEL_INVALID_HANDLE;
}

struct kernel_object *
kernel_object_from_handle (uint32_t handle)
{
  /* Get object from handle */
  return NULL;
}

int
kernel_object_unregister (struct kernel_object *object)
{
  /* Remove system-wide handle for object */
  return -1;
}
