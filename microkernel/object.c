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
#include <misc/list.h>

#include <misc/object.h>

#include <mm/slab.h>
#include <mm/salloc.h>

#include <kctx.h>
#include <util.h>

struct kernel_class *kernel_class_list;

void
kernel_class_register (struct kernel_class *class)
{
  list_insert_head ((void **) &kernel_class_list, class);
}

struct kernel_class *
kernel_class_lookup_by_name (const char *name)
{
  struct kernel_class *this;

  FOR_EACH (this, kernel_class_list)
    if (strcmp (this->name, name) == 0)
      return this;
	
  return NULL;
}

static struct kernel_object_ref *
__kernel_object_open (struct kernel_object *object, struct task *who)
{
  struct kernel_object_ref *ref;
  
  ASSERT (who);

  CONSTRUCT_STRUCT (kernel_object_ref, ref);

  ref->owner = who;
  ref->object = object;

  ++object->count;

  if (object->class->open != NULL)
    (object->class->open) (who, object->ptr);

  list_insert_head ((void **) &object->refs, ref);
  
  return ref;
}

static struct kernel_object *
__kernel_object_create (struct kernel_class *class, void *ptr)
{
  struct kernel_object *new;

  CONSTRUCT_STRUCT (kernel_object, new);
  
  new->handle = KERNEL_INVALID_HANDLE;
  new->class  = class;
  new->count  = 0;
  
  new->ptr    = ptr;

  list_insert_head ((void **) &class->instances, new);
  ++class->count;
  
  return new;
}

struct kernel_object *
kernel_object_create (struct kernel_class *class, void *ptr)
{
  struct kernel_object *new;
  
  DECLARE_CRITICAL_SECTION (create);

  TASK_ATOMIC_ENTER (create);

  new = __kernel_object_create (class, ptr);

  TASK_ATOMIC_LEAVE (create);

  return new;
}

struct kernel_object_ref *
kernel_object_open (struct kernel_object *object)
{
  struct kernel_object_ref *ref;
  
  DECLARE_CRITICAL_SECTION (open);

  TASK_ATOMIC_ENTER (open);

  ref = __kernel_object_open (object, get_current_task ());

  TASK_ATOMIC_LEAVE (open);

  return ref;
}

struct kernel_object_ref *
kernel_object_open_task (struct kernel_object *object, struct task *task)
{
  struct kernel_object_ref *ref;
  
  DECLARE_CRITICAL_SECTION (open);

  TASK_ATOMIC_ENTER (open);

  ref = __kernel_object_open (object, task);

  TASK_ATOMIC_LEAVE (open);

  return ref;
}

static struct kernel_object *
__kernel_object_dup (struct kernel_object *object)
{
  void *copy;

  if (object->class->dup == NULL)
    return NULL;

  if ((copy = (object->class->dup) (object->ptr)) == NULL)
    return NULL;

  return __kernel_object_create (object->class, copy);
}

struct kernel_object *
kernel_object_dup (struct kernel_object *object)
{
  struct kernel_object *new;
  
  DECLARE_CRITICAL_SECTION (dup);

  TASK_ATOMIC_ENTER (dup);

  new = __kernel_object_dup (object);

  TASK_ATOMIC_LEAVE (dup);

  return new;
}

/* To be used internally */
static void
__kernel_object_destroy (struct kernel_object *object)
{
  ASSERT (object->class->count);
  
  list_remove_element ((void **) &object->class->instances, object);

  --object->class->count;
  
  if (object->class->dtor != NULL)
    (object->class->dtor) (object->ptr);

  sfree (object);
}

static struct kernel_object_ref *
__kernel_object_instance (struct kernel_class *class, void *ptr, struct task *owner)
{
  struct kernel_object *object;
  struct kernel_object_ref *ref;
  
  if ((object = __kernel_object_create (class, ptr)) == NULL)
    return NULL;

  if ((ref = __kernel_object_open (object, owner)) == NULL)
  {
    __kernel_object_destroy (object);

    return NULL;
  }

  return ref;
}

struct kernel_object_ref *
kernel_object_instance_task (struct kernel_class *class, void *ptr, struct task *owner)
{
  struct kernel_object_ref *ref;
  
  DECLARE_CRITICAL_SECTION (instance);

  TASK_ATOMIC_ENTER (instance);
  
  ref = __kernel_object_instance (class, ptr, owner);

  TASK_ATOMIC_LEAVE (instance);

  return ref;
}

struct kernel_object_ref *
kernel_object_instance (struct kernel_class *class, void *ptr)
{
  struct kernel_object_ref *ref;
  
  DECLARE_CRITICAL_SECTION (instance);

  TASK_ATOMIC_ENTER (instance);
  
  ref = __kernel_object_instance (class, ptr, get_current_task ());

  TASK_ATOMIC_LEAVE (instance);

  return ref;
}


