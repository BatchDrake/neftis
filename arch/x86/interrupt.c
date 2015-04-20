/*
 *    High-level interrupt handling
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
#include <string.h>

#include <task/task.h>
#include <misc/hook.h>
#include <irq/irq.h>

#include <asm/interrupt.h>
#include <asm/task.h>
#include <asm/regs.h>
#include <asm/seg.h>
#include <asm/io.h>
#include <asm/syscall.h>

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

/* User-defined gates */
extern void isr56 (void);
extern void isr57 (void);
extern void isr58 (void);
extern void isr59 (void);
extern void isr60 (void);
extern void isr61 (void);
extern void isr62 (void);
extern void isr63 (void);
extern void isr64 (void);
extern void isr65 (void);
extern void isr66 (void);
extern void isr67 (void);
extern void isr68 (void);
extern void isr69 (void);
extern void isr70 (void);
extern void isr71 (void);
extern void isr72 (void);
extern void isr73 (void);
extern void isr74 (void);
extern void isr75 (void);
extern void isr76 (void);
extern void isr77 (void);
extern void isr78 (void);
extern void isr79 (void);
extern void isr80 (void);
extern void isr81 (void);
extern void isr82 (void);
extern void isr83 (void);
extern void isr84 (void);
extern void isr85 (void);
extern void isr86 (void);
extern void isr87 (void);
extern void isr88 (void);
extern void isr89 (void);
extern void isr90 (void);
extern void isr91 (void);
extern void isr92 (void);
extern void isr93 (void);
extern void isr94 (void);
extern void isr95 (void);
extern void isr96 (void);
extern void isr97 (void);
extern void isr98 (void);
extern void isr99 (void);
extern void isr100 (void);
extern void isr101 (void);
extern void isr102 (void);
extern void isr103 (void);
extern void isr104 (void);
extern void isr105 (void);
extern void isr106 (void);
extern void isr107 (void);
extern void isr108 (void);
extern void isr109 (void);
extern void isr110 (void);
extern void isr111 (void);
extern void isr112 (void);
extern void isr113 (void);
extern void isr114 (void);
extern void isr115 (void);
extern void isr116 (void);
extern void isr117 (void);
extern void isr118 (void);
extern void isr119 (void);
extern void isr120 (void);
extern void isr121 (void);
extern void isr122 (void);
extern void isr123 (void);
extern void isr124 (void);
extern void isr125 (void);
extern void isr126 (void);
extern void isr127 (void);
extern void isr128 (void);
extern void isr129 (void);
extern void isr130 (void);
extern void isr131 (void);
extern void isr132 (void);
extern void isr133 (void);
extern void isr134 (void);
extern void isr135 (void);
extern void isr136 (void);
extern void isr137 (void);
extern void isr138 (void);
extern void isr139 (void);
extern void isr140 (void);
extern void isr141 (void);
extern void isr142 (void);
extern void isr143 (void);
extern void isr144 (void);
extern void isr145 (void);
extern void isr146 (void);
extern void isr147 (void);
extern void isr148 (void);
extern void isr149 (void);
extern void isr150 (void);
extern void isr151 (void);
extern void isr152 (void);
extern void isr153 (void);
extern void isr154 (void);
extern void isr155 (void);
extern void isr156 (void);
extern void isr157 (void);
extern void isr158 (void);
extern void isr159 (void);

/* Microkernel services */
extern void isr160 (void);

/* IPC (mutexes, messages and so on) */
extern void isr161 (void);

/* VMO subsystem - yet to be implemented */
extern void isr162 (void);



