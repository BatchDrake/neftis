/*
 *    ELF32 Linux ABI VDSO for Atomik
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
  return kernel (0, 0, 0, 0, 0, 0);
}

int
puti (int i)
{
  return kernel (1, i, 0, 0, 0, 0);
}

int
putp (void *p)
{
  return kernel (3, (int) p, 0, 0, 0, 0);
}

int
puts (const char *s)
{
  return kernel (2, (int) s, 0, 0, 0, 0);
}

int
setintgate (unsigned long gate, const void *isr)
{
  return kernel (0x80000000, gate, (unsigned long) isr, 0, 0, 0);
}
