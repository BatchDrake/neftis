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
elf32_translate (struct elf32_state *state, uint32_t ptr)
{
  int i;

  for (i = 0; i < state->header->e_phnum; ++i)
    if (state->phdrs[i].p_type == PT_LOAD &&
	state->phdrs[i].p_vaddr <= ptr &&
	ptr < state->phdrs[i].p_vaddr + state->phdrs[i].p_filesz)
      return (ptr - state->phdrs[i].p_vaddr) + (void *) state->header;

  return NULL;
}

uint32_t
elf32_addr_offset (struct elf32_state *state, uint32_t ptr)
{
  int i;

  for (i = 0; i < state->header->e_phnum; ++i)
    if (state->phdrs[i].p_type == PT_LOAD &&
	state->phdrs[i].p_vaddr <= ptr &&
	ptr < state->phdrs[i].p_vaddr + state->phdrs[i].p_filesz)
      return (ptr - state->phdrs[i].p_vaddr) + state->phdrs[i].p_offset;

  return -1;
}

int
elf32_parse_dyn (struct elf32_state *state, Elf32_Phdr *phdr)
{
  Elf32_Dyn *dyn;
  int i, j, count;
  uint32_t *hash;
  uint32_t  hash_off;
  uint32_t  strtab_off = -1;
  uint32_t  symtab_off = -1;
  uint32_t  rel_off    = -1;
  uint32_t  rela_off   = -1;
  uint32_t  jmprel_off = -1;
  uint32_t  jmprelsz   = -1;
  
  if (OVERFLOW (phdr->p_offset + phdr->p_filesz - 1, state->size))
    return -1;

  dyn   = (Elf32_Dyn *) OFFADDR (state, phdr->p_offset);
  
  count = phdr->p_filesz / sizeof (Elf32_Dyn);

  for (i = 0; i < count; ++i)
    switch (dyn[i].d_tag)
    {
    case DT_RELA:
      if ((rela_off = elf32_addr_offset (state, dyn[i].d_un.d_ptr)) == -1)
	error ("Broken DT_RELA (unmappable %p)\n", dyn[i].d_un.d_ptr);
      break;

    case DT_RELASZ:
      state->rela_count = dyn[i].d_un.d_val / sizeof (Elf32_Rela);
      break;

    case DT_REL:
      if ((rel_off = elf32_addr_offset (state, dyn[i].d_un.d_ptr)) == -1)
	error ("Broken DT_REL (unmappable %p)\n", dyn[i].d_un.d_ptr);
      break;

    case DT_RELSZ:
      state->rel_count = dyn[i].d_un.d_val / sizeof (Elf32_Rel);
      break;

    case DT_PLTREL:
      state->jmprel_type = dyn[i].d_un.d_val;
      break;

    case DT_PLTRELSZ:
      jmprelsz = dyn[i].d_un.d_val;
      break;
      
    case DT_JMPREL:
      if ((jmprel_off = elf32_addr_offset (state, dyn[i].d_un.d_ptr)) == -1)
	error ("Broken DT_JMPREL (unmappable %p)\n", dyn[i].d_un.d_ptr);
      break;

    case DT_SYMTAB:
      if ((symtab_off = elf32_addr_offset (state, dyn[i].d_un.d_ptr)) == -1)
	error ("Broken DT_SYMTAB (unmappable %p)\n", dyn[i].d_un.d_ptr);
      break;

    case DT_STRTAB:
      if ((strtab_off = elf32_addr_offset (state, dyn[i].d_un.d_ptr)) == -1)
	error ("Broken DT_STRTAB (unmappable %p)\n", dyn[i].d_un.d_ptr);
      break;

    case DT_STRSZ:
      state->strtab_size = dyn[i].d_un.d_val;
      break;
      
    case DT_HASH:
      if ((hash_off = elf32_addr_offset (state, dyn[i].d_un.d_ptr)) != -1)
      {
	if (!OVERFLOW (hash_off + sizeof (uint32_t) - 1, state->size))
	  state->symtab_size = hash[1];
	else
	  error ("Broken DT_HASH (unmappable %p)\n", dyn[i].d_un.d_ptr + sizeof (uint32_t));
      }
      else
        error ("Broken DT_HASH (unmappable %p)\n", dyn[i].d_un.d_ptr);

      break;

    case DT_GNU_HASH:
      if ((hash_off = elf32_addr_offset (state, dyn[i].d_un.d_ptr)) != -1)
      {
	hash = (uint32_t *) OFFADDR (state, hash_off);
	
        /* Extremely dangerous. Check whether traversing hash[0] entries is safe */

	if (OVERFLOW (hash_off + sizeof (uint32_t) * 2 - 1, state->size) ||
	    OVERFLOW (hash_off + sizeof (uint32_t) * (4 + hash[2] + hash[0]) - 1, state->size))
	  error ("DT_GNU_HASH broken\n");
	
	else
	{
	  state->symtab_size = 0;
	  state->symtab_first = hash[1] - 1;
	  
	  for (j = 0; j < hash[0]; ++j)
	    if (state->symtab_size < hash[4 + hash[2] + j])
	      state->symtab_size = hash[4 + hash[2] + j];
	  
	  state->symtab_size += 2;
	}
      }
      else
        error ("Cannot translate DT_GNU_HASH address (0x%x)\n", dyn[i].d_un.d_ptr);

      break;

    }

  if (strtab_off == -1 || symtab_off == -1)
  {
    error ("No symtab, refusing to load\n");

    return -1;
  }

  if (OVERFLOW (symtab_off + state->symtab_size * sizeof (Elf32_Sym) - 1, state->size))
  {
    error ("DT_SYMTAB overflow\n");
    return -1;
  }

  if (OVERFLOW (strtab_off + state->strtab_size - 1, state->size))
  {
    error ("DT_STRTAB overflow\n");
    return -1;
  }

  state->symtab = (Elf32_Sym *) OFFADDR (state, symtab_off);
  state->strtab = (char *) OFFADDR (state, strtab_off);
  
  if (rel_off != -1)
  {
    if (OVERFLOW (rel_off + state->rel_count * sizeof (Elf32_Rel), state->size))
    {
      error ("DT_REL overflow\n");
      return -1;
    }

    state->rel = (Elf32_Rel *) OFFADDR (state, rel_off);
  }
  else
  {
    state->rel = NULL;
    state->rel_count = 0;
  }
  
  if (rela_off != -1)
  {
    if (OVERFLOW (rela_off + state->rela_count * sizeof (Elf32_Rela), state->size))
    {
      error ("DT_RELA overflow\n");
      return -1;
    }

    state->rela = (Elf32_Rela *) OFFADDR (state, rela_off);
  }
  else
  {
    state->rela = NULL;
    state->rela_count = 0;
  }

  if (jmprel_off != -1)
  {
    if (jmprelsz == -1)
    {
      error ("No DT_PLTRELSZ found\n");
      return -1;
    }

    if (state->jmprel_type == DT_REL)
      state->jmprel_count = jmprelsz / sizeof (Elf32_Rel);
    else if (state->jmprel_type == DT_RELA)
      state->jmprel_count = jmprelsz / sizeof (Elf32_Rela);
    else
    {
      error ("Unsupported DT_JMPREL format\n");
      return -1;
    }
    
    if ((state->jmprel_type == DT_REL && OVERFLOW (jmprel_off + state->jmprel_count * sizeof (Elf32_Rel), state->size)) ||
	(state->jmprel_type == DT_RELA && OVERFLOW (jmprel_off + state->jmprel_count * sizeof (Elf32_Rela), state->size)))
    {
      error ("DT_JMPREL overflow\n");
      return 1;
    }

    state->jmprel = (Elf32_Rel *) OFFADDR (state, jmprel_off);
  }
  else
  {
    state->jmprel       = NULL;
    state->jmprel_count = 0;
  }
  
  return 0;
}

