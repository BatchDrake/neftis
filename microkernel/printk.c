/*
 *    printk.c: Print text messages to the screen and the current log output.
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
 
#include <types.h>
#include <string.h>
#include <stdarg.h>

#include <misc/vkprintf.h>
#include <console/console.h>

extern struct console *syscon;

static int printk_putchar (struct vkprintf_stream *, char);
static int printk_puts (struct vkprintf_stream *, const char *);

static struct vkprintf_stream printk_stream =
{
  .counter = 0,
  .opaque  = NULL,
  .putchar = printk_putchar,
  .puts    = printk_puts
};

void
putchar (char c)
{
  if (syscon)
    console_putchar (syscon, c);
}

void
puts (const char* s)
{
  if (syscon)
    console_puts (syscon, s);
}

static int
printk_putchar (struct vkprintf_stream *stream, char c)
{
  putchar (c);
}

static int printk_puts (struct vkprintf_stream *stream, const char *s)
{
  puts (s);
}

void
printk (const char *fmt, ...)
{
  va_list ap;
  int i;
  
  va_start (ap, fmt);
  
  vkprintf (&printk_stream, fmt, ap);

  /* va_end?? */
}

