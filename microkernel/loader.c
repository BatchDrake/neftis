/*
 *    Implements the misc executable loader API
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

#include <types.h>
#include <mm/regions.h>
#include <mm/spalloc.h>

#include <task/loader.h>

#include <string.h>
#include <util.h>

static struct loader loader_list[LOADER_MAX];
static int           loader_count;

/* TODO: lock */
struct loader *
loader_register (const char *name, const char *desc)
{
  if (loader_count >= LOADER_MAX)
    return NULL;

  loader_list[loader_count].name = name;
  loader_list[loader_count].desc = desc;

  return loader_list + loader_count++;
}

/* TODO: add error status */
loader_handle *
loader_open_exec (struct vm_space *space, void *exec_start, uint32_t exec_size)
{
  int i;
  loader_handle *handle;
  void *opaque;
  
  for (i = 0; i < loader_count; ++i)
    if (loader_is_usable (loader_list + i))
      if ((opaque = (loader_list[i].open) (exec_start, exec_size)) != KERNEL_INVALID_POINTER)
      {
        if (PTR_UNLIKELY_TO_FAIL (handle = spalloc (sizeof (loader_handle))))
        {
          error ("Not enough memory to allocate loader handle!\n");

          (loader_list[i].close) (opaque);

          return KERNEL_INVALID_POINTER;
        }

        handle->loader       = loader_list + i;
        handle->target_space = space;
        handle->exec_base    = exec_start;
        handle->exec_size    = exec_size;
        handle->opaque       = opaque;
        
        return handle;
      }

  return KERNEL_INVALID_POINTER;
}

busword_t 
loader_get_exec_entry (loader_handle *handle)
{
  return (handle->loader->entry) (handle->opaque);
}

/* Walk exec gives information about type & flags of the computed segment */
int
loader_walk_exec (loader_handle *handle, int (*callback) (struct vm_space *, int, int, busword_t, busword_t, void *, busword_t))
{
  return (handle->loader->walkseg) (handle->opaque, handle->target_space, callback);
}

void
loader_close_exec (loader_handle *handle)
{
  (handle->loader->close) (handle->opaque);

  spfree (handle);
}

void elf_init (void);

void
loader_init (void)
{
  elf_init ();
}


DEBUG_FUNC (loader_register);
DEBUG_FUNC (loader_open_exec);
DEBUG_FUNC (loader_get_exec_entry);
DEBUG_FUNC (loader_walk_exec);
DEBUG_FUNC (loader_close_exec);
DEBUG_FUNC (loader_init);
