/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) <year>  <name of author>
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
    
#ifndef _ASM_TASK_H
#define _ASM_TASK_H

#include <asm/seg.h>

#define KERNEL_MODE_STACK_PAGES 4

struct task_ctx_stack_info
{
  DWORD stack_bottom;
  DWORD esp;
  DWORD useresp;
};

struct task_ctx_data
{
  struct task_ctx_stack_info stack_info;
  struct tss                 tss;
};

INLINE DWORD
get_eflags (void)
{
  DWORD ret;
  
  __asm__ __volatile__ ("pushf");
  __asm__ __volatile__ ("pop %0" : "=g" (ret)); /* We use this instead of
   popping directly to %eax to avoid warnings. TODO: do it in assembly */
   
   return ret;
}

INLINE int
esp_is_sane (struct task *task, DWORD esp)
{
  return ((DWORD) task + sizeof (struct task)) > esp &&
         esp < ((DWORD) task + KERNEL_MODE_STACK_PAGES << __PAGE_BITS);
}

struct task_ctx_data *get_task_ctx_data (struct task *);

#endif /* _ASM_TASK_H */

