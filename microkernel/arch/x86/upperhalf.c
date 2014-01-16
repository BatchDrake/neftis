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

#include <string.h>

#include <mm/vm.h>
#include <mm/regions.h>

#include <asm/pagedir.h>
#include <asm/upperhalf.h>

#include <arch.h>
#include <util.h>

#include <console/condefs.h>
#include <console/video.h>
#include <vga/crtc.h>

extern int kernel_start;
extern int kernel_end;
extern int text_start;

BOOT_SYMBOL (static int cur_x) = 0;
BOOT_SYMBOL (static int cur_y) = 0;
BOOT_SYMBOL (char bootstack[4 * PAGE_SIZE]); /* Boot stack, as used by _start */

/* Some functions to tell if something's wrong */
#define SCREEN_WIDTH 80

BOOT_FUNCTION (static void btoutportb (WORD, BYTE));

static void
btoutportb (WORD port, BYTE data)
{
  __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}


void
btclear (void)
{
  schar *base = (schar *) VIDEO_BASE;
  int i, j;
  schar clear_char = {' ', BOOTTIME_DEFAULT_ATTRIBUTE};
  
  for (j = 0; j < EGA_DEFAULT_SCREEN_HEIGHT; ++j)
    for (i = 0; i < SCREEN_WIDTH; ++i)
      base[i + j * SCREEN_WIDTH] = clear_char;
}

void
btputs (const char *str)
{
  schar thischar = {' ', BOOTTIME_DEFAULT_ATTRIBUTE};
  schar *base = (schar *) VIDEO_BASE;
  WORD fullpos;
  
  while (*str)
  {
    if (*str == '\n')
    {
      cur_x = 0;
      ++cur_y;

      if (cur_y == EGA_DEFAULT_SCREEN_HEIGHT)
        cur_y = 0;
    }
    else
    {
      thischar.glyph = *str;
      base[cur_x++ + cur_y * SCREEN_WIDTH].glyph = *str;

      if (cur_x == SCREEN_WIDTH)
      {
        cur_x = 0;
        ++cur_y;

        if (cur_y == EGA_DEFAULT_SCREEN_HEIGHT)
          cur_y = 0;
      }
    }

    ++str;
  }

  fullpos = cur_x + cur_y * SCREEN_WIDTH;
  
  btoutportb (VIDEO_CRTC_INDEX, VIDEO_CRTC_INDEX_CURSOR_LOCATION_LOW);
  btoutportb (VIDEO_CRTC_DATA, fullpos & 0xff);

  btoutportb (VIDEO_CRTC_INDEX, VIDEO_CRTC_INDEX_CURSOR_LOCATION_HIGH);
  btoutportb (VIDEO_CRTC_DATA, (fullpos >> 8) & 0xff);
}

BOOT_FUNCTION (static void dword_to_decimal (DWORD, char *));
BOOT_FUNCTION (static void dword_to_hex (DWORD, char *));

static void
dword_to_decimal (DWORD num, char *out)
{
  int i, p;
  char temp;
  
  p = 0;

  if (!num)
  {
    *out++ = '0';
    *out++ = '\0';
  }
  else
  {
    while (num)
    {
      out[p++] = (num % 10) + '0';
      num /= 10;
    }
    
    for (i = 0; i < p / 2; ++i)
    {
      temp = out[i];
      out[i] = out[p - i - 1];
      out[p - i - 1] = temp;
    }

    out[p] = '\0';
  }
}

static void
dword_to_hex (DWORD num, char *out)
{
  int i, p;
  char temp;
  char hexchars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  p = 0;

  if (!num)
  {
    *out++ = '0';
    *out++ = '\0';
  }
  else
  {
    while (num)
    {
      out[p++] = hexchars[num & 0xf];
      num >>= 4;
    }
    
    for (i = 0; i < p / 2; ++i)
    {
      temp = out[i];
      out[i] = out[p - i - 1];
      out[p - i - 1] = temp;
    }

    out[p] = '\0';
  }
}

void
btputx (DWORD num)
{
  char buf[20];

  dword_to_hex (num, buf);

  btputs (buf);
}

void
btputd (DWORD num)
{
  char buf[20];

  dword_to_decimal (num, buf);

  btputs (buf);
}

void
boot_prepare_paging_early (DWORD *pagedir)
{
  
}

BOOT_SYMBOL (char string0[]) = "This is Atomik's boot_entry, v0.1 alpha\n(c) 2014 Gonzalo J. Carracedo <BatchDrake@gmail.com>\n\n";
  
BOOT_SYMBOL (char string1[]) = "Moving to upper half, kernel from in 0x";
BOOT_SYMBOL (char string2[]) = " to 0x";
BOOT_SYMBOL (char string3[]) = " (";
BOOT_SYMBOL (char string4[]) = " bytes)\n";
BOOT_SYMBOL (char string5[]) = "Kernel virtual base address is 0x";
BOOT_SYMBOL (char string6[]) = "\n\nConfiguring inital virtual address space... (not implemented)";

void
boot_entry (void)
{
  btclear ();
  btputs (string0);
  
  btputs (string1);
  btputx ((DWORD) &kernel_start);
  btputs (string2);
  btputx ((DWORD) &kernel_end);
  btputs (string3);
  btputd ((DWORD) &kernel_end - (DWORD) &kernel_start);
  btputs (string4);
  btputs (string5);
  btputx ((DWORD) &text_start);
  btputs (string6);
  
  for (;;);
}
