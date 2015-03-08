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

#include <asm/8259-pic.h>
#include <asm/irq.h>

void
pic_remap_irq_vector (int offset1, int offset2)
{
  unsigned char a1, a2;

  a1 = inportb (PIC1_DATA);                        // save masks
  a2 = inportb (PIC2_DATA);
  
  outportb (PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait ();
  outportb (PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait ();
  outportb (PIC1_DATA, offset1);
  io_wait ();
  outportb (PIC2_DATA, offset2);
  io_wait ();
  outportb (PIC1_DATA, 4);
  io_wait ();
  outportb (PIC2_DATA, 2);
  io_wait ();
  outportb (PIC1_DATA, ICW4_8086);
  io_wait ();
  outportb (PIC2_DATA, ICW4_8086);
  io_wait ();
  /* Mask all irqs */
  outportb (PIC1_DATA, 0xff);
  outportb (PIC2_DATA, 0xff);
  
  outportb (PIC1_DATA, a1);   // restore saved masks.
  outportb (PIC2_DATA, a2);
}

void
pic_end_of_interrupt (unsigned char irq)
{
  if (irq >= 8)
    outportb (PIC2_COMMAND, PIC_EOI);

  outportb (PIC1_COMMAND, PIC_EOI);
}

void 
pic_init (void)
{
  pic_remap_irq_vector (PIC_MASTER_INT, PIC_SLAVE_INT); 
}

