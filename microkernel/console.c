/*
 * console.c: Interfaz de consolas.
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

#include <string.h>
#include <ctype.h>

#include <console/console.h>
#include <mm/alloc.h>

#include <misc/msgsink.h>
#include <util.h>

struct console *syscon;
struct console  syscon_list[SYSCON_NUM];

void
console_msgsink_putchar (void *opaque, char c)
{
  if (syscon)
    console_putchar (syscon, c);
}

void
console_msgsink_puts (void *opaque, const char *s)
{
  if (syscon)
    console_puts (syscon, s);
}

/* FIXME: protect this with locks */
void
console_setup (struct console *con)
{
  static struct msgsink console_msgsink =
  {
    .opaque  = NULL,
    .putchar = console_msgsink_putchar,
    .puts    = console_msgsink_puts
  };
  
  con->width = video_get_screen_width ();
  con->height = video_get_screen_height ();
  
  con->pos_x = video_cursor_get_pos_x ();
  con->pos_y = video_cursor_get_pos_y ();
  
  con->buffer = (schar *) video_get_base_addr ();
  
  console_set_param (con, CONSOLE_PARAM_EXPLICIT_CRLF, 
    CONSOLE_DEFAULT_CRLF_BEHAVIOR);
  
  console_set_param (con, CONSOLE_PARAM_CHANGE_CUR, 
    CONSOLE_DEFAULT_CUR_STATE);
  
  console_set_param (con, CONSOLE_PARAM_CLEAR_CHAR, 
    CONSOLE_DEFAULT_CLEAR_CHAR);
  
  console_set_param (con, CONSOLE_PARAM_CLEAR_COLOR, 
    CONSOLE_DEFAULT_CLEAR_COLOR);
  
  console_set_param (con, CONSOLE_PARAM_CR_CHAR,
    CONSOLE_DEFAULT_CR_CHAR);
  
  console_set_param (con, CONSOLE_PARAM_LF_CHAR, 
    CONSOLE_DEFAULT_LF_CHAR);
  
  console_set_param (con, CONSOLE_PARAM_BS_CHAR, 
    CONSOLE_DEFAULT_BS_CHAR);
  
  if (syscon == NULL)
  {
    console_set_param (con, CONSOLE_PARAM_CHANGE_CUR, 1);
    console_set_param (con, CONSOLE_PARAM_EXPLICIT_CRLF, 0);
    syscon = con;
  }

  if (kernel_option_enabled ("console", 1))
    msgsink_register (&console_msgsink);
}

void
console_set_buffer (struct console *con, void *base)
{
  con->buffer = (schar *) con->buffer;
}

void *
console_get_buffer (struct console *con)
{
  return (void *) con->buffer;
}

DWORD
console_get_param (struct console *con, int param)
{
  if (param > MAX_PARAM)
    return 0;
  
  return con->params[param];
}

/* !!! TODO: Detectar qué parámetros podemos cambiar y qué parámetros no
   según el contexto. */
int
console_set_param (struct console *con, int param, DWORD value)
{
  if (unlikely (param > MAX_PARAM))
    return KERNEL_ERROR_VALUE;
  
  con->params[param] = value;
  
  return 0;
}

void
console_gotoxy (struct console *con, WORD x, WORD y)
{
  if (x >= con->width)
    x = con->width - 1;
  
  if (y >= con->height)
    y = con->height - 1;
  
  con->pos_x = x;
  con->pos_y = y;
  
  if (con->params[CONSOLE_PARAM_CHANGE_CUR])
    video_cursor_set_pos_xy (x, y);
}

static void
console_scroll (struct console *con)
{
  int i;
  schar clear_pair;
  
  clear_pair = (schar) {con->params[CONSOLE_PARAM_CLEAR_CHAR], 
    con->params[CONSOLE_PARAM_CLEAR_COLOR]};
  
  for (i = 1; i < con->height; i++)
    memcpy (con->buffer + (i - 1) * con->width, con->buffer + i * con->width, 
      sizeof (schar) * con->width);
  
  for (i = 0; i < con->width; i++)
    con->buffer[i + con->width * (con->height - 1)] = clear_pair;
}

INLINE BYTE
__ecma_to_vga (BYTE color)
{
  BYTE colors[] = {VIDEO_COLOR_BLACK,
                   VIDEO_COLOR_RED,
                   VIDEO_COLOR_GREEN,
                   VIDEO_COLOR_YELLOW,
                   VIDEO_COLOR_BLUE,
                   VIDEO_COLOR_MAGENTA,
                   VIDEO_COLOR_CYAN,
                   VIDEO_COLOR_WHITE};
                   
  if (color > 7)
    return 0;
  
  return colors[color];
}

