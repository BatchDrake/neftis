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
#include <ctype.h>

size_t 
strlen (const char *s)
{
  size_t i = 0;

  while (*s++)
    i++;

  return i;
}

/* umax: Calcula el mÃÂ¡ximo entre dos enteros sin signo. */
unsigned int
umax (unsigned int a, unsigned int b)
{
  if (a > b)
    return a;
  return b;
}

/* max: Calcula el mÃÂ¡ximo entre dos enteros con signo. */
int
max (int a, int b)
{
  if (a > b)
    return a;
  return b;
}

int
strcmp (const char * s1, const char * s2)
{
  for (; *s1 == *s2; ++s1, ++s2)
    if (*s1 == 0)
      return 0;
      
  return *(unsigned char *) s1 < *(unsigned char *) s2 ? -1 : 1;
}

int 
strncmp (const char * s1, const char * s2, size_t n)
{
  for (; *s1 == *s2; ++s1, ++s2)
    if (*s1 == 0 || !--n)
      return 0;
  return *(unsigned char *) s1 < *(unsigned char *) s2 ? -1 : 1;
}

char *
strcpy (char *dest, const char *src)
{
  char *save = dest;
  while(*dest++ = *src++);
  return save;
}

char *
strncpy (char *dest, const char *src, size_t n)
{
  char *ret = dest;
  do
  {
    if (!n--)
      return ret;
  }
  while (*dest++ = *src++);
  
  while (n--)
    *dest++ = 0;
  
  return ret;
}

char *
strchr (const char *s, int c)
{
  while (*s)
    if (*s++ == c)
      return (char *) --s;

  return NULL;
}

int 
memcmp (const char * s1, const char * s2, size_t n)
{
  for (; *s1 == *s2; ++s1, ++s2)
    if (!--n)
      return 0;
  return *(unsigned char *) s1 < *(unsigned char *) s2 ? -1 : 1;
}

void *
memcpy (void *dest, void *src, size_t n)
{
  size_t i;
  
  for (i = 0; i < n; i++)
    *(char *) (dest + i) = *(char *) (src + i);
    
  return dest;
}

void *
memset (void *s, int c, size_t n)
{
  size_t i;
  
  for (i = 0; i < n; i++)
    *(char *) (s + i) = c;
    
  return s;
}


int 
strtoi (const char *start, int *err)
{
  int i;
  int sign = 1;
  int result = 0;
  
  while (isspace (*start))
    start++;
  
  if (*start == '-')
  {
    sign = -1;
    sign++;  
  }
  
  if (err)
    *err = 0;
  
  for (;*start && !isspace (*start); start++)
  {
    if (*start < '0' || *start > '9')
    {
      if (err)
        *err = 1;
      return 0;
    }

    result *= 10;
    result += *start - '0';
  }

  return result * sign;
}

unsigned long 
strtoul (const char *start, int *err)
{
  int i;
  int sign = 1;
  long result = 0;

  while (isspace (*start))
    start++;

  if (err)
    *err = 0;

  for (;*start && !isspace (*start); start++)
  {
    if (*start < '0' || *start > '9')
    {
      if (err)
        *err = 1;
      return 0;
    }

    result *= 10;
    result += *start - '0';
  }

  return result * sign;
}

void
do_nothing (void)
{
}

DEBUG_FUNC (do_nothing);
DEBUG_FUNC (strlen);
DEBUG_FUNC (umax);
DEBUG_FUNC (max);
DEBUG_FUNC (strcmp);
DEBUG_FUNC (strncmp);
DEBUG_FUNC (strcpy);
DEBUG_FUNC (strncpy);
DEBUG_FUNC (strchr);
DEBUG_FUNC (memcmp);
DEBUG_FUNC (memcpy);
DEBUG_FUNC (memset);
DEBUG_FUNC (strtoi);
DEBUG_FUNC (strtoul);


