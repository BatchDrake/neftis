/*
 *    Types and declarations for the Atomik's filesystem daemon
 *    Copyright (C) 2015  Gonzalo J. Carracedo
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

#ifndef _SERV_FS_H
#define _SERV_FS_H

#include <ipc/msg.h>

#define FS_REQ_TYPE_OPEN  0
#define FS_REQ_TYPE_CLOSE 1
#define FS_REQ_TYPE_READ  2
#define FS_REQ_TYPE_WRITE 3
#define FS_REQ_TYPE_STAT  4

#define FS_REP_TYPE_ERROR 5
#define FS_REP_TYPE_DATA  6

struct fs_msg
{
  struct msg_header fm_header;

  union
  {
    char     fm_filename[0]; /* Name of the file to open */
    uint32_t fm_handle;      /* Numerical handle */
  };
};

#endif /* _SERV_FS_H */
