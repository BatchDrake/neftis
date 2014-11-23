/*
 *    util.c: Some misc functions widely used by the whole microkernel.
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

#include <types.h>
#include <util.h>
#include <stdarg.h>
#include <console/console.h>
#include <arch.h>

extern struct console *syscon;

void
panic (const char *msg, ...)
{
  va_list ap;
  
  va_start (ap, msg);
  
  puts ("\033[0;31mpanic: ");
  vprintk (msg, ap);
  putchar ('\n');
}

void
kernel_halt (void)
{
  __halt ();
}

void
kernel_pause (void)
{
  __pause ();
}

int
kernel_get_param (const char *param, char *buf, size_t maxsize)
{
  const char *cmdline = kernel_command_line ();
  size_t size = strlen (cmdline);
  size_t paramlen = strlen (param);
  int i, j;

  for (i = 0; i < size; ++i)
  {
    if (strncmp (cmdline + i, param, paramlen) == 0)
    {
      if (cmdline[i + paramlen] == ' ' || cmdline[i + paramlen] == '\0')
      {
        if (buf != NULL && size > 0)
          *buf = '\0';

        return 0;
      }
      else if (cmdline[i + paramlen] == '=')
      {
        ++i;
        
        if (buf != NULL && size > 0)
        {
          for (j = 0;
               cmdline[i + paramlen + j] != ' '  &&
               cmdline[i + paramlen + j] != '\0' &&
               j < maxsize - 1;
               ++j)
            buf[j] = cmdline[i + paramlen + j];

          buf[j] = '\0';
        }

        return 0;
      }
    }
  }

  return -1;
}

int
kernel_option_enabled (const char *param, int def)
{
  char buf[6];

  if (kernel_get_param (param, buf, 6) == -1)
    return def;

  if (!*buf)
    return 1;

  if (strcmp (buf, "false") == 0 ||
      strcmp (buf, "no")    == 0 ||
      strcmp (buf, "off")   == 0)
    return 0;
  else if (strcmp (buf, "true") == 0 ||
           strcmp (buf, "yes")  == 0 ||
           strcmp (buf, "on")   == 0)
    return 1;
  else
    return def;
}

int
isprint (int c)
{
  return c >= ' ' && c < 128;
}

void
hexdump (const void *data, uint32_t size)
{
  const uint8_t *bytes = (const uint8_t *) data;

  int i, j;

  for (i = 0; i < size; ++i)
  {
    if ((i & 0xf) == 0)
      printk ("%w  ", i + bytes);

    printk ("%s%b ", (i & 0xf) == 8 ? " " : "", bytes[i]);

    if ((i & 0xf) == 0xf)
    {
      printk (" |");

      printk (" ");

      for (j = i - 15; j <= i; ++j)
        printk ("%c", isprint (bytes[j]) ? bytes[j] : '.');

      printk ("\n");
    }
  }

  if ((i & 0xf) != 0)
  {
    for (j = i; j < __ALIGN (size, 16); ++j)
      printk ("   %s", (j & 0xf) == 8 ? " " : "");

    printk (" |");

    printk (" ");

    for (j = i & ~0xf; j < size; ++j)
      printk ("%c", isprint (bytes[j]) ? bytes[j] : '.');

    printk ("\n");
  }

  printk ("%w  \n", i + bytes);
}

DEBUG_FUNC (panic);
DEBUG_FUNC (kernel_halt);
DEBUG_FUNC (kernel_pause);
DEBUG_FUNC (kernel_get_param);
DEBUG_FUNC (kernel_option_enabled);
DEBUG_FUNC (isprint);
DEBUG_FUNC (hexdump);