/* Incluir 2J, 1;1H y estos */
INLINE void
__console_parse_escape (struct console *con)
{
  int i, tmp;
  int light = 0;
  int blink = 0;
  int fore_color = 
    VIDEO_GET_FG (console_get_param (con, CONSOLE_PARAM_CLEAR_COLOR));
  int back_color = 
    VIDEO_GET_BG (console_get_param (con, CONSOLE_PARAM_CLEAR_COLOR));
  
  if (con->escape_type == '[')
  {
    if (con->escape_sfx = 'm')
    {
      for (i = 0; i <= con->escape_idx; i++)
      {
        if (con->escapes[i] == 0)
        {
          back_color = VIDEO_GET_BG (CONSOLE_DEFAULT_CLEAR_COLOR);
          fore_color = VIDEO_GET_FG (CONSOLE_DEFAULT_CLEAR_COLOR);
          light = 0;
          blink = 0;
        }
        else if (con->escapes[i] == 1)
          light = 1;
        else if (con->escapes[i] == 5)
          blink = 1;
        else if (con->escapes[i] == 7)
        {
          tmp = fore_color;
          fore_color = back_color;
          back_color = tmp;
        }
        else if (con->escapes[i] == 21 || con->escapes[i] == 22)
          light = 0;
        else if (con->escapes[i] == 25)
          blink = 0;
        else if (con->escapes[i] >= 30 && con->escapes[i] <= 37)
        {
          fore_color = __ecma_to_vga (con->escapes[i] - 30) |
            (light * VIDEO_COLOR_LIGHT);
        }
        
        else if (con->escapes[i] >= 40 && con->escapes[i] <= 47)
        {
          back_color = __ecma_to_vga (con->escapes[i] - 40) |
            (blink * VIDEO_COLOR_LIGHT);
        }
      }
      
      console_set_param (con, CONSOLE_PARAM_CLEAR_COLOR,
        VIDEO_ATTR (fore_color, back_color));
    }
  }
}

INLINE void
__console_inline_putchar_raw (struct console *con, char c)
{
  schar char_pair;
  
  char_pair = (schar) {c, con->params[CONSOLE_PARAM_CLEAR_COLOR]};
  con->buffer[con->pos_y * con->width + con->pos_x] = char_pair;
      
  if (++con->pos_x >= con->width)
  {
    con->pos_x = 0;
    if (++con->pos_y >= con->height)
    {
      con->pos_y--;
      console_scroll (con);
    }
  }
  
  if (con->params[CONSOLE_PARAM_CHANGE_CUR])
    video_cursor_set_pos_xy (con->pos_x, con->pos_y);
}

INLINE void
__console_inline_putchar (struct console *con, char c)
{
  if (con->escaped)
  {
    if (!con->escape_type)
      con->escape_type = c;
    else if (con->escape_type == '[')
    {
      if (c == ';')
      {
        con->escape_idx++;
        if (con->escape_idx >= CONSOLE_MAX_ESCAPE)
          con->escape_idx = CONSOLE_MAX_ESCAPE - 1;
          
        con->escapes[con->escape_idx] = 0;
      }
      else if (c >= '0' && c <= '9')
      {
        con->escapes[con->escape_idx] *= 10;
        con->escapes[con->escape_idx] += c - '0';
      }
      else
      {
        con->escape_sfx = c;
        __console_parse_escape (con);
        con->escaped = 0;
      }
    }
    else
      con->escaped = 0;
    
    return;
  }
  
  if (c == '\033')
  {
    con->escaped = 1;
    con->escape_type = 0;
    con->escape_sfx = 0;
    con->escape_idx = 0;
    con->escapes[0] = 0;
    
    return;
  }
  
  if (c == con->params[CONSOLE_PARAM_CR_CHAR])
  {
     con->pos_x = 0;
  }
  else
    if (c == con->params[CONSOLE_PARAM_LF_CHAR])
  {
    if (!con->params[CONSOLE_PARAM_EXPLICIT_CRLF])
      con->pos_x = 0;
    
    if (++con->pos_y >= con->height)
    {
      con->pos_y--;
      console_scroll (con);
    }
  }
  else
    if (c == con->params[CONSOLE_PARAM_BS_CHAR])
  {
    if (con->pos_x)
      con->pos_x--;
      
    con->buffer[con->pos_y * con->width + con->pos_x] = 
      (schar) {con->params[CONSOLE_PARAM_CLEAR_CHAR], 
      con->params[CONSOLE_PARAM_CLEAR_COLOR]};
  }
  else
    __console_inline_putchar_raw (con, c);
  
  if (con->params[CONSOLE_PARAM_CHANGE_CUR])
    video_cursor_set_pos_xy (con->pos_x, con->pos_y);
}

void
console_putchar (struct console *con, char c)
{
  __console_inline_putchar (con, c);
  
  if (con == syscon)
    video_refresh ();
}

