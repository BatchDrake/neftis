/*
 *    Common allocator used for small structures, objects and datatypes
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

#ifndef _ALLOC_H
#define _ALLOC_H

#include <types.h>

/* Most likely to be implemented as salloc, implemented as malloc in (future) unit testing */
void *kalloc (size_t);
void  kfree (void *);

void *__kalloc (size_t);
void  __kfree (void *);

#endif /* _ALLOC_H */

