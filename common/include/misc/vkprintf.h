/*
 *    vkprintf.h: Formatted output.
 *    Copyright (C) 2014  Gonzalo J. Carracedo <BatchDrake@gmail.com>
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

#ifndef _MISC_VKPRINTF_H
#define _MISC_VKPRINTF_H

#include <types.h>
#include <stdarg.h>

struct vkprintf_stream
{
  memsize_t counter;
  void *opaque;
  
  int (*putchar) (struct vkprintf_stream *, char);
  int (*puts) (struct vkprintf_stream *, const char *);
};

void  vkputchar (struct vkprintf_stream *, char);
void  vkputs (struct vkprintf_stream *, const char *);
void  vkprintf (struct vkprintf_stream *, const char *, va_list);

#endif /* _MISC_VKPRINTF_H */
