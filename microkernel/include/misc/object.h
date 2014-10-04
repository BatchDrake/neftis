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
#include <task/task.h>

#define KERNEL_INVALID_HANDLE        0xffffffff
#define KERNEL_OBJECT_INSTANCE_FIELD __instance
#define KERNEL_OBJECT struct kernel_object *KERNEL_OBJECT_INSTANCE_FIELD
#define OBJECT(ptr) ((ptr)->KERNEL_OBJECT_INSTANCE_FIELD)

struct kernel_object;

struct kernel_object_list
{
  LINKED_LIST;

  /* Each kernel_object_user must be created for each
     registered kernel object in the current task */
     
  struct kernel_object_user *user;
  
  struct kernel_object *object;
};

struct kernel_object_user
{
  LINKED_LIST;

  struct task *user;
};

struct kernel_class
{
  LINKED_LIST;
  
  char                 *name;      /* Descriptive name */
  struct kernel_object *instances; /* Instance list */
  
  void * (*dup)   (void *);                /* Duplicate with no users */
  void   (*open)  (struct task *, void *); /* Open instance */
  void   (*close) (struct task *, void *); /* Close instance */
  void   (*dtor)  (void *);                /* Destroy */
};

/* Add mutexes? */
struct kernel_object
{
  LINKED_LIST;
  uint32_t                   handle; /* System-wide handle */
  struct kernel_class       *class;  /* Object class */
  struct kernel_object_user *users;  /* List of object users */
  unsigned long              count;  /* Instance count */
  
  void *ptr; /* Pointer to the actual object */
};

typedef struct kernel_class  class_t;
typedef struct kernel_object object_t;

#endif /* _MISC_OBJECT_H */
