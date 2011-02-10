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
    
#ifndef _IRQ_IRQ_H
#define _IRQ_IRQ_H

#include <types.h>
#include <misc/hook.h>

#define SYS_MAX_IRQS 24

INLINE int
do_irq_from_interrupt (int irq, void *frame)
{
  extern struct hook_bucket *sys_irq_bucket;
  
  return trigger_hook (sys_irq_bucket, irq, frame);
}

void irq_interface_init (void);

#endif /* _IRQ_IRQ_H */

