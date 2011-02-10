/*
 *    Standard string functions
 *    Copyright (C) 2010 Gonzalo J. Carracedo <BatchDrake@gmail.com>
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
    
#ifndef _STRING_H
#define _STRING_H

#include <types.h>

size_t strlen (const char *);

int strcmp (const char *, const char *);

int strncmp (const char *, const char *, size_t);

char *strcpy (char *, const char *);

char *strncpy (char *, const char *, size_t);

char *strchr (const char *, int);

int memcmp (const char *, const char *, size_t);

void *memcpy (void *, void *, size_t);

void *memset (void *, int, size_t);

#endif /* _STRING_H */

