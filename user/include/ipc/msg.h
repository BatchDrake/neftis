/*
 *    Types and definitions for the userland message passing API
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

#ifndef _IPC_MSG_H
#define _IPC_MSG_H

#include <types.h>

#define MSG_MICRO_SIZE 256

struct msg_header
{
  busword_t mh_type;    /* Message type */
  busword_t mh_link;    /* Referrer */
  busword_t mh_sender;  /* Original TID */

  char mh_data[0]; /* Message data */
};

#endif /* _IPC_MSG_H */
