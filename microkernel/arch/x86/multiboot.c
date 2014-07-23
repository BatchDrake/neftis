/*
 *    multiboot.c: Multiboot handling functions
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

#include <util.h>
#include <multiboot.h>

BOOT_SYMBOL (static struct multiboot_info *multiboot_info);

void
got_multiboot (struct multiboot_info *mbi)
{
  multiboot_info = mbi;
}

struct multiboot_info *
multiboot_location (void)
{
  return multiboot_info;
}

const char *
kernel_command_line (void)
{
  if (multiboot_info->flags & (1 << 2))
    return (const char *) multiboot_info->cmdline;
  else
    return "";
}
