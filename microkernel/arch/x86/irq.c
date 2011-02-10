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
 
#include <types.h>
#include <misc/hook.h>

#include <asm/irq.h>
#include <asm/cpuid.h>

#include <asm/8259-pic.h>

/* io_wait: Espera a que una operaciÃ³n de entrada / salida se complete. */
void
io_wait (void)
{
  __asm__ __volatile__ ("jmp 1f");
  __asm__ __volatile__ ("1:");
  __asm__ __volatile__ ("jmp 1f");
  __asm__ __volatile__ ("1:");
}

void
x86_8259_mask_all (void)
{
  int i;
  
  for (i = 0; i < 16; i++)
    __irq_mask (i);
}

void
x86_early_irq_init (void)
{
  if (cpu_has_feature (CPU_FEATURE_APIC))
  {
    debug ("detected IOAPIC, ignored\n");
  }
  
  pic_init ();
  x86_8259_mask_all ();
}

