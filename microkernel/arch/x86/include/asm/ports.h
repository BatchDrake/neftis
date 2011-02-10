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
    
#ifndef _ASM_PORTS_H
#define _ASM_PORTS_H

#include <types.h>

/* inportb: Devuelve un byte del puerto de software PORT */
INLINE BYTE
inportb (WORD port)
{
  BYTE rv;
  __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
  return rv;
}

/* outportb: EnvÃÂ­a un byte DATA al puerto de software PORT */
INLINE void
outportb (WORD port, BYTE data)
{
  __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}


INLINE void
outl (WORD port, DWORD value)
{
  __asm__ __volatile__("outl %1, %0" : : "dN" (port), "a" (value));
}

INLINE DWORD
inl (WORD port)
{
  DWORD Ret;
  __asm__ __volatile__("inl %1, %0" : "=a" (Ret) : "dN" (port));
  return Ret;
}

#endif /* _ASM_PORTS_H */

