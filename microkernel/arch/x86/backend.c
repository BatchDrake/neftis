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
#include <backend.h>
#include <multiboot.h>

#include <mm/regions.h>
#include <console/console.h>

extern void kernel_end;
extern struct console *syscon;
struct console boot_console;

static int
addr_in_range (busword_t addr, busword_t low, busword_t high)
{
  return addr >= low && addr <= high;
}

void
boot_console_init ()
{
  console_setup (&boot_console);
  
  syscon = &boot_console;
}

void
hw_memory_init ()
{
  struct multiboot_info *mbi;
  struct memory_map *mmap_info;
  
  int i;
  
  mbi = multiboot_location ();
  
  if (mbi->flags & (1 << 6))
  {
    for (i = 0; i < mbi->mmap_length / sizeof (memory_map_t); i++)
    {
      mmap_info = (struct memory_map *) 
        (mbi->mmap_addr + i * sizeof (memory_map_t));

       if (mmap_info->type == 1)
       {
         if (addr_in_range ((busword_t) &kernel_end,
                            mmap_info->base_addr_low,
                            mmap_info->base_addr_low + 
                            mmap_info->length_low - 1))
           /* Oops. Let's exclude kernel from allocatable ranges. */
         {
           mmap_info->length_low   -= 
            (busword_t) &kernel_end - mmap_info->base_addr_low;
            
           mmap_info->base_addr_low = (busword_t) &kernel_end;
           
         }
         mm_register_region ((physptr_t)  mmap_info->base_addr_low,
                             (physptr_t) (mmap_info->base_addr_low +
                                          mmap_info->length_low - 1));
       }
      
    }
  }
  else
  {
    panic ("where are the memory maps? :S");
    kernel_halt ();
  }
}


