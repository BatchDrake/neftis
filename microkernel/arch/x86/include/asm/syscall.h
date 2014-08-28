/*
 *    x86 system call entry points
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

#ifndef _ASM_SYSCALL_H
#define _ASM_SYSCALL_H

void x86_sys_vmo (struct x86_stack_frame *);
void x86_sys_ipc (struct x86_stack_frame *);
void x86_sys_microkernel (struct x86_stack_frame *);

#endif /* _ASM_SYSCALL_H */
