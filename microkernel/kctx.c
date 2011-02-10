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

#include <kctx.h>
#include <mm/regions.h>

struct kernel_context contexts[NR_MAXCPUS];
extern struct mm_region *mm_regions;

struct kernel_context *
get_kctx (int cpu)
{
  ASSERT (cpu >= 0 && cpu < NR_MAXCPUS);
  
  return &contexts[cpu];
}

int 
get_cpu (void)
{
  return 0;
}

void
kctx_init (void)
{
  /* TODO: Some CPU detection. */
  memset (contexts, 0, sizeof (struct kernel_context) * NR_MAXCPUS);

  contexts[0].kc_context = KERNEL_CONTEXT_BOOT_TIME;
}


struct mm_region *
kctx_get_numa_friendly (int order)
{
  /* TODO: improve interface, this sucks! */
  
  int i;
  struct mm_region *this;
  
  this = mm_regions;
  
  for (i = 0; ; i++)
  {
    if (i == order || this == NULL)
      return this;
      
    this = this->mr_next;
  }
  
  return NULL;
}