static void
__kernel_object_ref_close (struct kernel_object_ref *ref)
{
  struct kernel_object *object = ref->object;

  ASSERT (object);
  ASSERT (object->count >= 0);

  if (object->count > 0)
  {
    ASSERT (ref);
    ASSERT (object->refs);
    
    if (object->class->close != NULL)
      (object->class->close) (ref->owner, object->ptr);
    
    if (--object->count == 0)
    {
      /* Ensure integrity */
      ASSERT (object->refs == ref);
      
      list_remove_element ((void **) &object->refs, ref);
    }
    else
      list_remove_element ((void **) &object->refs, ref);
    
    sfree (ref);
  }

  /* count == 0 => refs == NULL */
  ASSERT (object->count == 0 || object->refs != NULL);
  ASSERT (object->count != 0 || object->refs == NULL);
  
  /* Free object */
  if (object->count == 0)
    __kernel_object_destroy (object);
}

void
kernel_object_ref_close (struct kernel_object_ref *ref)
{
  DECLARE_CRITICAL_SECTION (close);

  TASK_ATOMIC_ENTER (close);

  __kernel_object_ref_close (ref);
  
  TASK_ATOMIC_LEAVE (close);
}

static struct kernel_object_list *
__kernel_object_list_element_new (struct kernel_object_ref *ref)
{
  struct kernel_object_list *new;

  CONSTRUCT_STRUCT (kernel_object_list, new);

  new->ref   = ref;
  ref->element = new; /* Update backpointer */
  
  return new;
}

static void
__kernel_object_list_element_destroy (struct kernel_object_list *element)
{
  /* Ref is automatically destroyed on close */
  __kernel_object_ref_close (element->ref);
  
  sfree (element);
}

static void
__kernel_object_list_destroy (struct kernel_object_list *head)
{
  struct kernel_object_list *copy, *this;
  
  this = head;
  
  while (this != NULL)
  {
    copy = LIST_NEXT (this);

    __kernel_object_list_element_destroy (this);
    
    this = copy;
  }
}

void
kernel_object_list_destroy (struct kernel_object_list *head)
{
  DECLARE_CRITICAL_SECTION (destroy);

  TASK_ATOMIC_ENTER (destroy);

  __kernel_object_list_destroy (head);

  TASK_ATOMIC_LEAVE (destroy);
}

static void
__kernel_object_list_remove_ref (struct kernel_object_list **head, struct kernel_object_ref *ref)
{
  ASSERT (ref->element);
  
  list_remove_element ((void **) head, ref->element);
}

void
kernel_object_list_remove_ref (struct kernel_object_list **head, struct kernel_object_ref *ref)
{
  DECLARE_CRITICAL_SECTION (remove);

  TASK_ATOMIC_ENTER (remove);

  __kernel_object_list_remove_ref (head, ref);
  
  TASK_ATOMIC_LEAVE (remove);
}

static int
__kernel_object_list_add_ref (struct kernel_object_list **head, struct kernel_object_ref *ref)
{
  struct kernel_object_list *new;
  
  if ((new = __kernel_object_list_element_new (ref)) == NULL)
    return -1;

  list_insert_head ((void **) head, new);

  return 0;
}

int
kernel_object_list_add_ref (struct kernel_object_list **head, struct kernel_object_ref *ref)
{
  int ret;

  DECLARE_CRITICAL_SECTION (add);

  TASK_ATOMIC_ENTER (add);

  ret = __kernel_object_list_add_ref (head, ref);
  
  TASK_ATOMIC_LEAVE (add);

  return ret;
}

static struct kernel_object_ref *
__kernel_object_open_from_list (struct kernel_object_list **head, struct kernel_object *object, struct task *task)
{
  struct kernel_object_ref *ref;

  if ((ref = __kernel_object_open (object, task)) == NULL)
    return NULL;

  if (__kernel_object_list_add_ref (head, ref) == -1)
  {
    __kernel_object_ref_close (ref);

    return NULL;
  }

  return ref;
}

void
__kernel_object_ref_close_from_list (struct kernel_object_list **head, struct kernel_object_ref *ref)
{
  __kernel_object_list_remove_ref (head, ref);

  __kernel_object_ref_close (ref);
}

struct kernel_object_ref *
kernel_object_open_from_list (struct kernel_object_list **head, struct kernel_object *object)
{
  struct kernel_object_ref *ref;
  
  DECLARE_CRITICAL_SECTION (open);

  TASK_ATOMIC_ENTER (open);

  ref = __kernel_object_open_from_list (head, object, get_current_task ());
  
  TASK_ATOMIC_LEAVE (open);

  return ref;
}

