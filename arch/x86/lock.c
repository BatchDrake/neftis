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
 
#include <lock/lock.h>
#include <asm/regs.h>

/* Proberen */
void spin_lock (spin_t *lock);

/* Shamelessly extracted from Linux implementation */
#define spin_lock_string \
        "1:\n" \
       "lock\n" \
       "decb %0\n\t" \
       "jns 3f\n" \
       "2:\t" \
       "rep;nop\n\t" \
       "cmpb $0,%0\n\t" \
       "jle 2b\n\t" \
       "jmp 1b\n" \
	     "3:\n\t"
	     
#define spin_unlock_string \
       "xchgb %b0, %1" \
               :"=q" (oldval), "=m" (*lock) \
               :"0" (oldval) : "memory"
	
void
spin_lock (spin_t *lock)
{
  #if NR_MAXCPUS > 1
  __asm__ __volatile__ (spin_lock_string : "=m" (*lock) : : "memory");
  #endif
  
}


/* Verhogen. Esto ni siquiera es crítico. */
void
spin_unlock (spin_t *lock)
{
  char oldval = 1;
  
  #if NR_MAXCPUS > 1
  __asm__ __volatile__ (spin_unlock_string);
  #endif
}

int
critical_is_inside (busword_t flags)
{
  return flags & EFLAGS_INTERRUPT;
}

/* This must be a per-CPU variable. */
void
critical_enter (spin_t *lock, busword_t *flags)
{
#if NR_MAXCPUS > 1
#error Multi-CPU configuration not supported (yet)
#else
  /* Save processor state */
  __asm__ __volatile__ ("pushf\n"
                        "popl %0" : "=g" (*flags));

  __asm__ __volatile__ ("cli");

  spin_lock (lock);
  

#endif
}

void
critical_leave (spin_t *lock, busword_t flags)
{
#if NR_MAXCPUS > 1
#error Multi-CPU configuration not supported (yet)
#else
  spin_lock (lock);

  /* Restore processor state */
  __asm__ __volatile__ ("pushl %0\n"
                        "popf" :: "g" (flags));

#endif  
}

void
critical_abort (void)
{
#if NR_MAXCPUS > 1
#error Multi-CPU configuration not supported (yet)
#else

  /* Restore processor state */
  __asm__ __volatile__ ("sti");

#endif  
}
