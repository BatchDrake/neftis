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
#include <linux.h>
#include <unistd_32.h>
#include <errno.h>

#include <syscall_list.h>

void
syscall_run (struct x86_common_regs regs)
{
  const struct syscall_desc *desc;

  if ((desc = syscall_get (regs.eax)) == NULL)
  {
    puts ("*** osabi: unknown syscall #");

    puti (regs.eax);

    puts ("\n");

    regs.eax = -ENOSYS;
  }
  else if (desc->sd_impl == NULL)
  {
    puts ("*** osabi: unimplemented syscall ");

    puts (desc->sd_name);

    puts ("\n");

    regs.eax = -ENOSYS;
  }
  else
    (desc->sd_impl) (&regs);
}

asm
(
  ".globl linux_syscall\n"
  "linux_syscall:\n"
  "  pusha\n"
  "  call syscall_run\n"
  "  popa\n"
  "  ret\n"
);

