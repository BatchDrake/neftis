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
    
#ifndef _ASM_TIMER_H
#define _ASM_TIMER_H

/* This timer is inaccurate as hell. Just having it as a fallback */

#define TIMER_PORT_CHANNEL(c) (0x40 + (c))

#define TIMER_PORT_COMMAND   0x43

#define TIMER_COUNTER_BCD    1


#define TIMER_MODE_0         (0 << 1)
#define TIMER_MODE_1         (1 << 1)
#define TIMER_MODE_2         (2 << 1)
#define TIMER_MODE_3         (3 << 1)
#define TIMER_MODE_4         (4 << 1)
#define TIMER_MODE_5         (5 << 1)

#define TIMER_LATCH_COMMAND  (0 << 4)
#define TIMER_ACCESS_LO      (1 << 4)
#define TIMER_ACCESS_HI      (2 << 4)
#define TIMER_ACCESS_LOHI    (TIMER_ACCESS_LO | TIMER_ACCESS_HI)

#define TIMER_CHANNEL(c)     ((c) << 6)

#define TIMER_BASE_FREQ      1193182L



#endif /* _ASM_TIMER_H */

