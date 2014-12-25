/*
 *    The ELF loader for Atomik microkernel - opaque state
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

#ifndef _ELF_STATE_H
#define _ELF_STATE_H

#define OFFADDR(state, off) ((void *) (state)->header + (off))

struct elf32_state
{
  Elf32_Ehdr *header;
  Elf32_Phdr *phdrs;
  Elf32_Shdr *shdrs;

  Elf32_Rel  *rel;
  uint32_t    rel_count;
  
  Elf32_Rela *rela;
  uint32_t    rela_count;

  int jmprel_type;
  
  union
  {
    Elf32_Rel  *jmprel;
    Elf32_Rela *jmprela;
  };
  
  uint32_t    jmprel_count;

  
  Elf32_Sym  *symtab;
  uint32_t    symtab_size;
  uint32_t    symtab_first;
  
  char       *strtab;
  uint32_t    strtab_size;
 
  uint32_t    size;
  int         dyn;
  uint32_t    addr;

  const char *abi_string; /* Static string */
};

struct elf32_note
{
  uint32_t namesz;
  uint32_t descsz;
  uint32_t type;
  char     data[0];
};

int elf32_parse_dyn (struct elf32_state *, Elf32_Phdr *);
int elf32_setup_abi (struct elf32_state *, Elf32_Phdr *);
int elf32_parse_rel (struct elf32_state *, struct vm_space *, Elf32_Rel *, uint32_t);
int elf32_parse_rela (struct elf32_state *, struct vm_space *, Elf32_Rela *, uint32_t);

#endif /* _ELF_STATE_H */

