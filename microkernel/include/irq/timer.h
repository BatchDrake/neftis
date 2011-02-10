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
    
#ifndef _IRQ_TIMER_H
#define _IRQ_TIMER_H

#include <types.h>
#include <misc/hook.h>

#define HZ 100
/*
  TODO: put this shit anywhere in the code. Is funny as hell.
  
  Emulate it with qemu -soundhw all 
  
static void play_sound(DWORD nFrequence) {
 	DWORD Div;
 	BYTE tmp;
 
        //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outportb(0x43, 0xb6);
 	outportb(0x42, (BYTE) (Div) );
 	outportb(0x42, (BYTE) (Div >> 8));
 
        //And play the sound using the PC speaker
 	tmp = inportb(0x61);
  	if (tmp != (tmp | 3)) {
 		outportb(0x61, tmp | 3);
 	}
 }
 
 //make it shutup
 static void nosound() {
 	BYTE tmp = (inportb(0x61) & 0xFC);
 
 	outportb(0x61, tmp);
 }
 
 //Make a beep
 void beep() {
 	 play_sound(1000);
          //set_PIT_2(old_frequency);
 }
*/


#define SYSTEM_TICK_HOOK 0

#define TIMER_HOOK_COUNT 1

void early_timers_init (void);


#endif /* _IRQ_TIMER_H */

