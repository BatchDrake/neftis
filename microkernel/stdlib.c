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

#include <mm/salloc.h>
#include <misc/tar.h>

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
memcmp (const void * s1, const void * s2, size_t n)
{
  for (; *(unsigned char *) s1 == *(unsigned char *) s2; ++s1, ++s2)
    if (!--n)
      return 0;
  return *(unsigned char *) s1 < *(unsigned char *) s2 ? -1 : 1;
}

void *
memcpy (void *dest, const void *src, size_t n)
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

char *
strdup (const char *str)
{
  char *copy;
  size_t len = strlen (copy);

  if ((copy = salloc_irq (len + 1)) == NULL)
    return NULL;

  memcpy (copy, str, len + 1);

  return copy;
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
ultostr (unsigned long num, char *buf, size_t max)
{
  unsigned long cover = 1;
  int size = 1;
  int i;
  
  while (num / cover >= 10)
  {
    cover *= 10;
    ++size;
  }

  if (size + 1 > max)
    size = max - 1;

  if (max)
    buf[size] = '\0';
  
  for (i = size - 1; i >= 0; --i)
  {
    buf[i] = (num % 10) + '0';
    num /= 10;
  }  
}

void
do_nothing (void)
{
}

int
tar_file_walk (const void *base, uint32_t size, const char *path_prefix, int (*cb) (const char *path, const void *base, uint32_t size, uint32_t mode, void *opaque), void *opaque)
{
  uint32_t p = 0;
  char c;
  struct posix_header *hdr;
  uint32_t fsize;
  uint32_t fact;
  uint32_t mode;
  int i;

  while (p + TAR_BLKSIZE <= size)
  {
    hdr = (struct posix_header *) (base + p);

    if (!hdr->name[0])
      break;
    
    p += TAR_BLKSIZE;
    
    fsize = 0;
    fact  = 1;
    mode  = 0;

    for (i = 0; i < 11; ++i)
    {
      if (!isodigit (c = hdr->size[10 - i]))
      {
	error ("offset 0x%x: corrupt header (%s)\n", p, hdr->size);
	return -1;
      }
      else
      {
	fsize += (c - '0') * fact;
	fact <<= 3;
      }
    }

    fact = 1;
    
    for (i = 0; i < 7; ++i)
    {
      if (!isodigit (c = hdr->mode[6 - i]))
      {
	error ("offset 0x%x: corrupt header\n", p);
	return -1;
      }
      else
      {
	mode += (c - '0') * fact;
	fact <<= 3;
      }
    }

    if ((hdr->typeflag == 0 || hdr->typeflag == '0') &&
	strncmp (hdr->name, path_prefix, strlen (path_prefix)) == 0)
      if ((cb) (hdr->name, base + p, fsize, mode, opaque) == -1)
	return -1;
    
    p += ((fsize / TAR_BLKSIZE) + !!(fsize & (TAR_BLKSIZE - 1))) * TAR_BLKSIZE;
  }

  return 0;
}

struct found_file
{
  const char *file;
  const void *base;
  uint32_t size;
};

static int
__file_lookup (const char *name, const void *base, uint32_t size, uint32_t mode, void *opaque)
{
  struct found_file *ff = (struct found_file *) opaque;

  if (ff->base == NULL && strcmp (name, ff->file) == 0)
  {
    ff->base = base;
    ff->size = size;
    return -1;
  }

  return 0;
}

int
tar_file_lookup (const void *base, uint32_t size, const char *file, const void ** pbase, uint32_t *psize)
{
  struct found_file ff = {file, NULL, 0};

  /*
    const void *base, uint32_t size, const char *path_prefix, int (*cb) (const char *path, const void *base, uint32_t size, uint32_t mode, void *opaque), void *opaque */
  
  if (tar_file_walk (base, size, file, __file_lookup, &ff) == 0)
    return -1;

  if (ff.base == NULL)
    return -1;

  *pbase = ff.base;
  *psize = ff.size;

  return 0;
}

DEBUG_FUNC (tar_file_walk);
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
DEBUG_FUNC (ultostr);
DEBUG_FUNC (do_nothing);
