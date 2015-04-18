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

void _start (void);
void __syscall_asm (void);

void
linux_abi_init (int (*entry) ())
{
  char *minimal_argv[] = {"dummyname", NULL};
  char *minimal_envp[] = {"TERM=linux", "SHELL=/bin/sh", "USER=root", "HOME=/", "PWD=/", NULL};
  
  setintgate (0x80, linux_syscall);

  asm
  (
    "pushl %2\n"
    "pushl %1\n"
    "pushl $1\n"
    "jmpl *%0\n"
    ::
     "g" (entry),
     "g" (minimal_argv),
     "g" (minimal_envp)
  );
}

asm
(
  ".globl _start\n"
  "_start:\n"
  "pushl %eax\n"
  "call linux_abi_init\n"
);

