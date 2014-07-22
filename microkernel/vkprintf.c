/*
 *    vkprintf.c: Formatted output.
 *    Copyright (C) 2014  Gonzalo J. Carracedo <BatchDrake@gmail.com>
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
#include <string.h>
#include <stdarg.h>
#include <misc/vkprintf.h>

static void
print_decimal (struct vkprintf_stream *stream, int n)
{
  int copy;
  register int i;
  char nbuf[30];
  
  copy = n;
  if (copy < 0)
    copy = -copy;
  
  for (i = 0; copy; i++)
  {
    nbuf[i] = '0' + copy % 10;
    copy /= 10;
  }
  
  if (n < 0)
    vkputchar (stream, '-');
  
  if (!i)
    vkputchar (stream, '0');
  else
    for (i--; i >= 0; i--)
      vkputchar (stream, nbuf[i]);
}

static void
print_octal (struct vkprintf_stream *stream, unsigned int n)
{
  int copy;
  register int i;
  char nbuf[30];

  copy = n;
  if (copy < 0)
    copy = -copy;

  for (i = 0; copy; i++)
  {
    nbuf[i] = '0' + copy % 8;
    copy /= 8;
  }
  
  if (!i)
    vkputchar (stream, '0');
  else
    for (i--; i >= 0; i--)
      vkputchar (stream, nbuf[i]);
}

static void 
print_hex (struct vkprintf_stream *stream, unsigned int n, int upper)
{
  int copy;
  register int i;
  char nbuf[30];
  char hexachars_upper[] = "0123456789ABCDEF";
  char hexachars_lower[] = "0123456789abcdef";
  
  for (i = 0; n; i++)
  {
    nbuf[i] = upper ? hexachars_upper [n % 16] : hexachars_lower [n % 16]; 
    n /= 16;
  }

  if (!i)
    vkputchar (stream, '0');
  else
    for (i--; i >= 0; i--)
      vkputchar (stream, nbuf[i]);
}

static void
print_DWORD (struct vkprintf_stream *stream, uint32_t num, int upper, int sep)
{
  char hexachars_upper[] = "0123456789ABCDEF";
  char hexachars_lower[] = "0123456789abcdef";
  
  register int i;
  
  for (i = 7; i >= 0; i--)
  {
    if (i == 3 && sep)
      vkputchar (stream, ':');
    vkputchar (stream, upper ? hexachars_upper[(num & 0xf << i * 4) >> i * 4] :
              hexachars_lower[(num & 0xf << i * 4) >> i * 4]);
  }
}

static void
print_byte (struct vkprintf_stream *stream, uint32_t num, int upper)
{
  char hexachars_upper[] = "0123456789ABCDEF";
  char hexachars_lower[] = "0123456789abcdef";
  
  vkputchar (stream, upper ? hexachars_upper[(num & 0xf << 4) >> 4] :
              hexachars_lower[(num & 0xf << 4) >> 4]);
  vkputchar (stream, upper ? hexachars_upper[(num & 0xf)] :
              hexachars_lower[(num & 0xf)]);
}

static void
print_human_readable (struct vkprintf_stream *stream, DWORD amount, int full)
{
  char *units[] = {"bytes", "KiB", "MiB", "GiB", "TiB",
                   "PiB", "EiB", "ZiB", "YiB"};
  
  int divider = 0;
  
  while (amount >= 1024)
  {
    amount >>= 10;
    divider++;
  }

  print_decimal (stream, amount);
  
  if (full)
  {
    vkputs (stream, " ");
    vkputs (stream, units[divider]);
  }
  else
    vkputchar (stream, *units[divider]);
}


static void
print_eflags (struct vkprintf_stream *stream, DWORD eflags)
{
  #define EFLAG(x) (eflags & (1 << (x)))
  #define PRINT_EFLAG(b, l)        \
    if (EFLAG (b))                 \
      vkputchar (stream, l);   \
    else                           \
      vkputchar (stream, '-'); \
  
  PRINT_EFLAG (0,  'C'); /* Acarreo */
  PRINT_EFLAG (2,  'P'); /* Paridad */
  PRINT_EFLAG (4,  'A'); /* Ajuste */
  PRINT_EFLAG (6,  'Z'); /* Cero */
  PRINT_EFLAG (7,  'S'); /* Signo */
  PRINT_EFLAG (8,  'T'); /* Trap (paso a paso) */
  PRINT_EFLAG (9,  'I'); /* Interrupciones activadas */
  PRINT_EFLAG (10, 'D'); /* Flag de dirección */
  PRINT_EFLAG (11, 'O'); /* Desbordamiento */
  PRINT_EFLAG (14, 'N'); /* Tarea anidada */
  PRINT_EFLAG (16, 'R'); /* Flag de continuación */
  PRINT_EFLAG (17, 'V'); /* Modo vm8086 */
  PRINT_EFLAG (18, 'a'); /* Comprobación de alineamiento */
  PRINT_EFLAG (19, 'i'); /* Interr. virtual */
  PRINT_EFLAG (20, 'p'); /* Intr. virt. pendiente */
  PRINT_EFLAG (21, 'd'); /* Identificación */
  
  #undef PRINT_FLAG
  #undef EFLAG
}

