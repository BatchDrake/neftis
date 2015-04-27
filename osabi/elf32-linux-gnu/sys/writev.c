/*
 *    Linux writev() system call implementation
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

struct iovec
{
  void *iov_base;   /* Dirección de comienzo */
  size_t iov_len;   /* Número de bytes */
};

void
sys_writev (struct x86_common_regs *regs)
{
  unsigned int i;
  struct iovec *vec = (struct iovec *) regs->ecx;
  
  if (regs->ebx == 1 || regs->ebx == 2)
  {
    puts ("\033[1;37m");
    
    for (i = 0; i < regs->edx; ++i)
      put (vec[i].iov_base, vec[i].iov_len);

    puts ("\033[0m");
  }
  else
  {
    puts ("(o) unimplemented call to writev (");

    puti (regs->ebx);

    puts (", ");

    putp ((void *) regs->ecx);

    puts (", ");

    puti (regs->edx);

    puts (");\n");

    regs->eax = -ENOSYS;
  }
}

