/*
 *    Anonymous mappings.
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

#ifndef _MM_ANON_H
#define _MM_ANON_H

struct vm_region *vm_region_anonmap (busword_t, busword_t, DWORD);
struct vm_region *vm_region_remap (busword_t, busword_t, busword_t, DWORD);
struct vm_region *vm_region_physmap (busword_t, busword_t, DWORD);

struct vm_region *vm_region_stack (busword_t, busword_t);
struct vm_region *vm_region_kernel_stack (busword_t);
struct vm_region *vm_region_iomap (busword_t, busword_t, busword_t);

#endif /* _MM_ANON_H */
