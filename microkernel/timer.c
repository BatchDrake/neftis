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

#include <irq/timer.h>
#include <misc/hook.h>
#include <task/task.h>

#include <arch.h>
#include <kctx.h>

/* This file once contained some obscure logic. Now it's gone. */

void
early_timers_init (void)
{
  hw_set_timer_interrupt_freq (HZ);
  
  hw_timer_enable ();
}

DEBUG_FUNC (early_timers_init);

