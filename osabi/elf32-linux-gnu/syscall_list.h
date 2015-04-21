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

#ifndef _SYSCALL_LIST_H
#define _SYSCALL_LIST_H

#define SYSCALL_LIST_MAX 349

#include <linux.h>

struct syscall_desc
{
  const char *sd_name;
  void (*sd_impl) (struct x86_common_regs *);
};

const struct syscall_desc *syscall_get (unsigned int nr);

#endif /* _SYSCALL_LIST_H */
