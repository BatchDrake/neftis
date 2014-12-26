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
  ipc (0, size, 0, 0, 0, 0);
}

int
msgmap (int id)
{
  return ipc (1, id, 0, 0, 0, 0);
}

int
msgunmap (int id)
{
  return ipc (2, id, 0, 0, 0, 0);
}

int
msgsend (int id)
{
  return ipc (3, id, 0, 0, 0, 0);
}

int
msgrecv (int id)
{
  return ipc (4, id, 0, 0, 0, 0);
}

int
msgread (int id, void *data, unsigned int datasize)
{
  return ipc (5, id, (int) data, (int) datasize, 0, 0);
}

int
msgwrite (int id, const void *data, unsigned int datasize)
{
  return ipc (6, id, (int) data, (int) datasize, 0, 0);
}

int
msggetinfo (int id, void *data)
{
  return ipc (7, id, (int) data, 0, 0, 0);
}

int
msgrelease (int id)
{
  return ipc (8, id, 0, 0, 0, 0);
}
