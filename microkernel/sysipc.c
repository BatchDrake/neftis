/*
 *    System call interface to IPC
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
#include <task/msg.h>
#include <kctx.h>

SYSPROTO (syscall_ipc_msg_request)
{
  return sys_msg_request ((int) args[0]);
}

SYSPROTO (syscall_ipc_msg_map)
{
  return sys_msg_map ((int) args[0]);
}

SYSPROTO (syscall_ipc_msg_unmap)
{
  return sys_msg_unmap ((int) args[0]);
}

SYSPROTO (syscall_ipc_msg_send)
{
  return sys_msg_send ((int) args[0], (int) args[1]);
}

SYSPROTO (syscall_ipc_msg_recv)
{
  return sys_msg_recv ((int) args[0]);
}

SYSPROTO (syscall_ipc_msg_read_micro)
{
  return sys_msg_read_micro ((int) args[0], (void *) args[1], (unsigned int) args[2]);
}

SYSPROTO (syscall_ipc_msg_write_micro)
{
  return sys_msg_write_micro ((int) args[0], (const void *) args[1], (unsigned int) args[2]);
}

SYSPROTO (syscall_ipc_msg_get_info)
{
  return sys_msg_get_info ((int) args[0], (struct msg_info *) args[1]);
}

SYSPROTO (syscall_ipc_msg_release)
{
  return sys_msg_release ((int) args[0]);
}
