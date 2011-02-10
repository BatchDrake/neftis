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
    
#ifndef _ASM_IRQ_H
#define _ASM_IRQ_H

#include <types.h>
#include <asm/io.h>

#define IOAPIC_MAX_IRQS 24

INLINE void
__irq_unmask (uint8_t irq)
{
  BYTE mask;
  mask  = inportb (0x21);
  mask  &= ~ (1 << irq);
  outportb (0x21, mask);
}

/* Mask IRQ. Disable it */
INLINE void
__irq_mask (uint8_t irq)
{
  BYTE mask;
  mask  = inportb (0x21);
  mask  |= (1 << irq);
  outportb (0x21, mask);
}


void io_wait (void);
void x86_early_irq_init (void);

#endif /* _ASM_IRQ_H */