static int
elf32_reloc (struct elf32_state *state, struct vm_space *space, uint32_t target, int type, uint32_t S, uint32_t A, uint32_t G, uint32_t L, uint32_t B, uint32_t GOT)
{
  uint32_t P = target;
  uint32_t R;
  
  switch (type)
  {
  case R_386_NONE:
    break;
    
  case R_386_32:
    R = S + A;
    break;
    
  case R_386_PC32:
    R = S + A - P;
    break;

  case R_386_GLOB_DAT:
  case R_386_JMP_SLOT:
    R = S;
    break;
    
  case R_386_RELATIVE:
    R = B + A;
    break;

  default:
    error ("Unsupported relocation type %d\n", type);
    return -1;
    
    break;
  }
  
  return copy2virt (space, target, &R, sizeof (uint32_t)) != sizeof (uint32_t) ? -1 : 0;
}

static int
elf32_relocation_requires_symbol (struct elf32_state *state, int type)
{
  return
    type == R_386_32 ||
    type == R_386_PC32 ||
    type == R_386_GLOB_DAT ||
    type == R_386_JMP_SLOT ||
    type == R_386_GOTOFF;
}

int
elf32_parse_rel (struct elf32_state *state, struct vm_space *space, Elf32_Rel *rel, uint32_t count)
{
  int i, symno;
  char *sym_name;
  Elf32_Sym *sym;
  uint32_t P, A, S = 0;
  int type;
  uint32_t sym_val;
  
  for (i = 0; i < count; ++i)
  {
    symno = ELF32_R_SYM  (rel[i].r_info);
    type  = ELF32_R_TYPE (rel[i].r_info);

    if (state->dyn)
      P   = state->addr + rel[i].r_offset;
    else
      P   = rel[i].r_offset;

    if (elf32_relocation_requires_symbol (state, type))
    {
      if (OVERFLOW (symno, state->symtab_size))
	sym_name = "<symbol out of bounds>";
      else
      {
	sym = &state->symtab[symno];
	
	if (OVERFLOW (sym->st_name, state->strtab_size))
	  sym_name = "<symbol name out of bounds>";
	else
	  sym_name = state->strtab + sym->st_name;
      }
      
      if (copy2phys (space, &A, P, sizeof (uint32_t)) != sizeof (uint32_t))
      {
	error ("Broken relocation on unmappable %p\n", P);
	return -1;
      }
      
      if (sym->st_shndx == SHN_UNDEF)
      {
	error ("Unresolved external in ABI VDSO: %s\n", sym_name);
	error ("ABI VDSO cannot have external dependencies!\n");
	return -1;
      }

      S = sym->st_value;

      if (state->dyn)
	if (1 <= sym->st_shndx && sym->st_shndx < state->header->e_shnum)
	  S += state->addr;
    }
    else
      S = -1;
    
    if (elf32_reloc (state, space, P, type, S, A, 0, 0, state->addr, 0) == -1)
      return -1;
  }
}

