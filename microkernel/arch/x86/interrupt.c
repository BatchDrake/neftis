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
#include <string.h>

#include <misc/hook.h>
#include <irq/irq.h>
 
#include <asm/interrupt.h>
#include <asm/regs.h>
#include <asm/seg.h>
#include <asm/io.h>

#include <kctx.h>

#include <mm/vm.h>

/* Be careful: IDT is paged, so you will need it to be visible at the time
   is needed in a privileged context. */
   
struct idt_entry idt_entries[256];
struct idt_ptr   idt_ptr;

/* These ones are defined in assembly */

extern void isr0  (void);
extern void isr1  (void);
extern void isr2  (void);
extern void isr3  (void);
extern void isr4  (void);
extern void isr5  (void);
extern void isr6  (void);
extern void isr7  (void);
extern void isr8  (void);
extern void isr9  (void);
extern void isr10 (void);
extern void isr11 (void);
extern void isr12 (void);
extern void isr13 (void);
extern void isr14 (void);
extern void isr15 (void);
extern void isr16 (void);
extern void isr17 (void);
extern void isr18 (void);
extern void isr19 (void);
extern void isr20 (void);
extern void isr21 (void);
extern void isr22 (void);
extern void isr23 (void);
extern void isr24 (void);
extern void isr25 (void);
extern void isr26 (void);
extern void isr27 (void);
extern void isr28 (void);
extern void isr29 (void);
extern void isr30 (void);
extern void isr31 (void);

extern void isr32 (void);
extern void isr33 (void);
extern void isr34 (void);
extern void isr35 (void);
extern void isr36 (void);
extern void isr37 (void);
extern void isr38 (void);
extern void isr39 (void);
extern void isr40 (void);
extern void isr41 (void);
extern void isr42 (void);
extern void isr43 (void);
extern void isr44 (void);
extern void isr45 (void);
extern void isr46 (void);
extern void isr47 (void); 

extern void isr48 (void); 
extern void isr49 (void); 
extern void isr50 (void); 
extern void isr51 (void); 
extern void isr52 (void); 
extern void isr53 (void); 
extern void isr54 (void); 
extern void isr55 (void); 

extern void isr255 (void); 

/* x86_enable_interrupts: Activa las interrupciones (SeT Interrupt bit) */
void
x86_enable_interrupts (void)
{
  __asm__ __volatile__ ("sti");
}

/* x86_disable_interrupts: Desactiva las interrupciones (CLear Interrupt bit) */
void
x86_disable_interrupts (void)
{
  __asm__ __volatile__ ("cli");
}

/* x86_io_wait: Espera a que una operaciÃ³n de entrada / salida se complete. */
void
x86_io_wait (void)
{
  __asm__ __volatile__ ("jmp 1f");
  __asm__ __volatile__ ("1:");
  __asm__ __volatile__ ("jmp 1f");
  __asm__ __volatile__ ("1:");
}

/*
void
x86_idt_flush (struct idt_ptr *ptr)
{
  __asm__ __volatile__ ("lidt %0" :: "g" (ptr));
}
*/
void
x86_idt_set_gate  (BYTE num, physptr_t base, WORD sel, BYTE flags)
{
  idt_entries[num].base_lo = (DWORD) base & 0xffff;
  idt_entries[num].base_hi = ((DWORD) base >> 16) & 0xffff;

  idt_entries[num].sel     = sel;
  idt_entries[num].always0 = 0;
  idt_entries[num].flags   = flags;
}

