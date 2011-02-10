/*
 *    Some useful type definitions
 *    Copyright (C) 2010 Gonzalo J. Carracedo <BatchDrake@gmail.com>
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
    
#ifndef _TYPES_H
#define _TYPES_H

#include <ansi.h>
#include <defines.h>

# ifndef ASM
typedef void * physptr_t;

#  ifdef __LP64__
typedef uint64_t memsize_t;  /* Data type to measure memory regions */
typedef uint64_t busword_t;  /* Data type to operate arithmetically with ptrs */
#  else
typedef uint32_t memsize_t;
typedef uint32_t busword_t;
#  endif /* __LP64__ */
# endif /* ASM */

#endif /* _TYPES_H */

