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

#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>
#include <malloc.h>

size_t strlen (const char *);
unsigned int umax (unsigned int, unsigned int);
int max (int, int);
int strcmp (const char *, const char *);
int strncmp (const char *, const char *, size_t);
char *strcpy (char *, const char *);
char *strncpy (char *, const char *, size_t);
char *strchr (const char *, int);
int memcmp (const void *, const void *, size_t);
void *memcpy (void *, const void *, size_t);
void *memset (void *, int, size_t);
int  strtoi (const char *, int *);
unsigned long strtoul (const char *, int *);
void ultostr (unsigned long, char *, size_t);
void abort (void);
void *sbrk (ptrdiff_t);
#endif /* _STDLIB_H */
