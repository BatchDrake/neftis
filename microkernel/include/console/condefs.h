/*
 * consoledefs.h: Definiciones usadas por la implementación de la
 * consola de texto del kernel.
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

#ifndef _CONSOLE_CONDEFS_H
#define _CONSOLE_CONDEFS_H

#include <types.h>
#include <lock/lock.h>

#define VIDEO_COLOR_BLACK     0x00
#define VIDEO_COLOR_BLUE      0x01
#define VIDEO_COLOR_GREEN     0x02
#define VIDEO_COLOR_CYAN      0x03
#define VIDEO_COLOR_RED       0x04
#define VIDEO_COLOR_MAGENTA   0x05
#define VIDEO_COLOR_YELLOW    0x06
#define VIDEO_COLOR_WHITE     0x07

#define VIDEO_COLOR_LIGHT     0x08

#define VIDEO_ATTR(f, b) ((f) | ((b) << 4))

#define VIDEO_OVERRIDE_FG(color, f) (((color) & 0xf0) | ((f)))
#define VIDEO_OVERRIDE_BG(color, b) (((color) & 0x0f) | ((b << 4)))

#define VIDEO_GET_FG(color) ((color) & 0x0f)
#define VIDEO_GET_BG(color) (((color) & 0xf0) >> 4)

#define AWFULLY_RED \
  VIDEO_ATTR (VIDEO_COLOR_WHITE | VIDEO_COLOR_LIGHT, VIDEO_COLOR_RED)
  
#define VGA_CONSOLE VIDEO_ATTR (VIDEO_COLOR_WHITE, VIDEO_COLOR_BLACK)
#define PHOSPHOR VIDEO_ATTR (VIDEO_COLOR_GREEN | VIDEO_COLOR_LIGHT, VIDEO_COLOR_GREEN)

#define BREOGAN VIDEO_ATTR (VIDEO_COLOR_WHITE, VIDEO_COLOR_RED | VIDEO_COLOR_LIGHT)

#define SYSCON_NUM                  12

#define CONSOLE_PARAM_EXPLICIT_CRLF 0
#define CONSOLE_PARAM_CHANGE_CUR    1
#define CONSOLE_PARAM_CLEAR_CHAR    2
#define CONSOLE_PARAM_CLEAR_COLOR   3
#define CONSOLE_PARAM_CR_CHAR       4
#define CONSOLE_PARAM_LF_CHAR       5
#define CONSOLE_PARAM_BS_CHAR       6
#define CONSOLE_PARAM_LOCALECHO     7

#define CONSOLE_DEFAULT_CRLF_BEHAVIOR 0
#define CONSOLE_DEFAULT_CUR_STATE     0
#define CONSOLE_DEFAULT_CLEAR_CHAR    ' '
#define CONSOLE_DEFAULT_CLEAR_COLOR  VGA_CONSOLE
#define CONSOLE_DEFAULT_CR_CHAR  '\r'
#define CONSOLE_DEFAULT_LF_CHAR  '\n'
#define CONSOLE_DEFAULT_BS_CHAR  '\b'

#define CONSOLE_MAX_ESCAPE       16

#define MAX_PARAM 16

typedef struct PACKED
{
  BYTE glyph;  /* Char goes here */
  BYTE attrib; /* VGA attribute */
}
video_char_t, schar;


struct console
{
  spin_t lock;
  WORD   width;
  WORD   height;
  
  WORD   pos_x;
  WORD   pos_y;
  
  DWORD  params [MAX_PARAM];
  
  int    escaped;
  BYTE   escape_type;
  BYTE   escape_sfx;
  int    escape_idx;
  BYTE   escapes[CONSOLE_MAX_ESCAPE];
  
  schar *buffer;
};

#endif /* _CONSOLE_CONDEFS_H */

