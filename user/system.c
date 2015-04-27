/*
 *    Atomik microkernel system interface
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

#include <atomik.h>

int
exit (int code)
{
  return kernel (SYS_KRN_exit, 0, 0, 0, 0, 0);
}

int
puti (int i)
{
  return kernel (SYS_KRN_debug_int, i, 0, 0, 0, 0);
}

int
putp (void *p)
{
  return kernel (SYS_KRN_debug_pointer, (int) p, 0, 0, 0, 0);
}

int
puts (const char *s)
{
  return kernel (SYS_KRN_debug_string, (int) s, 0, 0, 0, 0);
}


int
put (const void *buf, int size)
{
  return kernel (SYS_KRN_debug_buf, (int) buf, size, 0, 0, 0);
}

/* Only makes sense in x86 */
int
setintgate (unsigned long gate, const void *isr)
{
  return kernel (0x80000000, gate, (unsigned long) isr, 0, 0, 0);
}

void *
brk (void *new_break)
{
  return (void *) kernel (SYS_KRN_brk, (unsigned long) new_break, 0, 0, 0, 0);
}

int
set_tls (void *new_tls)
{
  return kernel (SYS_KRN_set_tls, (int) new_tls, 0, 0, 0, 0);
}

int
declare_service (const char *name)
{
  return kernel (SYS_KRN_declare_service, (int) name, 0, 0, 0, 0);
}

int
query_service (const char *name)
{
  return kernel (SYS_KRN_query_service, (int) name, 0, 0, 0, 0);
}
