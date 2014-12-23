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

void *
elf32_open (const void *base, uint32_t size)
{
  struct elf32_state *state;
  Elf32_Ehdr *ehdr;

  ehdr = (Elf32_Ehdr *) base;

  /* XXX: rename this macro, it means exactly the opposite it
     is intended for */
  
  if (IN_BOUNDS (sizeof (Elf32_Ehdr), size))
    goto FAIL_MISERABLY;

  if (memcmp (ehdr->e_ident, ELFMAG, SELFMAG))
    goto FAIL_MISERABLY;

  if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
    goto FAIL_MISERABLY;
  
  /* Currently only Elf32 LSB are supported */
  if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
    goto FAIL_MISERABLY;

  if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN)
    goto FAIL_MISERABLY;

  /* We must change this according to architecture */
  if (ehdr->e_machine != EM_386)
    goto FAIL_MISERABLY;

  /* Unexpected Elf32 program header entry size */
  if (ehdr->e_phentsize != sizeof (Elf32_Phdr))
    goto FAIL_MISERABLY;
  
  if (IN_BOUNDS (ehdr->e_phoff, size))
    goto FAIL_MISERABLY;

  if (IN_BOUNDS (ehdr->e_phoff + sizeof (Elf32_Phdr) * ehdr->e_phnum - 1, size))
    goto FAIL_MISERABLY;
  
  CONSTRUCT_STRUCT (elf32_state, state);
  
  state->header = ehdr;
  state->phdrs  = (Elf32_Phdr *) (base + ehdr->e_phoff);
  state->size   = size;
  state->dyn    = ehdr->e_type == ET_DYN;
  state->addr   = 0;
  
  return state;
  
FAIL_MISERABLY:
  return KERNEL_INVALID_POINTER;
}

busword_t
elf32_entry (void *opaque)
{
  struct elf32_state *state = (struct elf32_state *) opaque;

  return state->header->e_entry;
}

int
elf32_walkseg (void *opaque, struct vm_space *space, int (*callback) (struct vm_space *, int, int, busword_t, busword_t, const void *, busword_t))
{
  unsigned int i, phnum;
  int flags;
  int count = 0;
  BOOL zeropg = FALSE;
  
  struct elf32_state *state = (struct elf32_state *) opaque;

  phnum = state->header->e_phnum;
  
  for (i = 0; i < phnum; ++i)
    if (state->phdrs[i].p_type == PT_LOAD)
    {
      if (IN_BOUNDS (state->phdrs[i].p_offset, state->size) ||
          IN_BOUNDS (state->phdrs[i].p_offset + state->phdrs[i].p_filesz - 1, state->size))
      {
	warning ("program header %d maps outside executable, assuming zero page\n", i);
        zeropg = TRUE;
      }

      flags = 0;

      if (state->phdrs[i].p_flags & PF_X)
        flags |= VREGION_ACCESS_EXEC;

      if (state->phdrs[i].p_flags & PF_W)
        flags |= VREGION_ACCESS_WRITE;

      if (state->phdrs[i].p_flags & PF_R)
        flags |= VREGION_ACCESS_READ;

      if ((callback) (space,
		      VREGION_ROLE_USERMAP,
		      flags,
		      state->dyn ?
		        state->phdrs[i].p_vaddr + state->addr :
		        state->phdrs[i].p_vaddr,
		      state->phdrs[i].p_memsz,
		      ((void *) state->header) + state->phdrs[i].p_offset,
		      zeropg ? 0 : state->phdrs[i].p_filesz) == -1)
        return KERNEL_ERROR_VALUE;

      ++count;
    }

  return count;
}

int
elf32_rebase (void *opaque, busword_t base)
{
  struct elf32_state *state = (struct elf32_state *) opaque;

  if (base & (PAGE_SIZE - 1))
    return -1;

  state->addr = base;
  
  return 0;
}

void
elf32_close (void *opaque)
{
  sfree (opaque);
}

size_t
elf32_get_abi (void *opaque, char *buf, size_t size)
{
  const char *abi;

  struct elf32_state *state = (struct elf32_state *) opaque;

  if (state->header->e_ident[EI_OSABI] == ELFOSABI_LINUX)
    abi = "elf32-linux-gnu";
  else
    abi = "agnostic";

  strncpy (buf, abi, size);

  return strlen (abi);
}

void
elf_init (void)
{
  struct loader *elfloader;
  
  debug ("Adding ELF32 support...\n");

  if (PTR_UNLIKELY_TO_FAIL (elfloader = loader_register ("elf32", "Executable and Linking Format (ELF) - 32 bit")))
    FAIL ("Couldn't initialize ELF 32\n");

  elfloader->open    = elf32_open;
  elfloader->entry   = elf32_entry;
  elfloader->walkseg = elf32_walkseg;
  elfloader->close   = elf32_close;
  elfloader->rebase  = elf32_rebase;
  elfloader->get_abi = elf32_get_abi;
}