int
elf32_parse_rela (struct elf32_state *state, struct vm_space *space, Elf32_Rela *rela, uint32_t count)
{
  int i, symno;
  char *sym_name;
  Elf32_Sym *sym;
  uint32_t P, A, S = 0;
  int type;
  uint32_t sym_val;
  
  for (i = 0; i < count; ++i)
  {
    symno = ELF32_R_SYM  (rela[i].r_info);
    type  = ELF32_R_TYPE (rela[i].r_info);
    A     = rela[i].r_addend;
    
    if (state->dyn)
      P   = state->addr + rela[i].r_offset;
    else
      P   = rela[i].r_offset;

    if (elf32_relocation_requires_symbol (state, type))
    {  
      if (OVERFLOW (symno, state->symtab_size))
	sym_name = "<symbol out of bounds>";
      else
      {
	sym = &state->symtab[symno];
	
	if (OVERFLOW (sym->st_name, state->strtab_size))
	  sym_name = "<symbol name out of bounds>";
	else
	  sym_name = state->strtab + sym->st_name;
      }
      
      if (sym->st_shndx == SHN_UNDEF)
      {
	error ("Unresolved external in ABI VDSO: %s\n", sym_name);
	error ("ABI VDSO cannot have external dependencies!\n");
	return -1;
      }
      
      S = sym->st_value;

      if (state->dyn)
	if (1 <= sym->st_shndx && sym->st_shndx < state->header->e_shnum)
	  S += state->addr;
    }
    else
      S = -1;
    
    if (elf32_reloc (state, space, P, type, S, A, 0, 0, state->addr, 0) == -1)
      return -1;
  }
}

DEBUG_FUNC (elf32_translate);
DEBUG_FUNC (elf32_addr_offset);
DEBUG_FUNC (elf32_parse_dyn);
DEBUG_FUNC (elf32_reloc);
DEBUG_FUNC (elf32_relocation_requires_symbol);
DEBUG_FUNC (elf32_parse_rel);
DEBUG_FUNC (elf32_parse_rela);
