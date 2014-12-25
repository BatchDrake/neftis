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

static int
elf32_preload_info (struct elf32_state *state)
{
  int i = 0;
  for (i = 0; i < state->header->e_phnum; ++i)
    switch (state->phdrs[i].p_type)
    {
    case PT_NOTE:
      if (elf32_setup_abi (state, &state->phdrs[i]) == -1)
	return -1;
      break;

    case PT_DYNAMIC:
      if (elf32_parse_dyn (state, &state->phdrs[i]) == -1)
	return -1;
      break;
    }
  
  return 0;
}

void *
elf32_open (const void *base, uint32_t size)
{
  struct elf32_state *state;
  Elf32_Ehdr *ehdr;

  ehdr = (Elf32_Ehdr *) base;

  /* XXX: rename this macro, it means exactly the opposite it
     is intended for */
  
  if (OVERFLOW (sizeof (Elf32_Ehdr), size))
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
  
  if (OVERFLOW (ehdr->e_phoff, size))
    goto FAIL_MISERABLY;

  if (OVERFLOW (ehdr->e_phoff + sizeof (Elf32_Phdr) * ehdr->e_phnum - 1, size))
    goto FAIL_MISERABLY;

  if (OVERFLOW (ehdr->e_shoff, size))
    goto FAIL_MISERABLY;

  if (OVERFLOW (ehdr->e_shoff + sizeof (Elf32_Shdr) * ehdr->e_shnum - 1, size))
    goto FAIL_MISERABLY;

  CONSTRUCT_STRUCT (elf32_state, state);
  
  state->header = ehdr;
  state->phdrs  = (Elf32_Phdr *) (base + ehdr->e_phoff);
  state->shdrs  = (Elf32_Shdr *) (base + ehdr->e_shoff);
  state->size   = size;
  state->dyn    = ehdr->e_type == ET_DYN;
  state->addr   = 0;

  if (elf32_preload_info (state) == -1)
  {
    sfree (state);
    
    goto FAIL_MISERABLY;
  }

  return state;
  
FAIL_MISERABLY:
  return KERNEL_INVALID_POINTER;
}

busword_t
elf32_entry (void *opaque)
{
  struct elf32_state *state = (struct elf32_state *) opaque;

  return state->dyn ?
    state->addr + state->header->e_entry :
    state->header->e_entry;
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
      if (OVERFLOW (state->phdrs[i].p_offset, state->size) ||
          OVERFLOW (state->phdrs[i].p_offset + state->phdrs[i].p_filesz - 1, state->size))
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

int
elf32_relocate (void *opaque, struct vm_space *space)
{
  struct elf32_state *state = (struct elf32_state *) opaque;

  if (state->rel_count > 0)
    if (elf32_parse_rel (state, space, state->rel, state->rel_count) == -1)
      return -1;

  if (state->rela_count > 0)
    if (elf32_parse_rela (state, space, state->rela, state->rela_count) == -1)
      return -1;

  if (state->jmprel_count > 0)
  {
    if (state->jmprel_type == DT_REL)
    {
      if (elf32_parse_rel (state, space, state->jmprel, state->jmprel_count) == -1)
	return -1;
    }
    else
    {
      if (elf32_parse_rela (state, space, state->jmprela, state->jmprel_count) == -1)
	return -1;

    }
  }
  
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
  struct elf32_state *state = (struct elf32_state *) opaque;

  strncpy (buf, state->abi_string, size);

  return strlen (state->abi_string);
}

void
elf_init (void)
{
  struct loader *elfloader;
  
  debug ("Adding ELF32 support...\n");

  if (PTR_UNLIKELY_TO_FAIL (elfloader = loader_register ("elf32", "Executable and Linking Format (ELF) - 32 bit")))
    FAIL ("Couldn't initialize ELF 32\n");

  elfloader->open     = elf32_open;
  elfloader->entry    = elf32_entry;
  elfloader->walkseg  = elf32_walkseg;
  elfloader->close    = elf32_close;
  elfloader->rebase   = elf32_rebase;
  elfloader->get_abi  = elf32_get_abi;
  elfloader->relocate = elf32_relocate;
  
}

DEBUG_FUNC (elf32_setup_abi);
DEBUG_FUNC (elf32_open);
DEBUG_FUNC (elf32_entry);
DEBUG_FUNC (elf32_walkseg);
DEBUG_FUNC (elf32_rebase);
DEBUG_FUNC (elf32_close);
DEBUG_FUNC (elf32_get_abi);
DEBUG_FUNC (elf_init);
