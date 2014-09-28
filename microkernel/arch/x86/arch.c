/*
 *    Main architecture-specific functions for x86 platform.
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
#include <mm/anon.h>

#include <misc/hook.h>

#include <vga/crtc.h>

#include <console/console.h>

#include <arch.h>
#include <kctx.h>

busword_t kernel_pagedir;

static int got_interrupts_working = 0;
extern int kernel_start;
extern int kernel_end;
extern int text_start;

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

extern DWORD __free_start;

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
           (busword_t) __free_start - mmap_info->base_addr_low;
            
          mmap_info->base_addr_low = __free_start;
           
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

extern char bootstack[4 * PAGE_SIZE];

busword_t
vm_get_prefered_stack_bottom (void)
{
  return (busword_t) &text_start;
}

int
vm_kernel_space_map_image (struct vm_space *space)
{
  struct vm_region *region;
  busword_t image_size;
  busword_t phys_end;
  busword_t stack_addr;
  busword_t stack_length;
  
  /* Register upper region kernel */

  image_size = __UNITS ((busword_t) &kernel_end - (busword_t) &kernel_start, PAGE_SIZE);

  RETURN_ON_PTR_FAILURE (region = vm_region_remap ((busword_t) &text_start, (busword_t) &kernel_start, image_size, VREGION_ACCESS_READ | VREGION_ACCESS_WRITE | VREGION_ACCESS_EXEC | VM_PAGE_KERNEL));
  
  MANDATORY (SUCCESS (vm_space_add_region (space, region)));

  /* Register stack address. THIS MUST DISAPPEAR */
  RETURN_ON_PTR_FAILURE (region = vm_region_physmap (PAGE_BITS & (DWORD) bootstack, 5, VREGION_ACCESS_READ | VREGION_ACCESS_WRITE | VM_PAGE_KERNEL));

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
    debug ("where are the memory maps? :S (mbi: %p, flags: %p)\n", mbi, mbi->flags);
    return KERNEL_ERROR_VALUE;
  }
  
  if (FAILED_PTR (
    region = vm_region_iomap (VIDEO_BASE,
                            VIDEO_BASE,
                            VIDEO_PAGES))
  )
    FAIL ("can't map video memory\n");
  
  
  MANDATORY (SUCCESS (vm_space_add_region (space, region)));
  
  return KERNEL_SUCCESS_VALUE;
}

INLINE DWORD
__vm_flags_to_x86_flags (DWORD flags)
{
  DWORD x86_flags = 0;

  if (flags & VM_PAGE_PRESENT)
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

  if (flags & VM_PAGE_USER)
    x86_flags |= PAGE_FLAG_USERLAND;

  return x86_flags;
}

int
__vm_flush_pages (busword_t virt, busword_t pages)
{
  int i;
  
  for (i = 0; i < pages; ++i)
    __asm__ __volatile__ ("invlpg (%0)" :: "r" (virt + (i << __PAGE_BITS)));
}

int
__vm_map_to (void *pagedir, busword_t virt, busword_t phys, busword_t pages,
  DWORD flags)
{
  return x86_pagedir_map_range (pagedir, virt, phys, pages, 
    __vm_flags_to_x86_flags (flags));
}

int
__vm_set_flags (void *pagedir, busword_t virt, busword_t pages, DWORD flags)
{
  return x86_pagedir_set_flags (pagedir, virt, pages, 
    __vm_flags_to_x86_flags (flags));
}

int
__vm_unset_flags (void *pagedir, busword_t virt, busword_t pages, DWORD flags)
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
__pause (void)
{
  __asm__ __volatile__ ("hlt");
}

void
__halt (void)
{
  __asm__ __volatile__ ("1:");
  __asm__ __volatile__ ("cli");
  __asm__ __volatile__ ("hlt");
  __asm__ __volatile__ ("jmp 1b");
}

void
hw_vm_init (void)
{
  DWORD cr0, cr3;

  kernel_pagedir = (busword_t) current_kctx->kc_vm_space->vs_pagetable;
  
  SET_REGISTER ("%cr3", kernel_pagedir);
  
  GET_REGISTER ("%cr0", cr0);
  
  cr0 |= CR0_PAGING_ENABLED;
  
  SET_REGISTER ("%cr0", cr0);

  debug ("paging enabled correctly, pagedir: %p\n", kernel_pagedir);

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
DEBUG_FUNC (vm_get_prefered_stack_bottom);
DEBUG_FUNC (vm_kernel_space_map_image);
DEBUG_FUNC (vm_kernel_space_map_io);
DEBUG_FUNC (__vm_flags_to_x86_flags);
DEBUG_FUNC (__vm_flush_pages);
DEBUG_FUNC (__vm_map_to);
DEBUG_FUNC (__vm_set_flags);
DEBUG_FUNC (__vm_unset_flags);
DEBUG_FUNC (__vm_alloc_page_table);
DEBUG_FUNC (__vm_free_page_table);
DEBUG_FUNC (__pause);
DEBUG_FUNC (__halt);
DEBUG_FUNC (hw_vm_init);
DEBUG_FUNC (hw_set_timer_interrupt_freq);
DEBUG_FUNC (hw_timer_enable);
DEBUG_FUNC (hw_timer_disable);
DEBUG_FUNC (hook_timer);
DEBUG_FUNC (bugcheck);
DEBUG_FUNC (debug_ascii_table);
