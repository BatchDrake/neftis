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
 
#include <mm/salloc.h>
#include <misc/hook.h>

#include <util.h>

struct hook_bucket *
hook_bucket_new (int hooks)
{
  struct hook_bucket *new;
  int i;
  
  if (hooks < 1)
  {
    debug ("hooks < 1? wtf\n");
    return NULL;
  }
  
  PTR_RETURN_ON_PTR_FAILURE (
    new = salloc (sizeof (struct hook_bucket) + 
                 sizeof (struct hook_func) * (hooks - 1))
  );
                 
    
  new->hb_hook_count = hooks;
  
  for (i = 0; i < hooks; i++)
    new->hb_hooks[i] = NULL;
    
  return new;
}

INLINE void
hook_add (struct hook_bucket *bucket, int code, struct hook_func *cb)
{
  ASSERT (bucket != NULL);
  ASSERT (cb != NULL);
  
  if (bucket->hb_hooks[code] != NULL)
    bucket->hb_hooks[code]->hf_prev = cb;
  
  cb->hf_next = bucket->hb_hooks[code];
  cb->hf_prev = NULL;
  
  bucket->hb_hooks[code] = cb;
}

int
hook_register (struct hook_bucket *bucket, int code,
               int (*func) (int, void *, void *),
               void *data)
{
  struct hook_func *cb;
  
  if (IN_BOUNDS (code, bucket->hb_hook_count))
  {
    debug ("code (%d) out of bounds!\n", code);
    return KERNEL_ERROR_VALUE;
  }
  
  
  RETURN_ON_PTR_FAILURE (cb = salloc (sizeof (struct hook_func)));
    
  cb->hf_func = func;
  cb->hf_data = data;
  
  hook_add (bucket, code, cb);
  
  return 0;
}

INLINE void
hook_func_free (struct hook_bucket *bucket, int code)
{
  struct hook_func *func, *next;
  
  if (IN_BOUNDS (code, bucket->hb_hook_count))
  {
    debug ("code (%d) out of bounds!\n", code);
    return;
  }
  
  
  func = bucket->hb_hooks[code];
  
  while (func != NULL)
  {
    next = func->hf_next;
    sfree (func);
    func = next;
  }
}

void
hook_bucket_free (struct hook_bucket *bucket)
{
  int i;
  
  for (i = 0; i < bucket->hb_hook_count; i++)
    hook_func_free (bucket, i);
    
  sfree (bucket);
}

int
trigger_hook (struct hook_bucket *bucket, int code, void *data)
{
  int acum;
  struct hook_func *func;
    
  if (IN_BOUNDS (code, bucket->hb_hook_count))
  {
    debug ("code (%d) out of bounds!\n", code);
    return KERNEL_ERROR_VALUE;
  }
  
  acum = 0;
  
  func = bucket->hb_hooks[code];
  
  while (func != NULL)
  {
    if ((func->hf_func) (code, func->hf_data, data) == HOOK_LOCK_CHAIN)
    {
      acum++; /* What if hf_func fails? */
      break;
    }
    
    acum++;
    func = func->hf_next;
  }
  
  return acum;
}

DEBUG_FUNC (hook_bucket_new);
DEBUG_FUNC (hook_add);
DEBUG_FUNC (hook_register);
DEBUG_FUNC (hook_func_free);
DEBUG_FUNC (hook_bucket_free);
DEBUG_FUNC (trigger_hook);

