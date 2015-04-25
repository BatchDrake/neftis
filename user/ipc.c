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

int
msgreq (int size)
{
  ipc (SYS_IPC_msg_request, size, 0, 0, 0, 0);
}

int
msgmap (int id)
{
  return ipc (SYS_IPC_msg_map, id, 0, 0, 0, 0);
}

int
msgunmap (int id)
{
  return ipc (SYS_IPC_msg_unmap, id, 0, 0, 0, 0);
}

int
msgsend (int id)
{
  return ipc (SYS_IPC_msg_send, id, 0, 0, 0, 0);
}

int
msgrecv (int id)
{
  return ipc (SYS_IPC_msg_recv, id, 0, 0, 0, 0);
}

int
msgmicro_read (int id, void *data, unsigned int datasize)
{
  return ipc (SYS_IPC_msg_read_micro, id, (int) data, (int) datasize, 0, 0);
}

int
msgmicro_write (int id, const void *data, unsigned int datasize)
{
  return ipc (SYS_IPC_msg_write_micro, id, (int) data, (int) datasize, 0, 0);
}

int
msgread_by_type (unsigned int type,
                 unsigned int link,
                 void *data,
                 unsigned int size,
                 int nonblock)
{
  return ipc (SYS_IPC_msg_read_by_type, type, link, (int) data, size, nonblock);
}

int
msgread (void *data,
         unsigned int size,
         int nonblock)
{
  return ipc (SYS_IPC_msg_read, (int) data, size, nonblock, 0, 0);
}

int
msgwrite (int tid, const void *data, unsigned int size)
{
  return ipc (SYS_IPC_msg_write, tid, (int) data, size, 0, 0);
}

int
msggetinfo (int id, void *data)
{
  return ipc (SYS_IPC_msg_get_info, id, (int) data, 0, 0, 0);
}

int
msgrelease (int id)
{
  return ipc (SYS_IPC_msg_release, id, 0, 0, 0, 0);
}
