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
    
#ifndef _UTIL_H
#define _UTIL_H

#include <values.h>

#define _JOIN(x, y) x ## y
#define JOIN(x, y) _JOIN (x, y)

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)


# ifndef __GNUC__
#  define __builtin_expect(a, b)
# endif

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define FAIL(fmt, arg...)                                     \
  do                                                          \
  {                                                           \
    printk ("fail: %s: " fmt, __FUNCTION__, ##arg);           \
    bugcheck ();                                              \
  }                                                           \
  while (0);
  
#define CONSTRUCT_STRUCT(x, __new)                            \
  if ((__new = salloc (sizeof (struct x))) == NULL)           \
    return NULL;                                              \
                                                              \
  memset (__new, 0, sizeof (struct x));
  
#define CONSTRUCTOR_BODY_OF_STRUCT(x)                         \
  struct x *__new;                                            \
                                                              \
  CONSTRUCT_STRUCT (x, __new);                                \
                                                              \
  return __new;

#define MANDATORY(x)                                          \
  if (unlikely (!(x)))                                        \
    FAIL ("mandaroty action failed: \n   "                    \
      STRINGIFY (x) "\n")                                     \

#define LIKELY_TO_FAIL(x)      likely   (FAILED (x))
#define LIKELY_TO_SUCCESS(x)   likely   (SUCCESS (x))

#define UNLIKELY_TO_FAIL(x)    unlikely (FAILED (x))
#define UNLIKELY_TO_SUCCESS(x) unlikely (SUCCESS (x))

#define PTR_LIKELY_TO_FAIL(x)      likely   (FAILED_PTR (x))
#define PTR_LIKELY_TO_SUCCESS(x)   likely   (SUCCESS_PTR (x))

#define PTR_UNLIKELY_TO_FAIL(x)    unlikely   (FAILED_PTR (x))
#define PTR_UNLIKELY_TO_SUCCESS(x) unlikely   (SUCCESS_PTR (x))


#define RETURN_ON_FAILURE(x)                                  \
  if (UNLIKELY_TO_FAIL (x))                                   \
    return KERNEL_ERROR_VALUE
    
#define PTR_RETURN_ON_FAILURE(x)                              \
  if (UNLIKELY_TO_FAIL (x))                                   \
    return KERNEL_INVALID_POINTER

#define RETURN_ON_PTR_FAILURE(x)                              \
  if (PTR_UNLIKELY_TO_FAIL (x))                               \
    return KERNEL_ERROR_VALUE

#define PTR_RETURN_ON_PTR_FAILURE(x)                          \
  if (PTR_UNLIKELY_TO_FAIL (x))                               \
    return KERNEL_INVALID_POINTER

#define IN_BOUNDS(x, higher)                                  \
  unlikely ((x) < 0 || (x) >= (higher))

void printk (const char *, ...);

void kernel_halt (void);
void kernel_pause (void);

#endif /* _UTIL_H */

