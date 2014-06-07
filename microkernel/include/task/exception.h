/*
 *    Basic exception handling.
 *    Copyright (c) 2014 Gonzalo J. Carracedo
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

#ifndef _TASK_EXCEPTION_H
#define _TASK_EXCEPTION_H

#define EX_MAX               4

#define EX_FPE               0
#define EX_SEGMENT_VIOLATION 1
#define EX_PRIV_INSTRUCTION  2
#define EX_ILL_INSTRUCTION   3

void task_set_exception_handler (struct task *task, int exception, void (*handler) (struct task *, int, busword_t, busword_t, int));
void task_trigger_exception (struct task *task, int exception, busword_t textaddr, busword_t data, int code);

#endif /* _TASK_EXCEPTION_H */
