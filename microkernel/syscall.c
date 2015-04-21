/*
 *    Architecture-agnostic syscall implementation
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
#include <task/syscall.h>
#include <kctx.h>

syscall_entry_t krn_syscall_list[SYS_KRN_COUNT] =
{
  syscall_krn_exit,
  syscall_krn_debug_int,
  syscall_krn_debug_string,
  syscall_krn_debug_pointer,
  syscall_krn_debug_buf,
  syscall_krn_brk
};

syscall_entry_t ipc_syscall_list[SYS_IPC_COUNT] =
{
  syscall_ipc_msg_request,
  syscall_ipc_msg_map,
  syscall_ipc_msg_unmap,
  syscall_ipc_msg_send,
  syscall_ipc_msg_recv,
  syscall_ipc_msg_read_micro,
  syscall_ipc_msg_write_micro,
  syscall_ipc_msg_get_info,
  syscall_ipc_msg_release
};

syscall_entry_t vmo_syscall_list[SYS_VMO_COUNT];

