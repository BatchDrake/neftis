/*
 * console.h: Manejo de consolas (alto nivel)
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

#ifndef _CONSOLE_CONSOLE_H
#define _CONSOLE_CONSOLE_H

#include "console/condefs.h"

/* console_setup: Configura una consola dada por CON con los valores por
   defecto. Es lo que se debe usar para inicializar una consola antes de
   usarla. */
void console_setup (struct console *con);

/* console_set_base_addr: Establece la direcciÃ³n del buffer donde se alo-
   jarÃ¡n los caracteres a BASE en la consola CON, donde BASE usualmente
   serÃ¡ la direcciÃ³n de VIDEO_BASE_ADDR (buffer de vÃ­deo de EGA) */
void console_set_base_addr (struct console *con, void *base);

/* console_get_base_addr: Devuelve la direcciÃ³n del buffer de caracteres
   de la consola CON. */
void *console_get_buffer (struct console *con);

/* console_get_param: Devuelve un parÃ¡metro de la consola CON indexado
   por PARAM. Debemos usar esta funciÃ³n siempre que queramos conocer
   algunos valores configurables de la consola, como el tipo de salto
   de lÃ­nea que se debe usar, el color por defecto, etcÃ©tera. El argumento
   PARAM es uno de los valores prefijados por CONSOLE_PARAM_ definidos
   en el fichero `condefs.h' */
DWORD console_get_param (struct console *con, int param);

/* console_set_param: Establece el valor de un parÃ¡metro PARAM de
   la consola CON al valor VALUE. Ver `console_get_param'. Devuelve
   0 si el valor se ha podido establecer o -errno en caso contrario. */
int console_set_param (struct console *con, int param, DWORD value);

/* console_gotoxy: Posiciona el cursor de escritura de la consola
   CON a las coordenadas x (columna empezando en 0) e y (fila comenzando
   en 0) */
void console_gotoxy (struct console *con, WORD x, WORD y);

/* console_putchar: EnvÃ­a un caracter C a la consola CON, avanzando
   el cursor en una columna. Si dicho avance supera el valor de columnas
   por lÃ­nea, se produce un salto de lÃ­nea, y el cursor se posiciona en
   la columna cero de la siguiente fila (fila actual mÃ¡s uno). Si este
   avance supera el nÃºmero mÃ¡ximo de lÃ­neas de la consola se produce un
   "scroll", es decir, la primera fila se suprime, se mueven las lÃ­neas
   restantes una posiciÃ³n arriba (copiamos atrÃ¡s cada fila Y de forma que
   vaya a ocupar la posiciÃ³n Y - 1) y se deja la Ãºltima fila de la conso-
   la en blanco segÃºn el caracter de borrado. NÃ³tese que este comporta-
   miento puede cambiar si se trata de caracters especiales. */
void console_putchar (struct console *con, char c);

/* console_write: EnvÃ­a una serie de SIZE caracteres comenzando en
   la direcciÃ³n de memoria dada por BUFFER a la consola CON. Esto
   equivale a llamar a console_putchar por cada uno de los SIZE
   caracteres que hay en BUFFER. */
void console_write (struct console *con, const char *buffer, int size);

/* console_puts: EnvÃ­a una cadena S acabada en nulo (ascii 0) a la con-
   sola CON. Equivalende a hacer un `console_putchar' por cada caracter
   previo al nulo de S. Ver `console_write' */
void console_puts (struct console *con, const char *s);

void console_putchar_raw (struct console *con, char c);
void console_write_raw (struct console *con, const char *buffer, int size);
void console_puts_raw (struct console *con, const char *s);

/* console_clear: Limpia la pantalla (rellenando cada elemento del
   buffer de la consola con el caracter de borrado y el atributo
   asociado), y pone el cursor en la posiciÃ³n (0, 0) */
void console_clear (struct console *con);

void setup_system_consoles ();

int console_switch (int con);

int console_get_current ();

// void console_vprintf (struct console *, const char *, va_list);

#endif /*_CONSOLE_CONSOLE_H */

