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
 
#include <util.h>
#include <asm/seg.h>
#include <asm/upperhalf.h>
#include <layout.h>

struct gdt_entry gdt[GDT_MAX_SEGMENTS];

BOOT_SYMBOL (struct tss tss);

INLINE void
gdt_entry_setup (struct gdt_entry *dest,
                  DWORD base, DWORD page_limit, BYTE access)
{
  if (page_limit > 0xfffff)
  {
    error ("page_limit beyond 0xfffff? wtf\n");
    return;
  }
  
  dest->limit_low  = page_limit & 0xffff;
  dest->base_low   = base & 0xffff;
  dest->base_mid   = (base >> 16) & 0xff;
  dest->base_high  = (base >> 24) & 0xff;
  dest->limit_high = (page_limit >> 16) & 0xf;
  dest->access     = access;
  dest->flags      = GDT_GRANULARITY_PAGES | GDT_SELECTOR_32_BIT;
}

INLINE void
gdt_entry_setup_tss (struct gdt_entry *dest)
{
  uint32_t tss_base  = (uint32_t) &tss;
  uint32_t tss_limit = tss_base + sizeof (struct tss) - 1;

  /* Kernel stack is in kernel data segment */
  
  tss.ss0          = GDT_SEGMENT_KERNEL_DATA;
  
  dest->limit_low  = tss_limit         & 0xffff;
  dest->base_low   = tss_base          & 0xffff;
  dest->base_mid   = (tss_base  >> 16) & 0xff;
  dest->base_high  = (tss_base  >> 24) & 0xff;
  dest->limit_high = (tss_limit >> 16) & 0xf;
  
  dest->access     = GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_ACCESED | GDT_ACCESS_RING (3);
  dest->flags      = GDT_SELECTOR_32_BIT;
}

INLINE void
debug_access (int access)
{
  if (access & GDT_ACCESS_ACCESED)
    printk ("ACCESED ");
  if (access & GDT_ACCESS_READWRITE)
    printk ("READWRITE ");
  if (access & GDT_ACCESS_CONFORMING)
    printk ("CONF/DOWN ");
  if (access & GDT_ACCESS_EXECUTABLE)
    printk ("EXECUTABLE ");
  if (access & GDT_ACCESS_SEGMENT)
    printk ("SEGMENT ");
  else
    printk ("TSS");
    
  if (access & GDT_ACCESS_PRESENT)
    printk ("PRESENT RING(%d) ", (access >> 5) & 3);
  printk ("\n");
}

INLINE void
debug_gdt (struct gdt_ptr *ptr)
{
  struct gdt_entry *this_entry;
  int elements, i;
  
  elements = __UNITS (ptr->limit, sizeof (struct gdt_entry));
  
  printk ("%d elements in the original GDT, located at %p\n",
    elements, ptr->base);
    
  this_entry = ptr->base;
  
  for (i = 0; i < elements; i++)
  {
    printk ("%d. Base: %p, limit: %p, access: %p, flags %p\n", i, 
      (this_entry[i].base_high << 24) | (this_entry[i].base_mid << 16) |
      (this_entry[i].base_low),
      this_entry[i].limit_low | ((0xf & this_entry[i].limit_high) << 16),
      this_entry[i].access,
      this_entry[i].flags);
      
    debug_access (this_entry[i].access);
  }
}

/* This is a Linux-fashioned way to implement it */
void
x86_setup_tls (uint32_t base, uint32_t limit)
{
  /* Please note that USER_TLS_BASE is in the middle of a page */
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_USER_TLS), base, limit, 
    GDT_ACCESS_READWRITE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT |
    GDT_ACCESS_RING (3));
}

void
gdt_init (void)
{
  struct gdt_ptr ptr, ptr2;
  BYTE *gdt_bytes;
  int i;
  
  ptr.limit = sizeof (struct gdt_entry) * GDT_MAX_SEGMENTS - 1;
  ptr.base  = gdt;

  gdt_bytes = (BYTE *) gdt;
  
  for (i = 0; i <= ptr.limit; ++i)
    gdt_bytes[i] = 0;
  
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_KERNEL_CODE), 0, 0xfffff,
    GDT_ACCESS_EXECUTABLE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT);
    
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_KERNEL_DATA), 0, 0xfffff,
    GDT_ACCESS_READWRITE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT);
  
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_USER_CODE), 0, 0xfffff,
    GDT_ACCESS_EXECUTABLE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT |
    GDT_ACCESS_RING (3));
    
  gdt_entry_setup (GDT_ENTRY (GDT_SEGMENT_USER_DATA), 0, 0xfffff, 
    GDT_ACCESS_READWRITE | GDT_ACCESS_SEGMENT | GDT_ACCESS_PRESENT |
    GDT_ACCESS_RING (3));
  
  gdt_entry_setup_tss (GDT_ENTRY (GDT_SEGMENT_TSS));


  x86_flush_gdt (&ptr);
  
  x86_flush_tss ();
  
  x86_get_current_gdt (&ptr2);
}

void
x86_set_kernel_stack (uint32_t addr)
{
  tss.esp0 = addr;
}

DEBUG_FUNC (gdt_entry_setup);
DEBUG_FUNC (gdt_entry_setup_tss);
DEBUG_FUNC (debug_access);
DEBUG_FUNC (debug_gdt);
DEBUG_FUNC (gdt_init);
DEBUG_FUNC (x86_set_kernel_stack);
DEBUG_FUNC (x86_enter_user);
