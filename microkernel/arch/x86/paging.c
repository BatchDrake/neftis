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

#include <string.h>

#include <mm/vm.h>
#include <mm/regions.h>

#include <asm/pagedir.h>

#include <arch.h>
#include <util.h>

INLINE int
x86_page_table_present (DWORD *pagedir, DWORD addr)
{
  return !! (pagedir[PAGE_TABLE (addr)] & PAGE_FLAG_PRESENT);
}

INLINE DWORD *
x86_get_page_table (DWORD *pagedir, DWORD addr)
{
  return (DWORD *) (pagedir[PAGE_TABLE (addr)] & PAGE_BITS);
}

DWORD *
x86_alloc_new_pagedir (void)
{
  DWORD *page;
  
  PTR_RETURN_ON_PTR_FAILURE (page = page_alloc (1));
  
  memset (page, 0, PAGE_SIZE);
  
  return page;
}

int
x86_prepare_page_table (DWORD *pagedir, DWORD addr)
{
  DWORD *tableptr;
  
  if (x86_page_table_present (pagedir, addr))
    return KERNEL_SUCCESS_VALUE;
    
  RETURN_ON_PTR_FAILURE (tableptr = page_alloc (1));
    
  memset (tableptr, 0, PAGE_SIZE);

  /* REMOVE PAGE_FLAG_USERLAND AS SOON AS POSSIBLE */
  pagedir[PAGE_TABLE (addr)] = (DWORD) tableptr | PAGE_TABLE_DFL_FLAGS | PAGE_FLAG_USERLAND;
  
  return KERNEL_SUCCESS_VALUE;
}

int
x86_pagedir_map_range (DWORD *pagedir, DWORD virt, DWORD phys, DWORD pages,
  DWORD flags)
{
  DWORD i;
  DWORD this_virt, this_phys;
  DWORD *pagetable;
  
  RETURN_ON_FAILURE (x86_prepare_page_table (pagedir, virt));
    
  pagetable = x86_get_page_table (pagedir, virt);
  
  for (i = 0; i < pages; i++)
  {
    this_virt = virt + (i << 12);
    this_phys = phys + (i << 12);
    
    if (PAGE_ENTRY (this_virt) == 0)
    {
      if (x86_prepare_page_table (pagedir, this_virt) == -1)
        return KERNEL_ERROR_VALUE;
        
      pagetable = x86_get_page_table (pagedir, this_virt);
    }

    /* REMOVE PAGE_FLAG_USERLAND AS SOON AS POSSIBLE */
    pagetable[PAGE_ENTRY (this_virt)] = this_phys | flags | PAGE_FLAG_USERLAND;
  }
  
  return KERNEL_SUCCESS_VALUE;
}

int
x86_pagedir_set_flags (DWORD *pagedir, DWORD virt, DWORD pages, DWORD flags)
{
  DWORD i;
  DWORD this_virt;
  DWORD *pagetable;
  
  ASSERT (x86_page_table_present (pagedir, virt));
    
  pagetable = x86_get_page_table (pagedir, virt);
  
  for (i = 0; i < pages; i++)
  {
    this_virt = virt + (i << 12);
    
    if (PAGE_ENTRY (this_virt) == 0)
    {
      ASSERT (x86_page_table_present (pagedir, this_virt));
        
      pagetable = x86_get_page_table (pagedir, this_virt);
    }
    
    pagetable[PAGE_ENTRY (this_virt)] &= PAGE_BITS;
    pagetable[PAGE_ENTRY (this_virt)] |= flags;
  }
  
  return KERNEL_SUCCESS_VALUE;
}

int
x86_pagedir_unset_flags (DWORD *pagedir, DWORD virt, DWORD pages, DWORD flags)
{
  DWORD i;
  DWORD this_virt;
  DWORD *pagetable;
  
  ASSERT (x86_page_table_present (pagedir, virt));
    
  pagetable = x86_get_page_table (pagedir, virt);
  
  for (i = 0; i < pages; i++)
  {
    this_virt = virt + (i << 12);
    
    if (PAGE_ENTRY (this_virt) == 0)
    {
      ASSERT (x86_page_table_present (pagedir, this_virt));
        
      pagetable = x86_get_page_table (pagedir, this_virt);
    }
    
    pagetable[PAGE_ENTRY (this_virt)] &= ~flags;
  }
  
  return KERNEL_SUCCESS_VALUE;
}

void
x86_debug_pagedir (DWORD *pagedir)
{
  DWORD i, j;
  DWORD in_range = 0;
  DWORD *pagetable;
  
  for (i = 0; i < 1024; i++)
  {
    if (!x86_page_table_present (pagedir, i << 22))
    {
      if (in_range)
      {
        printk ("-%y\n", (i << 22) - 1);
        in_range = 0;
      }
      
      continue;
    }
    
    pagetable = x86_get_page_table (pagedir, i << 22);
    
    for (j = 0; j < 1024; j++)
    {
      if (pagetable[j] & PAGE_FLAG_PRESENT)
      {
        if (!in_range)
        {
          printk ("%y", i << 22 | j << 12);
          in_range = 1;
        }
      }
      else if (in_range)
      {
        printk ("-%y\n", (i << 22 | j << 12) - 1);
        in_range = 0;
      }
    }
  }
  
  if (in_range)
  {
    printk ("-%y\n", (i << 22) - 1);
    in_range = 0;
  }
}

void
x86_free_pagedir (DWORD *pagedir)
{
  DWORD i, j;
  DWORD in_range = 0;
  DWORD *pagetable;
  
  for (i = 0; i < 1024; i++)
  {
    if (x86_page_table_present (pagedir, i << 22))
    {
      pagetable = x86_get_page_table (pagedir, i << 22);
      page_free (pagetable, 1);
    }
  }
  
  page_free (pagedir, 1);
}

void
x86_page_test (void)
{
  DWORD *pagedir;
  int result;
  
  pagedir = x86_alloc_new_pagedir ();
  
  result = x86_pagedir_map_range (pagedir, 0xb8000, 0xb8000, 4, 
    PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE);
  
  ASSERT (result != -1);
  
  result = x86_pagedir_map_range (pagedir, 0, 0, 100, 
    PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE);
  
  ASSERT (result != -1);
  
  result = x86_pagedir_map_range (pagedir, 0xc0000000, 0xc0000000, 
  100, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE);
  
  ASSERT (result != -1);
  
  x86_debug_pagedir (pagedir);
}

void
x86_paging_init (void)
{
}

