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
#include <elf.h>

void _start (void);
void __syscall_asm (void);
void __kernel_vsyscall (void);

asm
(
  ".globl __kernel_vsyscall\n"
  "__kernel_vsyscall:\n"
  "  int $0x80\n"
  "  ret\n"
);

/* Linux programs need auxiliary vectors to be properly initialized. */

/* Please read http://articles.manugarg.com/aboutelfauxiliaryvectors.html */

/* Trust me, I'm an engineer */
static uint32_t some_fancy_random_chars[4] = {0xdeadcefe, 0xcafebabe, 0x001c0c0a, 0x12345678};

void
linux_abi_init (int (*entry) ())
{
  char *initial_stack[] =
    {
      (char *) 1, /* argc */

      "dummyname", NULL, /* argv */
      
      "TERM=linux", "SHELL=/bin/sh", "USER=root", "HOME=/", "PWD=/", NULL, /* envp */
      
      /* This is just ugly, and somebody should make it prettier */
      
      (char *) AT_SYSINFO,
      (char *) __kernel_vsyscall,
      
      (char *) AT_RANDOM,
      (char *) some_fancy_random_chars,
      
      /* End of auxiliary vectors */
      (char *) AT_NULL,
      NULL
    };
  
  setintgate (0x80, linux_syscall);

  /* Setup TLS. In Linux, %gs:0x0 points
     to its virtual address (some programs that
     cannot use segment registers -like most
     C applications- need to know the equivalent
     virtua address of TLS)*/
  asm ("movl $0xcffff800, %gs:0x0");
  
  asm
  (
    "movl %0, %%eax\n"
    "movl %1, %%esp\n"
    "jmpl *%%eax\n"
    ::
     "g" (entry),
     "c" (initial_stack)
  );
}

asm
(
  ".globl _start\n"
  "_start:\n"
  "pushl %eax\n"
  "call linux_abi_init\n"
);

