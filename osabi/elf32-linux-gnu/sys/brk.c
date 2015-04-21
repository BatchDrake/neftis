/*
 *    Linux brk() system call implementation
 *    Copyright (C) 2015  Gonzalo J. Carracedo
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
#include <linux.h>
#include <unistd_32.h>
#include <errno.h>
#include <stdlib.h>
#include <cpu.h>

void
sys_brk (struct x86_common_regs *regs)
{
  void *prog_brk;

  prog_brk = brk ((void *) regs->ebx) + 1;

  if (regs->ebx == 0)
    puts ("*** osabi: brk(0) call: ");
  else
  {
    puts ("*** osabi: unimplemented brk(");

    putp ((void *) regs->ebx);
    
    puts (") call: ");
  }
  
  putp (prog_brk);

  puts ("\n");
  
  regs->eax = (DWORD) prog_brk;
}

