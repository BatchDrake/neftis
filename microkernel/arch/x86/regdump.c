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

#include <asm/seg.h>
#include <asm/interrupt.h>
#include <asm/regs.h>

extern char bootstack[4 * PAGE_SIZE];
extern void _start;
extern void kernel_base;

#define DISTANCE(x, y) \
  ((busword_t) (x) < (busword_t) (y) ? \
  (busword_t) (y) - (busword_t) (x) : (busword_t) (x) - (busword_t) (y))

struct kernel_symbol *get_nearest_symbol (void *addr)
{
  struct kernel_symbol *this_sym, *nearest;
  extern struct kernel_symbol __start_debugsyms;
  extern struct kernel_symbol __stop_debugsyms;
  busword_t curr_dist;

  curr_dist = 0;
  
  nearest = NULL;
  
  if ((busword_t) addr < (busword_t) &kernel_base ||
      (busword_t) addr >= (busword_t) &__stop_debugsyms)
    return NULL;

  for (this_sym = &__start_debugsyms; 
       (busword_t) this_sym < (busword_t) &__stop_debugsyms; 
        this_sym++)
  {
    if ((busword_t) addr >= (busword_t) this_sym->addr)
    {
      if (nearest == NULL)
      {
        nearest = this_sym;
        curr_dist = DISTANCE (nearest->addr, addr);
      }
      else if (DISTANCE (this_sym->addr, addr) < curr_dist)
      {
        nearest = this_sym;
        curr_dist = DISTANCE (nearest->addr, addr);
      }
    }
  }

  return nearest;
}


void x86_stack_trace (struct x86_stack_frame *frame)
{
  busword_t eip, ebp;
  busword_t stack_top;
  busword_t stack_bottom;
  
  struct kernel_symbol *nearest;

  stack_top    = (busword_t) bootstack;
  stack_bottom = (busword_t) bootstack + 4 * PAGE_SIZE;

  ebp = frame->regs.ebp;

  printk ("call trace:\n");
  
  nearest = get_nearest_symbol ((void *) frame->priv.eip);
  
  if (nearest)
      printk ("  [<%w>] %s+0x%x\n", frame->priv.eip, 
         nearest->name, frame->priv.eip - 
        (busword_t) nearest->addr);
    else
      printk ("  [<%w>] ??\n", frame->priv.eip);
      
  while (ebp > stack_top && ebp <= stack_bottom)
  {
    /*
    if (!kernel_accesable (ebp))
    {
      kprintf ("stack_trace: punto %ebp al siguiente marco corrupto (%w)\n", ebp);
      break;
    }
    */
    
    
    eip = *((DWORD *) ebp + 1);
    
    if (ebp == *(DWORD *) ebp)
      break;
      
    ebp = *(DWORD *) ebp;
    nearest = get_nearest_symbol ((void *) eip);

    if (nearest)
      printk ("  [<%w>] %s+0x%x\n", eip, nearest->name, eip - 
        (busword_t) nearest->addr);
    else
      printk ("  [<%w>] ??\n", eip);
  }
}


void
x86_regdump (struct x86_stack_frame *frame)
{
  struct gdt_ptr gdtptr;
  struct idt_ptr idtptr;
  DWORD cr0, cr2, cr3, cr4;
  
  x86_stack_trace (frame);
  
  printk ("eax=%y ebx=%y ecx=%y edx=%y\n",
    frame->regs.eax, frame->regs.ebx, frame->regs.ecx, frame->regs.edx);
    
  printk ("esi=%y edi=%y ebp=%y esp=%y\n",
    frame->regs.esi, frame->regs.edi, frame->regs.ebp, frame->regs.esp);
  
  GET_REGISTER ("%cr0", cr0);
  GET_REGISTER ("%cr2", cr2);
  GET_REGISTER ("%cr3", cr3);
  GET_REGISTER ("%cr4", cr4);
  
  printk ("kernel mode: cr0=%y cr2=%y cr3=%y cr4=%y\n", cr0, cr2, cr3, cr4);
  printk ("prev mode:   cr0=%y cr3=%y\n", frame->cr0, frame->cr3);
  
  printk ("eip=%y efl=%y (%C)\n",
    frame->priv.eip, frame->priv.eflags, frame->priv.eflags);
  
  printk ("cs=%p ds=%p es=%p fs=%p gs=%p ss=%p\n",
    frame->segs.cs, frame->segs.ds, frame->segs.es, frame->segs.fs,
    frame->segs.gs, frame->segs.ss);
  
  x86_get_current_idt (&idtptr);
  x86_get_current_gdt (&gdtptr);
  
  printk ("gdtr=%y %y ", gdtptr.limit, gdtptr.base);
  printk ("idtr=%y %y\n", idtptr.limit, idtptr.base);
  
  
  
}

DEBUG_FUNC (x86_regdump);
DEBUG_FUNC (_start);

