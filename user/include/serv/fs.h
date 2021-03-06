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
#include <sys/stat.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define FS_REQ_TYPE_OPEN     0
#define FS_REQ_TYPE_CLOSE    1
#define FS_REQ_TYPE_READ     2
#define FS_REQ_TYPE_WRITE    3
#define FS_REQ_TYPE_STAT     4
#define FS_REQ_TYPE_LSEEK    5
#define FS_REQ_TYPE_GETDENTS 6

#define FS_REP_TYPE_ERROR    0
#define FS_REP_TYPE_DATA     1
#define FS_REP_TYPE_HANDLE   2

#define FS_MSG_SIZE_FROM_PAYLOAD(size) (sizeof (struct frag_msg_header) + (size))

struct fs_dent
{
  uint32_t fd_ino;
  uint16_t fd_fnsiz;
  char     fd_fname[0];
};

struct fs_msg
{
  struct frag_msg_header fm_header;

  union
  {
    char     fm_filename[0]; /* Name of the file to open */
    uint8_t  fm_data[0];     /* File data */
    uint32_t fm_handle;      /* Numerical handle */
    uint32_t fm_errno;       /* Error code */
    struct fs_dent fm_dents[0];
    struct stat fm_sbuf;     /* File status */
  };
};

int fs_open (const char *, int);

#endif /* _SERV_FS_H */
