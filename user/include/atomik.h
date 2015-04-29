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

/* Size of a micromessage */
#define MSG_MICRO_SIZE 256

/* Microkernel system calls */
#define SYS_KRN_exit             0
#define SYS_KRN_debug_int        1
#define SYS_KRN_debug_string     2
#define SYS_KRN_debug_pointer    3
#define SYS_KRN_debug_buf        4
#define SYS_KRN_brk              5
#define SYS_KRN_set_tls          6
#define SYS_KRN_declare_service  7
#define SYS_KRN_query_service    8

/* IPC system calls */
#define SYS_IPC_msg_request      0
#define SYS_IPC_msg_map          1
#define SYS_IPC_msg_unmap        2
#define SYS_IPC_msg_send         3
#define SYS_IPC_msg_recv         4
#define SYS_IPC_msg_read_micro   5
#define SYS_IPC_msg_write_micro  6
#define SYS_IPC_msg_get_info     7
#define SYS_IPC_msg_release      8
#define SYS_IPC_msg_read_by_type 9
#define SYS_IPC_msg_read         10
#define SYS_IPC_msg_write        11

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
int   set_tls (void *);
int   declare_service (const char *);
int   query_service (const char *);

/* IPC system calls */
int msgreq (int size);
int msgmap (int id);
int msgunmap (int id);
int msgsend (int id);
int msgrecv (int id);
int msgread_by_read (unsigned int type, unsigned int link, void *data, unsigned int datasize, int nonblock);
int msgread (void *data, unsigned int datasize, int nonblock);
int msgwrite (int tid, const void *data, unsigned int datasize);
int msggetinfo (int id, void *data);
int msgrelease (int id);



#endif /* _ATOMIK_H */
