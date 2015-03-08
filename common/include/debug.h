/*
 *    Debug macros and declarations.
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
    
#ifndef _DEBUG_H
#define _DEBUG_H

# ifndef ASM
#  include <util.h>
# endif

#define FUNC 0
#define VAR  1
#define WEAK 2

#define __DEBUG_PREFIX__ "(d) "
#define __WARNING_PREFIX__ "(!) "
#define __ERROR_PREFIX__ "(x) "

# ifndef ASM
struct kernel_symbol
{
  char *name;
  void *addr;
  int   type;
};
# endif /* ASM */

#define _DEBUG_SYMBOL(sym, type)                                     \
  static const struct kernel_symbol __sym_##sym                      \
  __attribute__ ((section ("debugsyms"))) =                          \
  {#sym, (void *) &sym, type}
  
#define DEBUG_SYMBOL(sym, type) _DEBUG_SYMBOL (sym, type)
#define DEBUG_FUNC(sym) _DEBUG_SYMBOL (sym, FUNC)
#define DEBUG_VAR(sym) _DEBUG_SYMBOL (sym, VAR)

# ifndef NDEBUG

#   define debug(fmt, arg...)                                        \
  printk (__DEBUG_PREFIX__ "%s: " fmt, __PRETTY_FUNCTION__, ##arg)
  
#   define warning(fmt, arg...)                                      \
  printk ("\033[1;33m" __WARNING_PREFIX__ "%s: " fmt                 \
  "\033[0m", __PRETTY_FUNCTION__, ##arg)
  
#   define error(fmt, arg...)                                        \
  printk ("\033[1;31m" __ERROR_PREFIX__ "%s: " fmt                   \
  "\033[0m", __PRETTY_FUNCTION__, ##arg)

#   define ASSERT(cond)                                              \
    if (unlikely (!(cond)))                                          \
      FAIL ("assertion \"" STRINGIFY (cond) "\" failed!\n");
      
# else
#    define debug(fmt, arg...)
#    define warning(fmt, arg...)
#    define error(fmt, arg...)
#    define ASSERT(cond)
# endif /* NDEBUG */

#endif /* _DEBUG_H */

