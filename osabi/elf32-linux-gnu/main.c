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
#include <errno.h>
#include <elf.h>

#include <serv/fs.h>

static int fs_tid;

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
wait_for_service (const char *service)
{
  /* Horrible busy loop */
  while ((fs_tid = query_service (service)) == -ESRCH);
}

void
test_open (const char *file)
{
  puts ("osabi-test-open: ");
  puts (file);
  puts (": ");
  puti (fs_open (file, 0));
  puts ("\n");
}

void
fs_init (void)
{
  int fd;

  test_open ("/");
  test_open ("/etc");
  test_open ("/fsdfasd");
  test_open ("/bin/uname");
  test_open ("/etc/atomik/settings.conf");
  test_open ("/etc/atomik/settings.conf/nothing");
  
}

void
linux_abi_init (int (*entry) (), Elf32_Ehdr *imagebase)
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

      (char *) AT_PHNUM,
      (char *) (unsigned long) imagebase->e_phnum,

      (char *) AT_PHDR,
      (char *) imagebase + imagebase->e_phoff,

      (char *) AT_SECURE,
      (char *) 0,
      
      /* End of auxiliary vectors */
      (char *) AT_NULL,
      NULL
    };

  wait_for_service ("fs");
  fs_init ();

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
  "pushl %ebx\n" /* Image base */
  "pushl %eax\n" /* Program entry */
  "call linux_abi_init\n"
);

