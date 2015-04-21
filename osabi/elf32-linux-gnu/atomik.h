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

#ifndef _ATOMIK_H
#define _ATOMIK_H

#include <types.h>
#include <layout.h>

#define NULL ((void *) 0)

static inline int
kernel (int syscall, int b, int c, int d, int e, int f)
{
  int ret;
  
  asm volatile ("xchg %%edi, %%ebx\n"
                "int $0xa0\n"
                "xchg %%edi, %%ebx\n" : "=a" (ret) : "a" (syscall), "D" (b), "c" (c), "d" (d), "S" (e) : "memory");

  return ret;
}

static inline int
ipc (int syscall, int b, int c, int d, int e, int f)
{
  int ret;
  
  asm volatile ("xchg %%edi, %%ebx\n"
                "int $0xa1\n"
                "xchg %%edi, %%ebx\n" : "=a" (ret) : "a" (syscall), "D" (b), "c" (c), "d" (d), "S" (e) : "memory");

  return ret;
}

/* Generic system calls */
int   exit (int code);
int   puti (int i);
int   putp (void *p);
int   puts (const char *s);
int   put (const void *, int);

int   setintgate (unsigned long gate, const void *isr);
void *brk (void *);

/* IPC system calls */
int msgreq (int size);
int msgmap (int id);
int msgunmap (int id);
int msgsend (int id);
int msgrecv (int id);
int msgread (int id, void *data, unsigned int datasize);
int msgwrite (int id, const void *data, unsigned int datasize);
int msggetinfo (int id, void *data);
int msgrelease (int id);



#endif /* _ATOMIK_H */
