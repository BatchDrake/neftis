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
#include <asm/cpuid.h>

void
cpuid (int code, DWORD *a, DWORD *b, DWORD *c, DWORD *d)
{
  __asm__ __volatile__ ("cpuid" : "=a" (*a),
                                  "=b" (*b),
                                  "=c" (*c),
                                  "=d" (*d)
                                : "0" (code));
}
 
void
cpuid_string (int code, DWORD where[4])
{
  __asm__ __volatile__ ("cpuid" : "=a" (*where),
                                  "=b"(*(where + 1)),
                                  "=c"(*(where + 2)),
                                  "=d"(*(where + 3))
                                : "0"(code));                                
}

int
cpu_has_feature (int feature)
{
  DWORD eax, ebx, ecx, edx;
  
  cpuid (CPUID_GETFEATURES, &eax, &ebx, &ecx, &edx);
  
  if (feature > 31)
  {
    feature -= 32;
    
    return !!(ecx & (1 << feature));
  }
  else
    return !!(edx & (1 << feature));
}


