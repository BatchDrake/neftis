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

#include <misc/msgsink.h>
#include <misc/vkprintf.h>
#include <console/console.h>

extern struct console *syscon;

static struct msgsink *msgsinks;

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
msgsink_register (struct msgsink *sink)
{
  sink->next = msgsinks;
  msgsinks = sink;
}

void
putchar (char c)
{
  struct msgsink *this = msgsinks;

  while (this)
  {
    (this->putchar) (this->opaque, c);
    
    this = this->next;
  }
}

void
puts (const char* s)
{
  struct msgsink *this = msgsinks;
  const char *p;
  
  while (this)
  {
    if (this->puts != NULL)
      (this->puts) (this->opaque, s);
    else
      for (p = s; *p; ++p)
        (this->putchar) (this->opaque, *p);

    this = this->next;
  }
}

static int
printk_putchar (struct vkprintf_stream *stream, char c)
{
  putchar (c);

  return 0;
}

static int
printk_puts (struct vkprintf_stream *stream, const char *s)
{
  puts (s);

  return strlen (s);
}

void
printk (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  
  vkprintf (&printk_stream, fmt, ap);

  /* va_end?? */
}

void
vprintk (const char *fmt, va_list ap)
{
  vkprintf (&printk_stream, fmt, ap);
}

DEBUG_FUNC (msgsink_register);
DEBUG_FUNC (putchar);
DEBUG_FUNC (puts);
DEBUG_FUNC (printk_putchar);
DEBUG_FUNC (printk_puts);
DEBUG_FUNC (printk);
DEBUG_FUNC (vprintk);

