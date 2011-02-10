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
 
#include <types.h>
#include <string.h>
#include <stdarg.h>

#include <console/console.h>


extern struct console *syscon;

static void
print_decimal (struct console *cons, int n)
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
    console_putchar (cons, '-');
  
  if (!i)
    console_putchar (cons, '0');
  else
    for (i--; i >= 0; i--)
      console_putchar (cons, nbuf[i]);
}


static void
print_octal (struct console *cons, unsigned int n)
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
    console_putchar (cons, '0');
  else
    for (i--; i >= 0; i--)
      console_putchar (cons, nbuf[i]);
}

static void 
print_hex (struct console *cons, unsigned int n, int upper)
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
    console_putchar (cons, '0');
  else
    for (i--; i >= 0; i--)
      console_putchar (cons, nbuf[i]);
}

static void
print_DWORD (struct console *cons, uint32_t num, int upper, int sep)
{
  char hexachars_upper[] = "0123456789ABCDEF";
  char hexachars_lower[] = "0123456789abcdef";
  
  register int i;
  
  for (i = 7; i >= 0; i--)
  {
    if (i == 3 && sep)
      console_putchar (cons, ':');
    console_putchar (cons, upper ? hexachars_upper[(num & 0xf << i * 4) >> i * 4] :
              hexachars_lower[(num & 0xf << i * 4) >> i * 4]);
  }
}


static void
print_byte (struct console *cons, uint32_t num, int upper)
{
  char hexachars_upper[] = "0123456789ABCDEF";
  char hexachars_lower[] = "0123456789abcdef";
  
  console_putchar (cons, upper ? hexachars_upper[(num & 0xf << 4) >> 4] :
              hexachars_lower[(num & 0xf << 4) >> 4]);
  console_putchar (cons, upper ? hexachars_upper[(num & 0xf)] :
              hexachars_lower[(num & 0xf)]);
}

static void
print_human_readable (struct console *cons, DWORD amount, int full)
{
  char *units[] = {"bytes", "KiB", "MiB", "GiB", "TiB",
                   "PiB", "EiB", "ZiB", "YiB"};
  
  int divider = 0;
  
  while (amount >= 1024)
  {
    amount >>= 10;
    divider++;
  }

  print_decimal (cons, amount);
  
  if (full)
  {
    console_puts (cons, " ");
    console_puts (cons, units[divider]);
  }
  else
    console_putchar (cons, *units[divider]);
}

static void
print_eflags (struct console *cons, DWORD eflags)
{
  #define EFLAG(x) (eflags & (1 << (x)))
  #define PRINT_EFLAG(b, l)        \
    if (EFLAG (b))                 \
      console_putchar (cons, l);   \
    else                           \
      console_putchar (cons, '-'); \
  
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
console_vprintf (struct console *cons, const char *fmt, va_list ap)
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
          print_decimal (cons, va_arg (ap, int));
          break;
        
        case 'h':
          print_human_readable (cons, va_arg (ap, DWORD), 0);
          break;
        
        case 'H':
          print_human_readable (cons, va_arg (ap, DWORD), 1);
          break;
          
        case 'b':
          print_byte (cons, va_arg (ap, unsigned int), 0);
          break;
          
        case 'B':
          print_byte (cons, va_arg (ap, unsigned int), 1);
          break;
          
        case 'x':
          print_hex (cons, va_arg (ap, unsigned int), 0);
          break;

        case 'o':
          print_octal (cons, va_arg (ap, unsigned int));
          break;
        
        case 'X':
          print_hex (cons, va_arg (ap, unsigned int), 1);
          break;
        
        case 'w':
          print_DWORD (cons, va_arg (ap, uint32_t), 0, 1);
          break;
        
        case 'W':
          print_DWORD (cons, va_arg (ap, uint32_t), 1, 1);
          break;
        
        case 'y':
          print_DWORD (cons, va_arg (ap, uint32_t), 0, 0);
          break;
        
        case 'Y':
          print_DWORD (cons, va_arg (ap, uint32_t), 1, 0);
          break;
          
        case 'c':
          console_putchar (cons, va_arg (ap, unsigned int));
          break;
        
        case 'C':
          print_eflags (cons, va_arg (ap, DWORD));
          break;
          
        case 'p':
          ptr = va_arg (ap, void *);
          if (ptr == NULL)
            console_puts (cons, "(null)");
          else
          {
            console_puts (cons, "0x");
            print_hex (cons, (unsigned int) ptr, 0);
          }
          
          break;
          
        case 's':
          console_puts (cons, va_arg (ap, char *));
          break;
        
        case '%':
          console_putchar (cons, '%');
          break;
          
        case '\0':
          return;
      }
    }
    else
      console_putchar (cons, fmt[i]);
  }
}


void
putchar (char c)
{
  if (syscon)
    console_putchar (syscon, c);
}

void
puts (const char* s)
{
  if (syscon)
    console_puts (syscon, s);
}

void
printk (const char *fmt, ...)
{
  va_list ap;
  int i;
  
  va_start (ap, fmt);
  
  if (syscon)
    console_vprintf (syscon, fmt, ap);
  
}

