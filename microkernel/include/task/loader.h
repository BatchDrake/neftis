/*
 *    Prototypes and declarations for the misc executable loader API
 *    Copyright (C) 2014 Gonzalo J. Carracedo <BatchDrake@gmail.com>
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

#ifndef _TASK_LOADER_H
#define _TASK_LOADER_H

#include <types.h>
#include <mm/vm.h>

#define LOADER_MAX 16

struct loader
{
  const char *name;
  const char *desc;

  void *        (*open)    (void *, uint32_t);
  busword_t     (*entry)   (void *);
  int           (*walkseg) (void *, struct vm_space *, int (*) (struct vm_space *, int, int, busword_t, busword_t, void *, busword_t));
  void          (*close)   (void *);
};

typedef struct
{
  struct loader   *loader;
  struct vm_space *target_space;
  void            *exec_base;
  uint32_t         exec_size;
  void            *opaque;
}
loader_handle;

INLINE BOOL
loader_is_usable (struct loader *loader)
{
  return loader->open    != NULL &&
         loader->entry   != NULL &&
         loader->walkseg != NULL &&
         loader->close   != NULL;
}

struct loader *loader_register (const char *, const char *);
loader_handle *loader_open_exec (struct vm_space *, void *, uint32_t);
busword_t loader_get_exec_entry (loader_handle *);
int loader_walk_exec (loader_handle *, int (*) (struct vm_space *, int, int, busword_t, busword_t, void *, busword_t));
void loader_close_exec (loader_handle *);
void loader_init (void);

#endif /* _TASK_LOADER_H */
