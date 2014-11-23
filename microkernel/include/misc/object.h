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

#ifndef _MISC_OBJECT_H
#define _MISC_OBJECT_H

#include <types.h>
#include <misc/list.h>

#define KERNEL_INVALID_HANDLE        0xffffffff
#define KERNEL_OBJECT_INSTANCE_FIELD __instance
#define KERNEL_OBJECT struct kernel_object *KERNEL_OBJECT_INSTANCE_FIELD
#define OBJECT(ptr) ((ptr)->KERNEL_OBJECT_INSTANCE_FIELD)
#define OBJCAST(type, obj) ((type *) (obj->ptr))
#define REFCAST(type, ref) OBJCAST (type, ref->object)
#define REFOBJ(ref) (ref)->object

struct kernel_object;
struct kernel_object_list;
struct task;

/* Reference to a kernel object */
struct kernel_object_ref
{
  LINKED_LIST;

  struct task *owner;
  struct kernel_object *object;
  struct kernel_object_list *element;
};

/* List element of kernel object reference */
struct kernel_object_list
{
  LINKED_LIST;

  struct kernel_object_ref *ref;
};

struct kernel_class
{
  LINKED_LIST;
  
  char                 *name;      /* Descriptive name */
  struct kernel_object *instances; /* Instance list */
  unsigned long         count;     /* Instance counter */
  
  void * (*dup)   (void *); /* Duplicate with no refs */
  void   (*open)  (struct task *, void *); /* Open instance */
  void   (*close) (struct task *, void *); /* Close instance */
  void   (*dtor)  (void *); /* Destroy */
};

/* Add mutexes? */
struct kernel_object
{
  LINKED_LIST;
  uint32_t                   handle; /* System-wide handle */
  struct kernel_class       *class;  /* Object class */
  struct kernel_object_ref  *refs;   /* List of object refs */
  unsigned long              count;  /* Instance count */
  
  void *ptr; /* Pointer to the actual object */
};

typedef struct kernel_class      class_t;
typedef struct kernel_object     object_t;
typedef struct kernel_object_ref objref_t;

/* Low level operations (non locked) */
struct kernel_object *__kernel_object_create (struct kernel_class *, void *);
struct kernel_object_ref *__kernel_object_open (struct kernel_object *, struct task *);
struct kernel_object *__kernel_object_dup (struct kernel_object *);
struct kernel_object_ref *__kernel_object_instance (struct kernel_class *, void *, struct task *);
void __kernel_object_ref_close (struct kernel_object_ref *);

void kernel_class_register (struct kernel_class *);
struct kernel_class *kernel_class_lookup_by_name (const char *);
struct kernel_object *kernel_object_create (struct kernel_class *, void *);
struct kernel_object_ref *kernel_object_instance (struct kernel_class *, void *);
struct kernel_object_ref *kernel_object_instance_task (struct kernel_class *, void *, struct task *);

struct kernel_object_ref *kernel_object_open (struct kernel_object *);
struct kernel_object_ref *kernel_object_open_task (struct kernel_object *, struct task *);
struct kernel_object *kernel_object_dup (struct kernel_object *);
void kernel_object_ref_close (struct kernel_object_ref *);

struct kernel_object_ref *kernel_object_open_from_list (struct kernel_object_list **, struct kernel_object *);
struct kernel_object_ref *kernel_object_open_from_list_task (struct kernel_object_list **, struct kernel_object *, struct task *);
void kernel_object_ref_close_from_list (struct kernel_object_list **, struct kernel_object_ref *);
struct kernel_object_list *kernel_object_list_dup (struct kernel_object_list *, struct task *);

void kernel_object_list_destroy (struct kernel_object_list *);

void kernel_debug_all_classes (void);

#endif /* _MISC_OBJECT_H */
