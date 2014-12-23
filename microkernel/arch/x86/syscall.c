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

#include <types.h>

#include <asm/interrupt.h>
#include <asm/regs.h>
#include <asm/seg.h>
#include <asm/syscall.h>

#include <kctx.h>

#include <misc/errno.h>
#include <task/syscall.h>

extern syscall_entry_t krn_syscall_list[SYS_KRN_COUNT];

extern syscall_entry_t ipc_syscall_list[SYS_IPC_COUNT];

void
x86_sys_microkernel (struct x86_stack_frame *frame)
{
  syscall_entry_t entry;

  busword_t args[5] = {frame->regs.ebx, frame->regs.ecx, frame->regs.edx, frame->regs.esi, frame->regs.edi};

  if (frame->regs.eax >= SYS_KRN_COUNT || (entry = krn_syscall_list[frame->regs.eax]) == NULL)
  {
    if (frame->regs.eax >= SYS_KRN_PRIVATE)
      frame->regs.eax = arch_private_syscall (frame->regs.eax, args);
    else
      frame->regs.eax = -ENOSYS;
  }
  else
    frame->regs.eax = (entry) (frame->regs.eax, args);
}

void
x86_sys_ipc (struct x86_stack_frame *frame)
{
  syscall_entry_t entry;
  
  busword_t args[5] = {frame->regs.ebx, frame->regs.ecx, frame->regs.edx, frame->regs.esi, frame->regs.edi};
  
  if (frame->regs.eax >= SYS_IPC_COUNT || (entry = ipc_syscall_list[frame->regs.eax]) == NULL)
    frame->regs.eax = -ENOSYS;
  else
    frame->regs.eax = (entry) (frame->regs.eax, args);
}

void
x86_sys_vmo (struct x86_stack_frame *frame)
{
  printk ("VMO subsystem call, function %d\n", frame->regs.eax);
}

DEBUG_FUNC (x86_sys_microkernel);
DEBUG_FUNC (x86_sys_ipc);
DEBUG_FUNC (x86_sys_vmo);
