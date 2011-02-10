/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) <year>  <name of author>
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
    
#ifndef _DEFINES_H
#define _DEFINES_H

#include <debug.h>
#include <config.h>
#include <values.h>

#define KERNEL_BRAND_NAME "Neftis microkernel 0.1a"

# ifdef __GNUC__
#  define PACKED            __attribute__ ((packed))
#  define ALIGNED(x)        __attribute__ ((aligned (x)))
#  define PACKED_ALIGNED(x) __attribute__ ((packed, aligned (x)))
#  define COMPILER_APPEND   "gcc timestamp: " __TIMESTAMP__
# else
#  error "Unsupported compiler! (only GCC currently supported)"
# endif

#define KERNEL_BOOT_STRING KERNEL_BRAND_NAME " - " COMPILER_APPEND "\n"

# if defined (HAVE_INLINE)
#  define INLINE static inline
# else
#  define INLINE static
# endif /* defined (HAVE_INLINE) */

# if defined (__386__) || defined (__AMD64__)
#  define __PAGE_BITS 12
# endif

# ifndef __PAGE_BITS
#  error "__PAGE_BITS undefined! (unknown CPU type)"
# endif /* __PAGE_BITS */

# define PAGE_SIZE (1 << __PAGE_BITS)
# define PAGE_MASK (PAGE_SIZE - 1)
# define PAGE_OFFSET(page) ((busword_t) (page) & PAGE_MASK)
# define PAGE_START(page)  ((busword_t) (page) & ~PAGE_MASK)
# define PAGE_NO(page) (PAGE_START (page) >> __PAGE_BITS)
# define PAGE_ADDR(num) ((num) << __PAGE_BITS)

# define __UNITS(x, wrdsiz) ((((x) + (wrdsiz - 1)) / wrdsiz))
# define __ALIGN(x, wrdsiz) (__UNITS(x, wrdsiz) * wrdsiz)

#endif /* _DEFINES_H */