void
vkprintf (struct vkprintf_stream *stream, const char *fmt, va_list ap)
{
  int len;
  register int i;
  void *ptr;
  
  len = strlen (fmt);
  
  i = 0;
  
  for (i = 0; i < len; i++)
  {
    if (fmt[i] == '%')
    {
      i++;
      
      switch (fmt[i])
      {
        case 'd':
          print_decimal (stream, va_arg (ap, int));
          break;
        
        case 'h':
          print_human_readable (stream, va_arg (ap, DWORD), 0);
          break;
        
        case 'H':
          print_human_readable (stream, va_arg (ap, DWORD), 1);
          break;
          
        case 'b':
          print_byte (stream, va_arg (ap, unsigned int), 0);
          break;
          
        case 'B':
          print_byte (stream, va_arg (ap, unsigned int), 1);
          break;
          
        case 'x':
          print_hex (stream, va_arg (ap, unsigned int), 0);
          break;

        case 'o':
          print_octal (stream, va_arg (ap, unsigned int));
          break;
        
        case 'X':
          print_hex (stream, va_arg (ap, unsigned int), 1);
          break;
        
        case 'w':
          print_DWORD (stream, va_arg (ap, uint32_t), 0, 1);
          break;
        
        case 'W':
          print_DWORD (stream, va_arg (ap, uint32_t), 1, 1);
          break;
        
        case 'y':
          print_DWORD (stream, va_arg (ap, uint32_t), 0, 0);
          break;
        
        case 'Y':
          print_DWORD (stream, va_arg (ap, uint32_t), 1, 0);
          break;
          
        case 'c':
          console_putchar (stream, va_arg (ap, unsigned int));
          break;
        
        case 'C':
          print_eflags (stream, va_arg (ap, DWORD));
          break;
          
        case 'p':
          ptr = va_arg (ap, void *);
          if (ptr == NULL)
            vkputs (stream, "(null)");
          else
          {
            vkputs (stream, "0x");
            print_hex (stream, (unsigned int) ptr, 0);
          }
          
          break;
          
        case 's':
          vkputs (stream, va_arg (ap, char *));
          break;
        
        case '%':
          vkputchar (stream, '%');
          break;
          
        case '\0':
          return;
      }
    }
    else
      vkputchar (stream, fmt[i]);
  }
}

void
vkputchar (struct vkprintf_stream *stream, char c)
{
  stream->counter += (stream->putchar) (stream, c) == 0;
}

void
vkputs (struct vkprintf_stream *stream, const char *s)
{
  if (stream->puts != NULL)
    stream->counter += (stream->puts) (stream, s);
  else
  {
    while (*s)
      stream->counter += (stream->putchar) (stream, *s++) == 0;
  }
}

