/*
 *    Load initial servers from initial ramdisk
 *    Copyright (C) 2014  BatchDrake@gmail.com
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

#include <mm/vm.h>
#include <mm/vremap.h>

#include <task/task.h>
#include <task/sched.h>
#include <task/loader.h>
#include <task/msg.h>

#include <arch.h>
#include <kctx.h>

#define TAR_BLKSIZE 512

struct posix_header
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
                                /* 500 */
};

#define isodigit(c) ((c) >= '0' && (c) < '8')

int
srvrd_load (const void *base, uint32_t size)
{
  struct task *task;
  
  uint32_t p = 0;
  char c;
  struct posix_header *hdr;
  uint32_t fsize;
  uint32_t fact;
  uint32_t mode;
  int i;

  pause ();
  
  while (p + TAR_BLKSIZE <= size)
  {
    hdr = (struct posix_header *) (base + p);

    if (!hdr->name[0])
      break;
    
    p += TAR_BLKSIZE;
    
    fsize = 0;
    fact  = 1;
    mode  = 0;

    for (i = 0; i < 11; ++i)
    {
      if (!isodigit (c = hdr->size[10 - i]))
      {
	error ("offset 0x%x: corrupt header (%s)\n", p, hdr->size);
	return -1;
      }
      else
      {
	fsize += (c - '0') * fact;
	fact <<= 3;
      }
    }

    fact = 1;
    
    for (i = 0; i < 7; ++i)
    {
      if (!isodigit (c = hdr->mode[6 - i]))
      {
	error ("offset 0x%x: corrupt header\n", p);
	return -1;
      }
      else
      {
	mode += (c - '0') * fact;
	fact <<= 3;
      }
    }

    if ((hdr->typeflag == 0 || hdr->typeflag == '0') &&
	strncmp (hdr->name, "serv/") &&
	mode & 0100)
    {
      if ((task = user_task_new_from_exec (base + p, fsize)) != NULL)
      {
	debug ("%s running (tid: %d)\n", hdr->name + 5, task->ts_tid);

	wake_up (task, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
      }
      else
	error ("cannot execute %s\n", hdr->name + 5);
    }
    
    p += ((fsize / TAR_BLKSIZE) + !!(fsize & (TAR_BLKSIZE - 1))) * TAR_BLKSIZE;
  }

  return 0;
}

DEBUG_FUNC (srvrd_load);
