/*
 * video.h: Funciones para el control de la pantalla (alto nivel)
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

/* Esta parte está destinada a ser implementada por el bajo nivel. */
#ifndef _CONSOLE_VIDEO_H
#define _CONSOLE_VIDEO_H

#include <types.h>

/* video_cursor_setup: Establece el ancho del cursor del controlador de vÃ­deo,
   comenzando en la fila START y acabando en la fila END de la celda de carac-
   ter de la pantalla. */
void video_cursor_setup (BYTE start, BYTE end);

/* video_cursor_get_pos_absolute: Obtiene la posiciÃ³n absoluta del cursor en
   la pantalla, contando el nÃºmero de caracteres que hay entre el comienzo
   de la memoria de vÃ­deo y la propia posiciÃ³n del cursor. Una posiciÃ³n X
   equivaldrÃ­a a la posiciÃ³n que tiene el cursor si, comenzando este en el
   margen superior izquierdo de la pantalla, escribimos una serie de X carac-
   teres seguidos sin incluir los de control (saltos de lÃ­nea, retornos de
   carro, etc) */
WORD video_cursor_get_pos_absolute ();

/* video_cursor_set_pos_absolute: Establece a travÃ©s de OFFSET la posiciÃ³n
   absoluta del cursor en la pantalla. Ver `video_cursor_get_pos_absolute' */
void video_cursor_set_pos_absolute (WORD offset);

/* video_cursor_set_pos_xy: Establece la posiciÃ³n del cursor en la pantalla
   a travÃ©s de las coordenadas X e Y, siendo X = 0 la primera columna e
   Y = 0 la primera fila. */
void video_cursor_set_pos_xy (BYTE x, BYTE y);

/* video_cursor_get_pos_y: Obtiene la fila en la que se encuentra actualmente
   el cursor en la pantalla. */
BYTE video_cursor_get_pos_y ();

/* video_cursor_get_pos_x: Obtiene la columna en la que se encuentra el cursor
   de la pantalla. */
BYTE video_cursor_get_pos_x ();

/* video_clear_char: Escribe el caracter CCHAR con el atributo ATTR tantas veces
   como sea posible hasta ocupar toda la memoria de vÃ­deo. */
void video_clear_char (BYTE cchar, BYTE attr);

/* video_get_screen_width: Obtiene el nÃºmero de caracteres que caben en cada
   lÃ­nea de la pantalla. */
BYTE video_get_screen_width ();

/* video_get_screen_height: Obtiene el nÃºmero mÃ¡ximo de lÃ­neas en la pantalla
   que se pueden representar. */
BYTE video_get_screen_height ();

/* video_set_screen_height: Establece el nÃºmero mÃ¡ximo de lÃ­neas en la pantalla
   que se pueden representar en HEIGHT lÃ­neas. */
void video_set_screen_height (BYTE height);

/* video_get_base_addr: Obtiene la direcciÃ³n de memoria a partir de la cual
   la controladora de vÃ­deo toma los datos para reproducir los caracteres
   por pantalla. Todos los caracteres en formato schar (ver `videochar.h')
   deben escribirse aquÃ­. */
void *video_get_base_addr ();

/* Refresca la pantalla después de una escritura. Se implementa como una
   función muda en VGA. */
void  video_refresh ();

/* Devuelve el número de páginas necesarias para alojar la memoria de vídeo */
memsize_t video_get_pages ();

#endif /* _CONSOLE_VIDEO_H */

