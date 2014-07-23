/*
 *    serial.h: Serial port interface.
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

#include <arch.h>
#include <misc/msgsink.h>
#include <dev/serial.h>

void
serial_msgsink_putchar (void *opaque, char c)
{
  serial_putchar ((busword_t) opaque, c);
}

void
serial_init (void)
{
  static struct msgsink serial_msgsink =
  {
    .opaque  = NULL,
    .putchar = serial_msgsink_putchar,
    .puts    = NULL
  };
  
  hw_serial_init ();

  if (serial_port_count () > 0)
    msgsink_register (&serial_msgsink);
}
