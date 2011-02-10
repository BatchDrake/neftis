/*
 *    ANSI C types and definitions
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
    
#ifndef _ANSI_H
#define _ANSI_H

# ifndef ASM
typedef signed char             int8_t;
typedef unsigned char           uint8_t;
typedef signed short int        int16_t;
typedef unsigned short int      uint16_t;
typedef signed int              int32_t;
typedef unsigned int            uint32_t;
typedef unsigned long long      uint64_t;
typedef long long               int64_t;

/* Windows-style low-level data types */
typedef unsigned char           BYTE;
typedef unsigned short          WORD;
typedef unsigned int            DWORD;
typedef uint64_t                QWORD;

typedef unsigned int            size_t;
# define NULL ((void *) 0)
# endif /* ASM */

#endif /* _ANSI_H */

