/*
 *    System call table
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

#ifndef _TASK_SYSCALL_H
#define _TASK_SYSCALL_H

#include <types.h>

typedef int (*syscall_entry_t) (const busword_t *);

#define SYS_KRN_exit     0

#define SYS_KRN_COUNT    3

#define SYS_IPC_COUNT    9

#define SYS_VMO_COUNT    1

#define SYSPROTO(name) \
  int name (const busword_t *args)

#include <task/syskrn.h>

#include <task/sysipc.h>

#endif /* _TASK_SYSCALL_H */