struct kernel_object_ref *
kernel_object_open_from_list_task (struct kernel_object_list **head, struct kernel_object *object, struct task *task)
{
  struct kernel_object_ref *ref;
  
  DECLARE_CRITICAL_SECTION (open);

  TASK_ATOMIC_ENTER (open);

  ref = __kernel_object_open_from_list (head, object, task);
  
  TASK_ATOMIC_LEAVE (open);

  return ref;
}

void
kernel_object_ref_close_from_list (struct kernel_object_list **head, struct kernel_object_ref *ref)
{
  DECLARE_CRITICAL_SECTION (close);

  TASK_ATOMIC_ENTER (close);

  __kernel_object_ref_close_from_list (head, ref);
  
  TASK_ATOMIC_LEAVE (close);
}

static struct kernel_object_list *
__kernel_object_list_dup (struct kernel_object_list *src_head, struct task *task)
{
  struct kernel_object_list *head = NULL, *this;
  struct kernel_object *dup;
  
  FOR_EACH (this, src_head)
  {
    if ((dup = __kernel_object_dup (this->ref->object)) == NULL)
      goto fail;

    if (__kernel_object_open_from_list (&head, dup, task) == NULL)
    {
      __kernel_object_destroy (dup);

      goto fail;
    }
  }

  return head;
  
fail:
  __kernel_object_list_destroy (head);
  
  return NULL;
}

struct kernel_object_list *
kernel_object_list_dup (struct kernel_object_list *src_head, struct task *task)
{
  struct kernel_object_list *head;
  
  DECLARE_CRITICAL_SECTION (dup);

  TASK_ATOMIC_ENTER (dup);

  head = __kernel_object_list_dup (src_head, task);
  
  TASK_ATOMIC_LEAVE (dup);

  return head;
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

void
kernel_debug_object (struct kernel_object *object)
{
  printk ("  Object of type \"%s\"\n", object->class->name);
  printk ("    Handle: %w\n", object->handle);
  printk ("    References: %d\n", object->count);
  printk ("    Private: %p\n\n", object->ptr);
}

void
kernel_debug_class (struct kernel_class *class)
{
  struct kernel_object *this;

  printk ("Class \"%s\" has %d instances\n", class->name, class->count);
  
  FOR_EACH (this, class->instances)
    kernel_debug_object (this);

  printk ("\n");
}

void
kernel_debug_all_classes (void)
{
  struct kernel_class *this;

  FOR_EACH (this, kernel_class_list)
    kernel_debug_class (this);
}

DEBUG_FUNC (kernel_class_register);
DEBUG_FUNC (kernel_class_lookup_by_name);
DEBUG_FUNC (__kernel_object_open);
DEBUG_FUNC (__kernel_object_create);
DEBUG_FUNC (kernel_object_create);
DEBUG_FUNC (kernel_object_open);
DEBUG_FUNC (kernel_object_open_task);
DEBUG_FUNC (__kernel_object_dup);
DEBUG_FUNC (kernel_object_dup);
DEBUG_FUNC (__kernel_object_destroy);
DEBUG_FUNC (__kernel_object_instance);
DEBUG_FUNC (kernel_object_instance_task);
DEBUG_FUNC (kernel_object_instance);
DEBUG_FUNC (__kernel_object_ref_close);
DEBUG_FUNC (kernel_object_ref_close);
DEBUG_FUNC (__kernel_object_list_element_new);
DEBUG_FUNC (__kernel_object_list_element_destroy);
DEBUG_FUNC (__kernel_object_list_destroy);
DEBUG_FUNC (kernel_object_list_destroy);
DEBUG_FUNC (__kernel_object_list_remove_ref);
DEBUG_FUNC (kernel_object_list_remove_ref);
DEBUG_FUNC (__kernel_object_list_add_ref);
DEBUG_FUNC (kernel_object_list_add_ref);
DEBUG_FUNC (__kernel_object_open_from_list);
DEBUG_FUNC (__kernel_object_ref_close_from_list);
DEBUG_FUNC (kernel_object_open_from_list);
DEBUG_FUNC (kernel_object_open_from_list_task);
DEBUG_FUNC (kernel_object_ref_close_from_list);
DEBUG_FUNC (__kernel_object_list_dup);
DEBUG_FUNC (kernel_object_list_dup);
DEBUG_FUNC (kernel_object_register);
DEBUG_FUNC (kernel_object_from_handle);
DEBUG_FUNC (kernel_object_unregister);
