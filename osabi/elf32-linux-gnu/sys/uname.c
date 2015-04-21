/*
 *    Linux uname() system call implementation
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

#include <atomik.h>
#include <linux.h>
#include <unistd_32.h>
#include <errno.h>
#include <stdlib.h>
#include <cpu.h>

#include <sys/uname.h>

void
sys_uname (struct x86_common_regs *regs)
{
  int errno = 0;
  struct utsname *buf = (struct utsname *) regs->ebx;

  if (buf == NULL)
  {
    errno = -EFAULT;
    goto done;
  }

  /* Atomik should provide a way in the future to quickly test a pointer */

  memset (buf, 0, sizeof (struct utsname));
  
  strncpy (buf->sysname,  "Linux",    _UTSNAME_SYSNAME_LENGTH);
  strncpy (buf->nodename, "localhost", _UTSNAME_NODENAME_LENGTH);
  strncpy (buf->release,  "3.0",       _UTSNAME_RELEASE_LENGTH);
  strncpy (buf->version,  "Atomik " __TIMESTAMP__, _UTSNAME_VERSION_LENGTH);
  strncpy (buf->machine,  ARCH_STRING, _UTSNAME_MACHINE_LENGTH);

  errno = 0;
  
done:
  regs->eax = errno;
}

