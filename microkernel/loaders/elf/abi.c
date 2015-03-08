/*
 *    The ELF loader for Atomik microkernel.
 *    Copyright (C) 2014 Gonzalo J. Carracedo <BatchDrake@gmail.com>
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

#include <mm/regions.h>
#include <mm/salloc.h>
#include <mm/vm.h>
#include <mm/anon.h>

#include <task/loader.h>
#include <string.h>
#include <util.h>

#include "elf.h"
#include "elf_state.h"

int
elf32_setup_abi (struct elf32_state *state, Elf32_Phdr *phdr)
{
  void *base = (void *) state->header;
  int p = 0;
  
  struct elf32_note *note;
  unsigned long size;

  const char *name;
  const char *desc;

  state->abi_string = "agnostic";
  
  size = phdr->p_filesz;
  p = 0;

  if (OVERFLOW (phdr->p_offset + size - 1, state->size))
      return -1;

  while (p < size)
  {      
    note = (struct elf32_note *) (base + p + phdr->p_offset);

    p += sizeof (struct elf32_note) + __ALIGN (note->namesz, 4) + note->descsz;
	
    if (OVERFLOW (p - 1, size))
      return -1;

    name = (const char *) note->data;
    desc = (const char *) (note->data + __ALIGN (note->namesz, 4));

    if (note->type == 1)
    {
      /* GNU eabi */
      if (note->namesz == 4 && strncmp (name, "GNU", 3) == 0)
      {
	switch (desc[3])
	{
	case 0:
	  state->abi_string = "elf32-linux-gnu";
	  break;

	case 1:
	  state->abi_string = "elf32-hurd-gnu";
	  break;

	case 2:
	  state->abi_string = "elf32-solaris-gnu";
	  break;

	default:
	  state->abi_string = "elf32-unknown-gnu";
	  
	}
      }
      else if (note->namesz == 4 && strncmp (name, "PaX", 3) == 0)
	state->abi_string = "elf32-netbsd";
      else if (note->namesz == 7 && strncmp (name, "NetBSD", 7) == 0)
	state->abi_string = "elf32-netbsd";

      return 0;
    }
  }

  return 0;
}
