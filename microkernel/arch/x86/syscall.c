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

void
x86_sys_microkernel (struct x86_stack_frame *frame)
{
  DECLARE_CRITICAL_SECTION (sys);

  TASK_ATOMIC_ENTER (sys);
  
  printk ("Microkernel system call (called from userland address %p), function %d - stack@%p\n", frame->priv.eip, frame->regs.eax, frame);

  printk ("Task about to be destroyed\n");

  (void) wake_up (get_current_task (), TASK_STATE_EXITED, 0);

  task_destroy (get_current_task ());

  schedule ();

  TASK_ATOMIC_LEAVE (sys);
}

void
x86_sys_ipc (struct x86_stack_frame *frame)
{
  printk ("IPC subsystem call, function %d\n", frame->regs.eax);
}

void
x86_sys_vmo (struct x86_stack_frame *frame)
{
  printk ("VMO subsystem call, function %d\n", frame->regs.eax);
}

DEBUG_FUNC (x86_sys_microkernel);
DEBUG_FUNC (x86_sys_ipc);
DEBUG_FUNC (x86_sys_vmo);