void
x86_init_all_gates (void)
{
  idt_ptr.limit = sizeof (struct idt_entry) * 256 - 1;
  idt_ptr.base  = idt_entries;

  memset (&idt_entries, 0, sizeof (struct idt_entry) * 256);

  x86_idt_set_gate  ( 0, isr0 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  ( 1, isr1 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  ( 2, isr2 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  ( 3, isr3 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  ( 4, isr4 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  ( 5, isr5 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  ( 6, isr6 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  ( 7, isr7 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  ( 8, isr8 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  ( 9, isr9 , GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (10, isr10, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (11, isr11, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (12, isr12, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (13, isr13, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (14, isr14, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (15, isr15, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (16, isr16, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (17, isr17, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (18, isr18, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (19, isr19, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (20, isr20, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (21, isr21, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (22, isr22, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (23, isr23, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (24, isr24, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (25, isr25, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (26, isr26, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (27, isr27, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (28, isr28, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (29, isr29, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (30, isr30, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (31, isr31, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);



  /* SOY RETRASADO MENTAL */
  x86_idt_set_gate  (32, isr32, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (33, isr33, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (34, isr34, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (35, isr35, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (36, isr36, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (37, isr37, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (38, isr38, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (39, isr39, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (40, isr40, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (41, isr41, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (42, isr42, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (43, isr43, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (44, isr44, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (45, isr45, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (46, isr46, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (47, isr47, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  /* IOAPIC extra IRQs */
  
  x86_idt_set_gate  (48, isr48, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (49, isr49, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (50, isr50, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (51, isr51, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (52, isr52, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (53, isr53, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (54, isr54, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);

  x86_idt_set_gate  (55, isr55, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);
  
  x86_idt_set_gate  (255, isr255, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_INTERRUPT_GATE);
    
  x86_idt_flush (&idt_ptr);
}

INLINE int
interrupt_is_fatal (unsigned int interrupt)
{
  return (interrupt <= X86_INT_SIMD_FPE  &&
	  interrupt != X86_INT_GENERAL_PROTECTION_FAULT &&
	  interrupt != X86_INT_PAGE_FAULT) || 
    interrupt == KERNEL_BUGCHECK_INTERRUPT;
  
}

void
x86_isr_handler (struct x86_stack_frame *frame)
{
  int old_ctx;
  void *old_frame;
  struct task *task;
  DWORD cr2;

  old_ctx   = get_current_context ();
  old_frame = get_interrupt_frame ();
  task      = get_current_task ();
  
  set_current_context (KERNEL_CONTEXT_INTERRUPT);
  set_interrupt_frame (frame);

  __asm__ __volatile__ ("movl %%cr2, %0" : "=g" (cr2));

  if (task->ts_state == TASK_STATE_RUNNING && frame->segs.ds == 0x20 && frame->segs.es == 0)
    FAIL ("detected!\n");
  
  if (frame->int_no >= 32 && frame->int_no < 56)
  {
    /* No problem: under IRQ, interrupts are disabled */
    
    outportb (0x20, 0x20); /* PIC_EOI, end of interrupt, interrupts will arrive later */
    
    /* We don't care about interrupts beyond this point. Still in
       interrupt context, no interrupts enabled meanwhile */
    if (!do_irq_from_interrupt (frame->int_no - 32, frame))
      debug ("spurious irq %d\n", frame->int_no - 32);
  }
  else
  {
    if (old_ctx == KERNEL_CONTEXT_BOOT_TIME)
    {
      panic ("microkernel interrupt %d while booting!\n", frame->int_no);
      kernel_halt ();
    }
    else if (old_ctx == KERNEL_CONTEXT_INTERRUPT)
    {
      panic ("fatal: microkernel fault while handling fault\n");
      
      kernel_halt ();
    }

    if (task->ts_type == TASK_TYPE_KERNEL_THREAD)
      x86_regdump (frame);

    switch (frame->int_no)
    {
    case KERNEL_BUGCHECK_INTERRUPT:
      panic ("microkernel bugcheck");
      kernel_halt ();
	    
      break;
      
    case X86_INT_DIVIDE_BY_ZERO:
    case X86_INT_X87_FLOATING_POINT_EXCEPTION:
    case X86_INT_SIMD_FPE:
      task_trigger_exception (task, EX_FPE, (busword_t) frame->priv.eip, 0, 0);
      break;

    case X86_INT_INVALID_OPCODE:
      task_trigger_exception (task, EX_ILL_INSTRUCTION, (busword_t) frame->priv.eip, 0, 0);
      break;

    case X86_INT_SECURITY_EXCEPTION:
      task_trigger_exception (task, EX_PRIV_INSTRUCTION, (busword_t) frame->priv.eip, 0, 0);
      break;
      
    case X86_INT_GENERAL_PROTECTION_FAULT:
    case X86_INT_DOUBLE_FAULT:
    case X86_INT_PAGE_FAULT:
      if (vm_handle_page_fault (task, cr2, VREGION_ACCESS_READ) == -1)
	task_trigger_exception (task, EX_SEGMENT_VIOLATION, (busword_t) frame->priv.eip, cr2, 0);
      
      break;

    default:
      panic ("int %d, code %d", frame->int_no, frame->priv.error);
  
      kernel_halt ();
    }
  }

  set_interrupt_frame (old_frame);
  set_current_context (old_ctx);
}

DEBUG_FUNC (x86_enable_interrupts);
DEBUG_FUNC (x86_disable_interrupts);
DEBUG_FUNC (x86_io_wait);
DEBUG_FUNC (x86_idt_flush);
DEBUG_FUNC (x86_init_all_gates);
DEBUG_FUNC (x86_isr_handler);

DEBUG_FUNC (do_irq_from_interrupt);

