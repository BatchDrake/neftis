/*
 * seg.S: gdt/ldt low-level operations
 */

/*
 * This file is part of Eulex.
 *
 * Eulex is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Eulex is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Eulex.  If not, see <http://www.gnu.org/licenses/>.
 */
 
.globl x86_get_current_gdt
.globl x86_flush_gdt
.globl x86_refresh_segments

#include <asm/seg.h>
        
x86_get_current_gdt:
    movl 4(%esp), %eax
    sgdt (%eax)
    ret

x86_flush_gdt:
    movl 4(%esp), %eax
    lgdt (%eax)
    
    call x86_refresh_segments
    
    ret

x86_refresh_segments:
  jmpl $GDT_SEGMENT_KERNEL_CODE, $.reload
  
.reload:
  pushl %eax
  movw $GDT_SEGMENT_KERNEL_DATA, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss
  popl %eax
  
  ret
  
  
  
  
