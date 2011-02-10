/*
 * stdarg.h: implementación de las funciones de manejo de listas
 * de argumentos variables. Debería reescribirse usando las
 * funciones propias del compilador.
 */

/*
 * This file is part of Eulex.
 *
 * Eulex is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Eulex is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Eulex.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef _STDARG_H
# define _STDARG_H

# ifdef __GNUC__

typedef __builtin_va_list va_list;

#   define va_start(v,l)   __builtin_va_start(v,l)
#   define va_end(v)       __builtin_va_end(v)    
#   define va_arg(v,l)     __builtin_va_arg(v,l)

# else

typedef struct _vararg_list
{
  void *last;
  void *ptr;
}
va_list;

#   ifndef __ALIGN
#   define __ALIGN(x, wrdsiz) (((sizeof (x) + (wrdsiz - 1)) / wrdsiz) * wrdsiz)
#   endif

/* Esta implementaciÃÂ³n es patatera. Mejorar en un futuro. */
#   define va_start(ap, last) ap = (struct _vararg_list) {(void *) &last, (void *) &last}
#   define va_arg(ap, type) (* ((type *) (ap.last += __ALIGN (type, 4))))
# endif /* __GNUC__ */

#endif /* _STDARG_H */
