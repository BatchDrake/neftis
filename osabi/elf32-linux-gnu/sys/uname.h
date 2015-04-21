/*
 *    Linux uname() system call implementation
 *    Copyright (C) 2015  Gonzalo J. Carracedo
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

#ifndef _SYS_UNAME_H
#define _SYS_UNAME_H

/* Mostly taken from GNU C library */

#define _UTSNAME_LENGTH 65

/* Linux provides as additional information in the `struct utsname'
   the name of the current domain.  Define _UTSNAME_DOMAIN_LENGTH
   to a value != 0 to activate this entry.  */
#define _UTSNAME_DOMAIN_LENGTH _UTSNAME_LENGTH
#ifndef _UTSNAME_SYSNAME_LENGTH
# define _UTSNAME_SYSNAME_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_NODENAME_LENGTH
# define _UTSNAME_NODENAME_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_RELEASE_LENGTH
# define _UTSNAME_RELEASE_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_VERSION_LENGTH
# define _UTSNAME_VERSION_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_MACHINE_LENGTH
# define _UTSNAME_MACHINE_LENGTH _UTSNAME_LENGTH
#endif

/* Structure describing the system and machine.  */
struct utsname
{
  /* Name of the implementation of the operating system.  */
  char sysname[_UTSNAME_SYSNAME_LENGTH];

  /* Name of this node on the network.  */
  char nodename[_UTSNAME_NODENAME_LENGTH];

  /* Current release level of this implementation.  */
  char release[_UTSNAME_RELEASE_LENGTH];
  /* Current version level of this release.  */
  char version[_UTSNAME_VERSION_LENGTH];

  /* Name of the hardware type the system is running on.  */
  char machine[_UTSNAME_MACHINE_LENGTH];

#if _UTSNAME_DOMAIN_LENGTH - 0
  /* Name of the domain of this node on the network.  */
# ifdef __USE_GNU
  char domainname[_UTSNAME_DOMAIN_LENGTH];
# else
  char __domainname[_UTSNAME_DOMAIN_LENGTH];
# endif
#endif
};

#ifdef __USE_SVID
/* Note that SVID assumes all members have the same size.  */
# define SYS_NMLN  _UTSNAME_LENGTH
#endif



#endif /* _SYS_UNAME_H */
