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

#include <multiboot.h>

#include <asm/interrupt.h>
#include <asm/8259-pic.h>
#include <asm/irq.h>
#include <asm/seg.h>
#include <asm/pagedir.h>
#include <asm/regs.h>
#include <asm/ports.h>
#include <asm/timer.h>

#include <mm/regions.h>
#include <mm/vm.h>

#include <misc/hook.h>

#include <vga/crtc.h>

#include <console/console.h>

#include <arch.h>
#include <kctx.h>

static int  got_interrupts_working = 0;
extern void kernel_start;
extern void kernel_end;

extern struct console *syscon;

struct console boot_console;

static int
addr_in_range (busword_t addr, busword_t low, busword_t high)
{
  return addr >= low && addr <= high;
}

void
boot_console_init (void)
{
  console_setup (&boot_console);
  
  syscon = &boot_console;
  
  video_cursor_setup (0, 15);
  
}

void
hw_interrupt_init (void)
{
  x86_init_all_gates ();
  got_interrupts_working++;
}

void
hw_early_irq_init (void)
{
  x86_early_irq_init ();
}

void
enable_interrupts (void)
{
  __asm__ __volatile__ ("sti");
}

void
disable_interrupts (void)
{
  __asm__ __volatile__ ("cli");
}

void
hw_memory_init (void)
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
         
        if (mmap_info->base_addr_low == 0)
          mmap_info->base_addr_low += 4096; /* This page is not allocatable */
          
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
  
  gdt_init ();
}

int
vm_kernel_space_map_image (struct vm_space *space)
{
  struct vm_region *region;
  
  /* TODO: protect data */
  
  RETURN_ON_PTR_FAILURE (region = vm_region_new (VREGION_TYPE_KERNEL));
    
  region->vr_access = VREGION_ACCESS_READ  |
                      VREGION_ACCESS_WRITE |
                      VREGION_ACCESS_EXEC;
                      
  region->vr_virt_start = (busword_t) &kernel_start;
  region->vr_virt_end   = (busword_t) &kernel_end - 1;
  
  MANDATORY (SUCCESS (vm_space_add_region (space, region)));
    
  return KERNEL_SUCCESS_VALUE;
}

int
vm_user_space_map_image (struct vm_space *space)
{
  struct vm_region *region;
  
  /* TODO: protect data */
  
  if ((region = vm_region_new (VREGION_TYPE_KERNEL)) == NULL)
    return KERNEL_ERROR_VALUE;
    
  region->vr_access = 0;
                      
  region->vr_virt_start = (busword_t) &kernel_start;
  region->vr_virt_end   = (busword_t) &kernel_end - 1;
  
  MANDATORY (SUCCESS (vm_space_add_region (space, region)));
    
  return KERNEL_SUCCESS_VALUE;
}

int
vm_kernel_space_map_io (struct vm_space *space)
{
  struct multiboot_info *mbi;
  struct memory_map *mmap_info;
  struct vm_region *region;
  
  int i;
  
  mbi = multiboot_location ();
  
  if (mbi->flags & (1 << 6))
  {      
    for (i = 0; i < mbi->mmap_length / sizeof (memory_map_t); i++)
    {      
      mmap_info = (struct memory_map *) 
        (mbi->mmap_addr + i * sizeof (memory_map_t));

      if (mmap_info->type != 1)
      {
        RETURN_ON_PTR_FAILURE (
           region = vm_region_iomap (mmap_info->base_addr_low,
                                 mmap_info->base_addr_low,
                                 __UNITS (mmap_info->length_low, PAGE_SIZE))
        );
        
        region->vr_access = VREGION_ACCESS_READ | VREGION_ACCESS_WRITE;
        
        if (FAILED (vm_space_add_region (space, region)))
        {
          warning ("region @ %y overlaps to existing kernel map!\n",
            mmap_info->base_addr_low);
            
          continue;
        }
      }   
    }
  }
  else
  {
    debug ("where are the memory maps? :S\n");
    return KERNEL_ERROR_VALUE;
  }
  
  if (FAILED_PTR (
    region = vm_region_iomap (VIDEO_BASE,
                            VIDEO_BASE,
                            VIDEO_PAGES))
  )
    FAIL ("can't map video memory\n");
                            
  region->vr_access = VREGION_ACCESS_READ | VREGION_ACCESS_WRITE;
         
  MANDATORY (SUCCESS (vm_space_add_region (space, region)));
  
  return KERNEL_SUCCESS_VALUE;
}

