/*
 * video.c: Some functions handling all the graphic and screen stuff
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
 
/* !!!TODO
 *    - Write documentation
 *
 *    - IDEA: video.c could work with a internal state. So, we have a
 *    function video_set_attr such that following functions will usre
 *    this attribute.
 */

#include <util.h>

#include <console/video.h>
#include <vga/crtc.h>

typedef struct PACKED
{
  BYTE glyph;  /* Char goes here */
  BYTE attrib; /* VGA attribute */
}
egachar;

/* Debido a que la controladora de vídeo no nos permite averiguar
   cuántas líneas caben en la pantalla, tendremos que dar la
   oportunidad de configurar manualmente dicho parámetro. Espero
   encontrar una forma más elegante de calcular esto, pero mien-
   tras tanto, dejaré esta variable estática a la vista de
   `get_screen_height' donde almacenar este valor. */

static int __screen_lines = EGA_DEFAULT_SCREEN_HEIGHT;

WORD
video_cursor_get_pos_absolute (void)
{
  register int offset;
  
  offset = (crtc_read_register (VIDEO_CRTC_INDEX_CURSOR_LOCATION_HIGH) << 8) |
      crtc_read_register (VIDEO_CRTC_INDEX_CURSOR_LOCATION_LOW);
  
  return offset;
}

BYTE
video_cursor_get_pos_x (void)
{
  return video_cursor_get_pos_absolute () % video_get_screen_width ();
}

BYTE
video_cursor_get_pos_y ()
{
  return video_cursor_get_pos_absolute () / video_get_screen_width ();
}

void
video_cursor_set_pos_absolute (WORD offset)
{
  crtc_write_register (VIDEO_CRTC_INDEX_CURSOR_LOCATION_LOW, offset & 0xff);
  crtc_write_register (VIDEO_CRTC_INDEX_CURSOR_LOCATION_HIGH, (offset >> 8) & 0xff);
}

void
video_cursor_set_pos_xy (BYTE x, BYTE y)
{
  register int offset;
  
  offset = y * video_get_screen_width () + x;
  
  video_cursor_set_pos_absolute (offset);
}

void
video_cursor_setup (BYTE start, BYTE end)
{
  crtc_write_register (VIDEO_CRTC_INDEX_CURSOR_START, start);
  crtc_write_register (VIDEO_CRTC_INDEX_CURSOR_END, end);
}

void
video_clear_char (BYTE cchar, BYTE attr)
{
  register int i, j, w, h;
  
  w = video_get_screen_width ();
  h = video_get_screen_height ();
  
  for (i = 0; i < w; i++)
    {
      for (j = 0; j < h; j++)
        {
          egachar *p = VIDEO_BASE_ADDR;
          p[i+ j*w] = (egachar) {cchar, attr};
        }
    }
}

BYTE
video_get_screen_width ()
{
  return crtc_read_register (VIDEO_CRTC_INDEX_HORIZONTAL_END) + 1;
}

BYTE
video_get_screen_height ()
{
  return __screen_lines;
}

void
video_set_screen_height (BYTE height)
{
  __screen_lines = height;
}

void *
video_get_base_addr ()
{
  return VIDEO_BASE_ADDR;
}

void 
video_refresh ()
{
  /* Done by hardware. Don't care */
}

memsize_t
video_get_pages ()
{
  return __UNITS (video_get_screen_width () * video_get_screen_height () *
                  sizeof (egachar), PAGE_SIZE);
}


/* video.c ends here */

