/*
 *    The Atomik filesystem interface
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
#include <stdlib.h>
#include <ipc/msg.h>
#include <serv/fs.h>
#include <errno.h>

static int fs_service_tid = -1;

int
fs_tid (void)
{
  if (fs_service_tid < 0)
    fs_service_tid = query_service ("fs");

  return fs_service_tid;
}

int
fs_open (const char *path, int mode)
{
  struct fs_msg *msg, *rep;
  unsigned int size;
  int msgid;
  int ret;
  int tid;
  
  if ((msg = malloc (FS_MSG_SIZE_FROM_PAYLOAD (strlen (path) + 1))) == NULL)
    return -ENOMEM;

  msg->fm_header.mh_type = FS_REQ_TYPE_OPEN;
  msg->fm_header.mh_link = 0;
  
  strcpy (msg->fm_filename, path);
  
  tid = fs_tid ();

  if ((ret = fragmsg_write (tid, msg, FS_MSG_SIZE_FROM_PAYLOAD (strlen (path) + 1))) < 0)
  {
    free (msg);
    return ret;
  }
  
  if ((ret = fragmsg_read ((void **) &rep, &size, 0)) < 0)
  {
    free (msg);
    return ret;
  }

  if (rep->fm_header.mh_type == FS_REP_TYPE_ERROR)
    ret = -rep->fm_errno;
  else
    ret = rep->fm_handle;
  
  free (msg);
  free (rep);

  return ret;
}
