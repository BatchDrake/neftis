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

#include <misc/tar.h>

#include <arch.h>
#include <kctx.h>
#include <string.h>

static int
__srv_load_exec (const char *path, const void *base, uint32_t size, uint32_t mode, void *opaque)
{
  struct task *task;

  if (mode & 0100)
  {
    if ((task = user_task_new_from_exec (base, size)) != NULL)
    {
      debug ("%s running (tid: %d)\n", path + 5, task->ts_tid);
      
      wake_up (task, TASK_STATE_RUNNING, WAKEUP_EXPLICIT);
    }
    else
      error ("cannot execute %s\n", path + 5);
  }

  return 0;
}

int
srvrd_load (const void *base, uint32_t size)
{
  pause ();

  return tar_file_walk (base, size, "serv/", __srv_load_exec, NULL);
}

DEBUG_FUNC (srvrd_load);
