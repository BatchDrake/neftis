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
    
#ifndef _ASM_CPUID_H
#define _ASM_CPUID_H

/* Extracted from http://wiki.osdev.org/CPUID */
enum cpuid_features
{
  CPU_FEATURE_SSE3             = 0 + 32, 
  CPU_FEATURE_PCLMUL           = 1 + 32,
  CPU_FEATURE_DTES64           = 2 + 32,
  CPU_FEATURE_MONITOR          = 3 + 32,  
  CPU_FEATURE_DS_CPL           = 4 + 32,  
  CPU_FEATURE_VMX              = 5 + 32,  
  CPU_FEATURE_SMX              = 6 + 32,  
  CPU_FEATURE_EST              = 7 + 32,  
  CPU_FEATURE_TM2              = 8 + 32,  
  CPU_FEATURE_SSSE3            = 9 + 32,  
  CPU_FEATURE_CID              = 10 + 32,
  CPU_FEATURE_FMA              = 12 + 32,
  CPU_FEATURE_CX16             = 13 + 32, 
  CPU_FEATURE_ETPRD            = 14 + 32, 
  CPU_FEATURE_PDCM             = 15 + 32, 
  CPU_FEATURE_DCA              = 18 + 32, 
  CPU_FEATURE_SSE4_1           = 19 + 32, 
  CPU_FEATURE_SSE4_2           = 20 + 32, 
  CPU_FEATURE_x2APIC           = 21 + 32, 
  CPU_FEATURE_MOVBE            = 22 + 32, 
  CPU_FEATURE_POPCNT           = 23 + 32, 
  CPU_FEATURE_XSAVE            = 26 + 32, 
  CPU_FEATURE_OSXSAVE          = 27 + 32, 
  CPU_FEATURE_AVX              = 28 + 32,

  CPU_FEATURE_FPU              = 0,  
  CPU_FEATURE_VME              = 1,  
  CPU_FEATURE_DE               = 2,  
  CPU_FEATURE_PSE              = 3,  
  CPU_FEATURE_TSC              = 4,  
  CPU_FEATURE_MSR              = 5,  
  CPU_FEATURE_PAE              = 6,  
  CPU_FEATURE_MCE              = 7,  
  CPU_FEATURE_CX8              = 8,  
  CPU_FEATURE_APIC             = 9,  
  CPU_FEATURE_SEP              = 11, 
  CPU_FEATURE_MTRR             = 12, 
  CPU_FEATURE_PGE              = 13, 
  CPU_FEATURE_MCA              = 14, 
  CPU_FEATURE_CMOV             = 15, 
  CPU_FEATURE_PAT              = 16, 
  CPU_FEATURE_PSE36            = 17, 
  CPU_FEATURE_PSN              = 18, 
  CPU_FEATURE_CLF              = 19, 
  CPU_FEATURE_DTES             = 21, 
  CPU_FEATURE_ACPI             = 22, 
  CPU_FEATURE_MMX              = 23, 
  CPU_FEATURE_FXSR             = 24, 
  CPU_FEATURE_SSE              = 25, 
  CPU_FEATURE_SSE2             = 26, 
  CPU_FEATURE_SS               = 27, 
  CPU_FEATURE_HTT              = 28, 
  CPU_FEATURE_TM1              = 29, 
  CPU_FEATURE_IA64             = 30,
  CPU_FEATURE_PBE              = 31
};

enum cpuid_requests
{
  CPUID_GETVENDORSTRING,
  CPUID_GETFEATURES,
  CPUID_GETTLB,
  CPUID_GETSERIAL,
 
  CPUID_INTELEXTENDED = 0x80000000,
  CPUID_INTELFEATURES,
  CPUID_INTELBRANDSTRING,
  CPUID_INTELBRANDSTRINGMORE,
  CPUID_INTELBRANDSTRINGEND,
};

int cpu_has_feature (int);

#endif /* _ASM_CPUID_H */

