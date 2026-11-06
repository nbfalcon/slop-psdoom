// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998, 1999 by Udo Munk
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION: Function to write the Doom end message text (ENDOOM)
// Translated from Code Page 437 to modern Linux UTF-8 box-drawing/block glyphs.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"
#include "doomstat.h"
#include "w_wad.h"
#include "z_zone.h"
#include "endtxt.h"

// CP437 table for 0x80 - 0xFF mapped to UTF-8 strings
static const char *const cp437_utf8[128] = {
    "\xc3\x87", "\xc3\xbc", "\xc3\xa9", "\xc3\xa2", "\xc3\xa4", "\xc3\xa0", "\xc3\xa5", "\xc3\xa7",
    "\xc3\xaa", "\xc3\xab", "\xc3\xa8", "\xc3\xaf", "\xc3\xae", "\xc3\xac", "\xc3\x84", "\xc3\x85",
    "\xc3\x89", "\xc3\xa6", "\xc3\x86", "\xc3\xb4", "\xc3\xb6", "\xc3\xb2", "\xc3\xbb", "\xc3\xb9",
    "\xc3\xbf", "\xc3\x96", "\xc3\x9c", "\xc2\xa2", "\xc2\xa3", "\xc2\xa5", "\xe2\x82\xa7", "\xc6\x92",
    "\xc3\xa1", "\xc3\xad", "\xc3\xb3", "\xc3\xba", "\xc3\xb1", "\xc3\x91", "\xc2\xaa", "\xc2\xba",
    "\xc2\xbf", "\xe2\x8c\x90", "\xc2\xac", "\xc2\xbd", "\xc2\xbc", "\xc2\xa1", "\xc2\xab", "\xc2\xbb",
    "\xe2\x96\x91", "\xe2\x96\x92", "\xe2\x96\x93", "\xe2\x94\x82", "\xe2\x94\xa4", "\xe2\x95\xa1", "\xe2\x95\xa2", "\xe2\x95\x96",
    "\xe2\x95\x95", "\xe2\x95\xa3", "\xe2\x95\x91", "\xe2\x95\x97", "\xe2\x95\x9d", "\xe2\x95\x9c", "\xe2\x95\x9b", "\xe2\x94\x90",
    "\xe2\x94\x94", "\xe2\x94\xb4", "\xe2\x94\xac", "\xe2\x94\x9c", "\xe2\x94\x80", "\xe2\x94\xbc", "\xe2\x95\x9e", "\xe2\x95\x9f",
    "\xe2\x95\x9a", "\xe2\x95\x94", "\xe2\x95\xa9", "\xe2\x95\xa6", "\xe2\x95\xa0", "\xe2\x95\x90", "\xe2\x95\xac", "\xe2\x95\xa7",
    "\xe2\x95\xa8", "\xe2\x95\xa4", "\xe2\x95\xa5", "\xe2\x95\x99", "\xe2\x95\x98", "\xe2\x95\x92", "\xe2\x95\x93", "\xe2\x95\xab",
    "\xe2\x95\xaa", "\xe2\x94\x98", "\xe2\x94\x8c", "\xe2\x96\x88", "\xe2\x96\x84", "\xe2\x96\x8c", "\xe2\x96\x90", "\xe2\x96\x80",
    "\xce\xb1", "\xc3\x9f", "\xce\x93", "\xcf\x80", "\xce\xa3", "\xcf\x83", "\xc2\xb5", "\xcf\x84",
    "\xce\xa6", "\xce\x98", "\xce\xa9", "\xce\xb4", "\xe2\x88\x9e", "\xcf\x86", "\xce\xb5", "\xe2\x88\xa9",
    "\xe2\x89\xa1", "\xc2\xb1", "\xe2\x89\xa5", "\xe2\x89\xa4", "\xe2\x8c\xa0", "\xe2\x8c\xa1", "\xc3\xb7", "\xe2\x89\x88",
    "\xc2\xb0", "\xe2\x88\x99", "\xc2\xb7", "\xe2\x88\x9a", "\xe2\x81\xbf", "\xc2\xb2", "\xe2\x96\xa0", "\xc2\xa0",
};

void ShowEndTxt(void)
{
    int i, j;
    int att = -1;
    unsigned short *text;

    // if option -noendtxt is set, don't print the text
    if (noendtxt)
        return;

    int lump = W_CheckNumForName("ENDOOM");
    if (lump < 0)
        return;

    text = W_CacheLumpNum(lump, PU_CACHE);

    // Standard Linux ANSI terminal reset before printing
    printf("\033[0m");

    // print 80x25 text and deal with the ANSI color attributes
    for (i = 1; i <= 80 * 25; i++) {
        j = (*text >> 8) & 0xff;
        if (j != att) {
            att = j;
            int fg = j & 0x0f;
            int bg = (j >> 4) & 0x07;
            int bold = (fg >= 8) ? 1 : 0;
            int fg_ansi = 30 + (fg & 0x07);
            int bg_ansi = 40 + (bg & 0x07);

            if (bold)
                printf("\033[0;1;%d;%dm", fg_ansi, bg_ansi);
            else
                printf("\033[0;%d;%dm", fg_ansi, bg_ansi);
        }

        unsigned char ch = (unsigned char)(*text & 0xff);
        text++;

        if (ch >= 0x80) {
            fputs(cp437_utf8[ch - 0x80], stdout);
        } else if (ch == 0) {
            putchar(' ');
        } else {
            putchar(ch);
        }

        if ((i % 80) == 0) {
            printf("\033[0m\n");
            att = -1;
        }
    }

    printf("\033[0m");
    fflush(stdout);
}
