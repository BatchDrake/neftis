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
    
#ifndef _ASM_8259_PIC_H
#define _ASM_8295_PIC_H

#define PIC1    0x20    /* IO base address for master PIC */
#define PIC2    0xA0    /* IO base address for slave PIC */
#define PIC1_COMMAND  PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND  PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI   0x20    /* End-of-interrupt command code */


#define ICW1_ICW4 0x01    /* ICW4 (not) needed */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL  0x08    /* Level triggered (edge) mode */
#define ICW1_INIT 0x10    /* Initialization - required! */

#define ICW4_8086 0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C    /* Buffered mode/master */
#define ICW4_SFNM 0x10    /* Special fully nested (not) */

#define PIC_MASTER_INT 0x20
#define PIC_SLAVE_INT  (PIC_MASTER_INT + 8)


#define IRQ_TIMER      0
#define IRQ_KEYBOARD   1
#define IRQ_CASCADE    2 /* Never raised. */
#define IRQ_COM2       3
#define IRQ_COM1       4
#define IRQ_FLOPPY     5
#define IRQ_LPT1       7
#define IRQ_CMOS_RTCLK 8

#define IRQ_MAX        16 /* Maximum number of IRQs supported */

void pic_remap_irq_vector (int, int);
void pic_end_of_interrupt (unsigned char irq);
void pic_init (void);

#endif /* _ASM_8259_PIC_H */