INLINE DWORD
__vm_flags_to_x86_flags (BYTE flags)
{
  DWORD x86_flags = 0;
  
  if ((flags & VM_PAGE_PRESENT) || (flags & VM_PAGE_READABLE))
    x86_flags |= PAGE_FLAG_PRESENT;
    
  if (flags & VM_PAGE_WRITABLE)
    x86_flags |= PAGE_FLAG_WRITABLE;
    
  if (flags & VM_PAGE_KERNEL)
    x86_flags |= PAGE_FLAG_GLOBAL;
    
  /* VM_PAGE_EXECUTABLE <-- disable bit NX */
  
  if (flags & VM_PAGE_ACCESSED)
    x86_flags |= PAGE_FLAG_ACCESSED;
    
  if (flags & VM_PAGE_DIRTY)
    x86_flags |= PAGE_FLAG_DIRTY;
    
  if (flags & VM_PAGE_KERNEL)
    x86_flags |= PAGE_FLAG_GLOBAL;

  return x86_flags;
}

int
__vm_map_to (void *pagedir, busword_t virt, busword_t phys, busword_t pages,
  BYTE flags)
{ 
  return x86_pagedir_map_range (pagedir, virt, phys, pages, 
    __vm_flags_to_x86_flags (flags));
}

int
__vm_set_flags (void *pagedir, busword_t virt, busword_t pages, BYTE flags)
{
  return x86_pagedir_set_flags (pagedir, virt, pages, 
    __vm_flags_to_x86_flags (flags));
}

int
__vm_unset_flags (void *pagedir, busword_t virt, busword_t pages, BYTE flags)
{
  return x86_pagedir_unset_flags (pagedir, virt, pages, 
    __vm_flags_to_x86_flags (flags));
}

void *
__vm_alloc_page_table (void)
{
  void *page;
  
  page = page_alloc (1);
  
  if (page)
    memset (page, 0, PAGE_SIZE);
    
  return page;
}

void
__vm_free_page_table (void *pagedir)
{
  x86_free_pagedir (pagedir);
}

void
hw_vm_init (void)
{
  DWORD cr0, cr3;
  
  SET_REGISTER ("%cr3", current_kctx->kc_vm_space->vs_pagetable);
  
  GET_REGISTER ("%cr0", cr0);
  
  cr0 |= CR0_PAGING_ENABLED;
  
  SET_REGISTER ("%cr0", cr0);
  
  debug ("paging enabled correctly, pagedir: %p\n",
    current_kctx->kc_vm_space->vs_pagetable);
}

void
hw_set_timer_interrupt_freq (int freq)
{
  DWORD timer_count;
  
  timer_count = TIMER_BASE_FREQ / freq;
  
  outportb (TIMER_PORT_COMMAND, TIMER_MODE_3 |
                                TIMER_ACCESS_LOHI |
                                TIMER_CHANNEL (0));

  outportb (TIMER_PORT_CHANNEL (0), timer_count & 0xff);
  outportb (TIMER_PORT_CHANNEL (0), timer_count >> 8);
}

void 
hw_timer_enable (void)
{
  __irq_unmask (IRQ_TIMER);
}

void 
hw_timer_disable (void)
{
  __irq_mask (IRQ_TIMER);
}

int
hook_timer (int (*func) (int, void *, void *))
{
  extern struct hook_bucket *sys_irq_bucket;
  
  RETURN_ON_FAILURE (hook_register (sys_irq_bucket, IRQ_TIMER, func, NULL));
  
  return KERNEL_SUCCESS_VALUE;
}

void
bugcheck (void)
{
  if (!unlikely (got_interrupts_working))
  {
    error ("too early for a regdump\n");
    kernel_halt ();
  }
  
  RAISE_INTERRUPT (KERNEL_BUGCHECK_INTERRUPT);
}

void
debug_ascii_table (void)
{
  int i, j;
  extern struct console *syscon;
  
  for (j = 0; j < 16; j++)
  {
    for (i = 0; i < 16; i++)
    {
      console_putchar_raw (syscon, (j << 4) | i);
      printk (" %b ", (j << 4) | i);
    }
  }
}

DEBUG_FUNC (addr_in_range);
DEBUG_FUNC (boot_console_init);
DEBUG_FUNC (hw_interrupt_init);
DEBUG_FUNC (hw_early_irq_init);
DEBUG_FUNC (enable_interrupts);
DEBUG_FUNC (disable_interrupts);
DEBUG_FUNC (hw_memory_init);
DEBUG_FUNC (vm_kernel_space_map_image);
DEBUG_FUNC (vm_user_space_map_image);
DEBUG_FUNC (vm_kernel_space_map_io);
DEBUG_FUNC (__vm_flags_to_x86_flags);
DEBUG_FUNC (__vm_map_to);
DEBUG_FUNC (__vm_set_flags);
DEBUG_FUNC (__vm_unset_flags);
DEBUG_FUNC (__vm_alloc_page_table);
DEBUG_FUNC (__vm_free_page_table);
DEBUG_FUNC (hw_vm_init);
DEBUG_FUNC (hw_set_timer_interrupt_freq);
DEBUG_FUNC (hw_timer_enable);
DEBUG_FUNC (hw_timer_disable);
DEBUG_FUNC (hook_timer);
DEBUG_FUNC (bugcheck);