void
console_write (struct console *con, const char *buffer, int size)
{
  int i;
  for (i = 0; i < size; i++)
    __console_inline_putchar (con, buffer[i]);
  
  if (con == syscon)
    video_refresh ();
}

void console_puts (struct console *con, const char *s)
{
  console_write (con, s, strlen (s));
  
  if (con == syscon)
    video_refresh ();
}


void
console_putchar_raw (struct console *con, char c)
{
  __console_inline_putchar_raw (con, c);
  
  if (con == syscon)
    video_refresh ();
}

void
console_write_raw (struct console *con, const char *buffer, int size)
{
  int i;
  
  for (i = 0; i < size; i++)
    __console_inline_putchar_raw (con, buffer[i]);
  
  if (con == syscon)
    video_refresh ();
}

void console_puts_raw (struct console *con, const char *s)
{
  console_write_raw (con, s, strlen (s));
  
  if (con == syscon)
    video_refresh ();
}

void
console_clear (struct console *con)
{
  int matrix_size, i;
  schar clear_pair;
  
  clear_pair = (schar) {con->params[CONSOLE_PARAM_CLEAR_CHAR], 
    con->params[CONSOLE_PARAM_CLEAR_COLOR]};
  
  matrix_size = con->width * con->height;
  
  for (i = 0; i < matrix_size; i++)
    con->buffer[i] = clear_pair;
  
  console_gotoxy (con, 0, 0);
  
  if (con == syscon)
    video_refresh ();
}

int
console_get_current ()
{
  int i;
  
  for (i = 0; i < SYSCON_NUM; i++)
    if (syscon == &syscon_list[i])
      return i;
      
  return KERNEL_ERROR_VALUE;
}

int
console_switch (int con)
{
  schar *buffer;
  schar *videobuf;

  
  if (IN_BOUNDS (con, SYSCON_NUM))
    return KERNEL_ERROR_VALUE;

  if (&syscon_list[con] == syscon)
    return KERNEL_SUCCESS_VALUE;

  RETURN_ON_PTR_FAILURE (buffer = (schar *) page_alloc (video_get_pages ()));
  
  memcpy (buffer, syscon->buffer, 
    syscon->width * syscon->height * sizeof (schar));

  videobuf = syscon->buffer;
  syscon->buffer = buffer;
  
  console_set_param (syscon, CONSOLE_PARAM_CHANGE_CUR, 0);
 
  syscon = &syscon_list[con];

  memcpy (videobuf, syscon->buffer, 
    syscon->width * syscon->height * sizeof (schar));

  page_free (syscon->buffer, video_get_pages ());

  syscon->buffer = videobuf;
  
  console_set_param (syscon, CONSOLE_PARAM_CHANGE_CUR, 1);

  console_gotoxy (syscon, syscon->pos_x, syscon->pos_y);
  
  video_refresh ();
  
  return KERNEL_SUCCESS_VALUE;
}

void
setup_system_consoles ()
{
  int i;
  
  syscon_list[0] = *syscon;
  
  for (i = 1; i < SYSCON_NUM; i++)
  {
    syscon_list[i].width = video_get_screen_width ();
    syscon_list[i].height = video_get_screen_height ();

    syscon_list[i].pos_x = 0;
    syscon_list[i].pos_y = 0;

    MANDATORY (
      PTR_LIKELY_TO_SUCCESS (
        syscon_list[i].buffer = (schar *) page_alloc (video_get_pages ())
      )
    )
    
    console_set_param (&syscon_list[i], CONSOLE_PARAM_EXPLICIT_CRLF, 
      CONSOLE_DEFAULT_CRLF_BEHAVIOR);
      
    console_set_param (&syscon_list[i], CONSOLE_PARAM_CHANGE_CUR, 
      0);
      
    console_set_param (&syscon_list[i], CONSOLE_PARAM_CLEAR_CHAR, 
      CONSOLE_DEFAULT_CLEAR_CHAR);
      
    console_set_param (&syscon_list[i], CONSOLE_PARAM_CLEAR_COLOR, 
      CONSOLE_DEFAULT_CLEAR_COLOR);
      
    console_set_param (&syscon_list[i], CONSOLE_PARAM_CR_CHAR, 
      CONSOLE_DEFAULT_CR_CHAR);
      
    console_set_param (&syscon_list[i], CONSOLE_PARAM_LF_CHAR, 
      CONSOLE_DEFAULT_LF_CHAR);
      
    console_set_param (&syscon_list[i], CONSOLE_PARAM_BS_CHAR, 
      CONSOLE_DEFAULT_BS_CHAR);
      
    console_set_param (&syscon_list[i], CONSOLE_PARAM_LOCALECHO, 
      1);
      
    console_clear (&syscon_list[i]);
  }
  
  syscon = syscon_list;
}

