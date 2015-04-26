/*
 *    Entry point for the Atomik's filesystem daemon
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
#include <stdlib.h>
#include <errno.h>

struct fs_msg
{
  uint32_t type;
  uint32_t link;
  uint32_t sender;
};

void
_start (void)
{
  struct fs_msg msg;
  int result;
  
  declare_service ("fs");

  puts ("fs: atomik filesystem service started - version 0.1\n");
  
  while ((result = msgread (&msg, sizeof (struct fs_msg), 0)) != -ENOSYS)
  {
    puts ("fs: received message");
    puts ("\n  result =    ");
    puti (result);
    puts ("\n  fs.type =   ");
    puti (msg.type);
    puts ("\n  fs.link =   ");
    puti (msg.link);
    puts ("\n  fs.sender = ");
    puti (msg.sender);
    puts ("\n\n");
  }

  puts ("fs exited, result: ");
  puti (result);
  puts ("\n");
  exit (0);
}
