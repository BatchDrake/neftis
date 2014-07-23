/*
 *    x86 COM ports implementation
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

#include <dev/serial.h>

#include <asm/com.h>
#include <asm/regs.h>
#include <asm/ports.h>

static uint16_t com_io_ports[] = {COM_PORT_1_IO, COM_PORT_2_IO, COM_PORT_3_IO, COM_PORT_4_IO};

void
com_port_init (uint16_t base)
{
  outportb (base + 1, 0x00);    /* Disable all interrupts */
  outportb (base + 3, 0x80);    /* Enable DLAB (set baud rate divisor) */
  outportb (base + 0, 0x03);    /* Set divisor to 3 (lo byte) 38400 baud */
  outportb (base + 1, 0x00);    /*                  (hi byte) */
  outportb (base + 3, 0x03);    /* 8 bits, no parity, one stop bit */
  outportb (base + 2, 0xC7);    /* Enable FIFO, clear them, with 14-byte threshold */
  outportb (base + 4, 0x0B);    /* IRQs enabled, RTS/DSR set */
}

int
com_transmit_empty (uint16_t base)
{
  return inportb (base + 5) & 0x20;
}
 
void
com_write (uint16_t base, char c)
{
  while (!com_transmit_empty (base));
 
  outportb (base, c);
}

int
serial_putchar (uint16_t port, char byte)
{
  if (port >= 4)
    return -1;

  com_write (com_io_ports[port], byte);
  
  return 0;
}

int
serial_port_count (void)
{
  return 4;
}

void
hw_serial_init (void)
{
  com_port_init (COM_PORT_1_IO);
  com_port_init (COM_PORT_2_IO);
  com_port_init (COM_PORT_3_IO);
  com_port_init (COM_PORT_4_IO);
}