/* Bugcheck interrupt */
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
    IDT_PRESENT | I386_INTERRUPT_GATE | IDT_PRIV (3)); /* To make int3 work */

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

  /* IRQ mapping */
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

  /* User-defined interrupt gates */
  x86_idt_set_gate  (56, isr56, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (57, isr57, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (58, isr58, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (59, isr59, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (60, isr60, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (61, isr61, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (62, isr62, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (63, isr63, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (64, isr64, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (65, isr65, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (66, isr66, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (67, isr67, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (68, isr68, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (69, isr69, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (70, isr70, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (71, isr71, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (72, isr72, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (73, isr73, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (74, isr74, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (75, isr75, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (76, isr76, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (77, isr77, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (78, isr78, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (79, isr79, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (80, isr80, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (81, isr81, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (82, isr82, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (83, isr83, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (84, isr84, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (85, isr85, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (86, isr86, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (87, isr87, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (88, isr88, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (89, isr89, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (90, isr90, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (91, isr91, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (92, isr92, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (93, isr93, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (94, isr94, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (95, isr95, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (96, isr96, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (97, isr97, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (98, isr98, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (99, isr99, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (100, isr100, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (101, isr101, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (102, isr102, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (103, isr103, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (104, isr104, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (105, isr105, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (106, isr106, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (107, isr107, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (108, isr108, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (109, isr109, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (110, isr110, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (111, isr111, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (112, isr112, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (113, isr113, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (114, isr114, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (115, isr115, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (116, isr116, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (117, isr117, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (118, isr118, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (119, isr119, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (120, isr120, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (121, isr121, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (122, isr122, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (123, isr123, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (124, isr124, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (125, isr125, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (126, isr126, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (127, isr127, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (128, isr128, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (129, isr129, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (130, isr130, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (131, isr131, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (132, isr132, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (133, isr133, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (134, isr134, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (135, isr135, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (136, isr136, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (137, isr137, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (138, isr138, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (139, isr139, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (140, isr140, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (141, isr141, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (142, isr142, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (143, isr143, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (144, isr144, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (145, isr145, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (146, isr146, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (147, isr147, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (148, isr148, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (149, isr149, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (150, isr150, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (151, isr151, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (152, isr152, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (153, isr153, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (154, isr154, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (155, isr155, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (156, isr156, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (157, isr157, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (158, isr158, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (159, isr159, GDT_SEGMENT_KERNEL_CODE, 
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));


  /* System calls. These are trap gates as they don't need to disable interrupts. */
  x86_idt_set_gate  (160, isr160, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (161, isr161, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  x86_idt_set_gate  (162, isr162, GDT_SEGMENT_KERNEL_CODE,
    IDT_PRESENT | I386_TRAP_GATE | IDT_PRIV (3));

  /* Bugcheck */
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

int
x86_user_interrupt (struct task *task, struct x86_stack_frame *frame, busword_t eip)
{
  /* To make things easier, instead of pushing all general purpose registers, I'll push the return address. It will be up to userland to save all the possibly clobbered registers.

     This is a good idea, as we don't need anything like linux's sigreturn */

  frame->unpriv.old_esp   -= 4;
  
  if (copy2virt (REFCAST (struct vm_space, task->ts_vm_space), frame->unpriv.old_esp, &frame->unpriv.eip, sizeof (busword_t)) != sizeof (busword_t))
    return -1;

  frame->unpriv.eip = (void *) eip;

  return 0;
}

void
x86_isr_handler (struct x86_stack_frame *frame)
{
  int old_ctx;
  void *old_frame;
  struct task *task;
  struct task_ctx_data *ctx_data;
  
  DWORD cr2;

  old_ctx   = get_current_context ();
  old_frame = get_interrupt_frame ();
  task      = get_current_task ();
  
  set_current_context (KERNEL_CONTEXT_INTERRUPT);
  set_interrupt_frame (frame);

  __asm__ __volatile__ ("movl %%cr2, %0" : "=g" (cr2));

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

      x86_regdump (frame);
      
      kernel_halt ();
    }
    else if (old_ctx == KERNEL_CONTEXT_INTERRUPT)
    {
      panic ("fatal: microkernel fault while handling fault\n");

      x86_regdump (frame);
      
      kernel_halt ();
    }

    switch (frame->int_no)
    {
    case KERNEL_SYSCALL_MICROKERNEL:
      x86_sys_microkernel (frame);
      break;

    case KERNEL_SYSCALL_IPC:
      x86_sys_ipc (frame);
      break;

    case KERNEL_SYSCALL_VMO:
      x86_sys_vmo (frame);
      break;
    
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

    case X86_INT_BREAKPOINT:
    case X86_INT_DEBUG:
      x86_regdump (frame);
      task_trigger_exception (task, EX_DEBUGGER_TRAP, (busword_t) frame->priv.eip, 0, 0);
      break;

    case X86_INT_GENERAL_PROTECTION_FAULT:
    case X86_INT_DOUBLE_FAULT:
    case X86_INT_PAGE_FAULT:
      if (task->ts_type == TASK_TYPE_KERNEL_THREAD)
        x86_regdump (frame);
/*
  Uncomment the following when you need to debug the user's stack
      uint32_t dword;
      int i;

      for (i = 0; i < 20; ++i)
      {
        copy2phys (REFCAST (struct vm_space, task->ts_vm_space), &dword, frame->unpriv.old_esp + i * 4, 4);
        printk ("%w - %w\n", frame->unpriv.old_esp + i * 4, dword);
      }
*/    
      if (vm_handle_page_fault (task, cr2, VREGION_ACCESS_READ) == -1)
	task_trigger_exception (task, EX_SEGMENT_VIOLATION, (busword_t) frame->priv.eip, cr2, 0);
      
      break;

    default:
      /* User-defined interrupts. This will help porting foreign
	 ABIs to Atomik directly from userland */
      
      ctx_data = get_task_ctx_data (task);

      if (ctx_data->uisr_eip[frame->int_no] == 0)
	task_trigger_exception (task, EX_ILL_INSTRUCTION, (busword_t) frame->priv.eip, 0, 0);
      else if (x86_user_interrupt (task, frame, ctx_data->uisr_eip[frame->int_no]) == -1)
	task_trigger_exception (task, EX_SEGMENT_VIOLATION, (busword_t) frame->priv.eip, ctx_data->uisr_eip[frame->int_no], 0);
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
DEBUG_FUNC (interrupt_is_fatal);
