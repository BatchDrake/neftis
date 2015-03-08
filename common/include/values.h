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
    
#ifndef _VALUES_H
#define _VALUES_H

#define KERNEL_SUCCESS_VALUE    0
#define KERNEL_ERROR_VALUE      -1

#define KERNEL_INVALID_POINTER ((void *) 0)

#define FAILED(x)      ((x) == KERNEL_ERROR_VALUE)
#define SUCCESS(x)     ((x) == KERNEL_SUCCESS_VALUE)

#define FAILED_PTR(x)  ((x) == KERNEL_INVALID_POINTER)
#define SUCCESS_PTR(x) ((x) != KERNEL_INVALID_POINTER)


#endif /* _VALUES_H */

