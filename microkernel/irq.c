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
#include <misc/hook.h>
#include <irq/irq.h>

struct hook_bucket *sys_irq_bucket;

void
irq_interface_init (void)
{
  MANDATORY (PTR_LIKELY_TO_SUCCESS (
    sys_irq_bucket = hook_bucket_new (SYS_MAX_IRQS)
    )
  );
  
  debug ("irq bucket with room for %d IRQs\n", SYS_MAX_IRQS);
}

