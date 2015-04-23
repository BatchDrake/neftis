/*
 *    Linux brk() system call implementation
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

 struct user_desc
 {
   unsigned int  entry_number;
   unsigned int  base_addr;
   unsigned int  limit;
   unsigned int  seg_32bit:1;
   unsigned int  contents:2;
   unsigned int  read_exec_only:1;
   unsigned int  limit_in_pages:1;
   unsigned int  seg_not_present:1;
   unsigned int  useable:1;
 };

void
sys_set_thread_area (struct x86_common_regs *regs)
{
  struct user_desc *desc;

  if ((desc = (struct user_desc *) regs->ebx) == NULL)
  {
    regs->eax = -EFAULT;

    return;
  }
  else if (desc->seg_not_present || !desc->seg_32bit || desc->read_exec_only) /* This is not allowed */
  {
    puts ("*** osabi: illegal set_thread_area() call\n");
    
    regs->eax = -EINVAL;

    return;
  }
  
  if (desc->entry_number == -1)
    desc->entry_number = 6; /* 0x6 * 0x8 = 0x30 */

  if (desc->entry_number != 6)
  {
    puts ("*** osabi: application attempted to set an illegal TLS segment\n");
    
    regs->eax = -ESRCH;

    return;
  }

  if ((regs->eax = set_tls ((void *) desc->base_addr)) == 0)
    *((uint32_t *) desc->base_addr) = desc->base_addr;
}
