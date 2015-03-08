/*
 * crtc.h: Control del CRT (monitor) a bajo nivel
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

#ifndef _CRTC_H
#define _CRTC_H

#include <types.h>
#include <asm/io.h>

#define VIDEO_BASE      0xb8000
#define VIDEO_BASE_ADDR ((void * ) VIDEO_BASE)
#define VIDEO_PAGES     2

#define EGA_DEFAULT_SCREEN_HEIGHT 25

#define VIDEO_TOP_ADDR (((int) VIDEO_BASE_ADDR) + SCREEN_HEIGHT \
        * SCREEN_WIDTH * sizeof (schar))
#define SCREEN_HEIGHT_MAX 140

#define VIDEO_CRTC_INDEX 0x3D4
#define VIDEO_CRTC_DATA  0x3D5

/* http://www.speccy.org/websromero/articulos/modox/modox3.html */
#define VIDEO_CRTC_INDEX_HORIZONTAL_TOTAL            0x00
#define VIDEO_CRTC_INDEX_HORIZONTAL_END              0x01
#define VIDEO_CRTC_INDEX_START_HORIZONTAL_BLANKING   0x02
#define VIDEO_CRTC_INDEX_END_HORIZONTAL_BLANKING     0x03

#define VIDEO_CRTC_INDEX_START_HORIZONTAL_RETRACE    0x04
#define VIDEO_CRTC_INDEX_END_HORIZONTAL_RETRACE      0x05
#define VIDEO_CRTC_INDEX_VERTICAL_TOTAL              0x06
#define VIDEO_CRTC_INDEX_OVERFLOW                    0x07

#define VIDEO_CRTC_INDEX_PRESET_ROW_SCAN             0x08
#define VIDEO_CRTC_INDEX_CHARACTER_HEIGHT            0x09
#define VIDEO_CRTC_INDEX_CURSOR_START                0x0A
#define VIDEO_CRTC_INDEX_CURSOR_END                  0x0B

#define VIDEO_CRTC_INDEX_START_ADDRESS_HIGH          0x0C
#define VIDEO_CRTC_INDEX_START_ADDRESS_LOW           0x0D
#define VIDEO_CRTC_INDEX_CURSOR_LOCATION_HIGH        0x0E
#define VIDEO_CRTC_INDEX_CURSOR_LOCATION_LOW         0x0F

#define VIDEO_CRTC_INDEX_RETRACE_START               0x10
#define VIDEO_CRTC_INDEX_RETRACE_END                 0x11
#define VIDEO_CRTC_INDEX_VERTICAL_DISPLAY_ENABLE_END 0x12
#define VIDEO_CRTC_INDEX_OFFSET                      0x13

#define VIDEO_CRTC_INDEX_UNDERLINE_LOCATION          0x14
#define VIDEO_CRTC_INDEX_STAR_VERTICAL_BLANKING      0x15
#define VIDEO_CRTC_INDEX_END_VERTICAL_BLANKING       0x16
#define VIDEO_CRTC_INDEX_MODE_CONTROL                0x17
#define VIDEO_CRTC_INDEX_LINE_COMPARE                0x18

INLINE BYTE
crtc_read_register (BYTE reg)
{
  BYTE result;
  
  outportb (VIDEO_CRTC_INDEX, reg);
  result = inportb (VIDEO_CRTC_DATA);
  
  return result;
}

INLINE void
crtc_write_register (BYTE reg, BYTE value)
{
  outportb (VIDEO_CRTC_INDEX, reg);
  outportb (VIDEO_CRTC_DATA, value);
}

#endif /* _CRTC_H */

