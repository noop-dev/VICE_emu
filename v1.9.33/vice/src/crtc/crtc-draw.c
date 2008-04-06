/*
 * crtc.c - A line-based CRTC emulation (under construction).
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
 *
 * 16/24bpp support added by
 *  Steven Tieu <stieu@physics.ubc.ca>
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include "crtc-draw.h"
#include "crtc.h"
#include "crtctypes.h"
#include "raster-modes.h"
#include "types.h"


static DWORD dwg_table_0[256], dwg_table_1[256];
static DWORD dwg_table2x_0[256], dwg_table2x_1[256];
static DWORD dwg_table2x_2[256], dwg_table2x_3[256];

static void init_drawing_tables(void)
{
    int byte, p;
    BYTE msk;

    for (byte = 0; byte < 0x0100; byte++) {
        *((BYTE *)(dwg_table2x_0 + byte))
            = *((BYTE *)(dwg_table2x_0 + byte) + 1)
            = (byte & 0x80 ? 1 : 0);
        *((BYTE *)(dwg_table2x_0 + byte) + 2)
            = *((BYTE *)(dwg_table2x_0 + byte) + 3)
            = (byte & 0x40 ? 1 : 0);
        *((BYTE *)(dwg_table2x_1 + byte))
            = *((BYTE *)(dwg_table2x_1 + byte) + 1)
            = (byte & 0x20 ? 1 : 0);
        *((BYTE *)(dwg_table2x_1 + byte) + 2)
            = *((BYTE *)(dwg_table2x_1 + byte) + 3)
            = (byte & 0x10 ? 1 : 0);
        *((BYTE *)(dwg_table2x_2 + byte))
            = *((BYTE *)(dwg_table2x_2 + byte) + 1)
            = (byte & 0x08 ? 1 : 0);
        *((BYTE *)(dwg_table2x_2 + byte) + 2)
            = *((BYTE *)(dwg_table2x_2 + byte) + 3)
            = (byte & 0x04 ? 1 : 0);
        *((BYTE *)(dwg_table2x_3 + byte))
            = *((BYTE *)(dwg_table2x_3 + byte) + 1)
            = (byte & 0x02 ? 1 : 0);
        *((BYTE *)(dwg_table2x_3 + byte) + 2)
            = *((BYTE *)(dwg_table2x_3 + byte) + 3)
            = (byte & 0x01 ? 1 : 0);
    }

    for (byte = 0; byte < 0x0100; byte++) {
        for (msk = 0x80, p = 0; p < 4; msk >>= 1, p++)
            *((BYTE *)(dwg_table_0 + byte) + p)
                = (byte & msk ? 1 : 0);
        for (p = 0; p < 4; msk >>= 1, p++)
            *((BYTE *)(dwg_table_1 + byte) + p)
                = (byte & msk ? 1 : 0);
    }
}


/***************************************************************************/

static void draw_standard_background (int start_pixel, int end_pixel)
{
    memset(crtc.raster.draw_buffer_ptr + start_pixel,
           0,
           end_pixel - start_pixel + 1);
}

/***************************************************************************/

/* inline function... */
static inline void DRAW(int reverse_flag, int offset, int scr_rel,
                        int xs, int xc, int xe)
{
    /* FIXME: `p' has to be aligned on a 4 byte boundary!
              Is there a better way than masking `offset'?  */
    BYTE *p = crtc.raster.draw_buffer_ptr + (offset & ~3);
    BYTE *chargen_ptr, *screen_ptr;
    int screen_rel;
    int i, d;
    /* pointer to current chargen line */
    chargen_ptr = crtc.chargen_base
                + crtc.chargen_rel
                + (crtc.raster.ycounter & 0x0f);
    /* pointer to current screen line */
    screen_ptr = crtc.screen_base;
    screen_rel = ((scr_rel) + (xs));

    if (crtc.crsrmode && crtc.cursor_lines && crtc.crsrstate) {
        int crsrrel = ((crtc.regs[14] << 8) | crtc.regs[15]) & crtc.vaddr_mask;

        for (i = (xs); i < (xc); i++) {
            d = *(chargen_ptr
                + (screen_ptr[screen_rel & crtc.vaddr_mask] << 4));

            /* FIXME: mask with 0x3fff (screen_rel must be expanded) */
            if (screen_rel == crsrrel)
                d ^= 0xff;

            screen_rel++;

            if ((reverse_flag))
                d ^= 0xff;

            *(((DWORD *)p) + i * 2) = dwg_table_0[d];
            *(((DWORD *)p) + i * 2 + 1) = dwg_table_1[d];
        }
    } else {
        for (i = (xs); i < (xc); i++) {
            /* we use 16 bytes/char character generator */
            d = *(chargen_ptr
                + (screen_ptr[screen_rel & crtc.vaddr_mask] << 4));
            screen_rel++;

            if ((reverse_flag))
                d ^= 0xff;

            *(((DWORD *)p) + i * 2) = dwg_table_0[d];
            *(((DWORD *)p) + i * 2 + 1) = dwg_table_1[d];
        }
    }

    for (; i < (xe); i++) {
        d = 0;      /* blank */

        *(((DWORD *)p) + i * 2) = dwg_table_0[d];
        *(((DWORD *)p) + i * 2 + 1) = dwg_table_1[d];
    }
}


static void draw_standard_line(void)
{
    int rl_pos = crtc.xoffset + crtc.hjitter;
/*
    if (crtc.current_line == 1)
        printf("rl_pos=%d, scr_rel=%d, hw_cols=%d, rl_vis=%d, rl_len=%d\n",
               rl_pos, crtc.screen_rel, crtc.hw_cols, crtc.rl_visible,
               crtc.rl_len);
*/
    /* FIXME: check the ends against the maximum line length */
    /* the first part is left of rl_pos. Data is taken from prev. rl */
    if (rl_pos > 8) {
        DRAW(0,
             rl_pos % 8,
             crtc.prev_screen_rel,
             (crtc.prev_rl_len + 1) * crtc.hw_cols - (rl_pos / 8),
             crtc.prev_rl_visible * crtc.hw_cols,
             (crtc.prev_rl_len + 1) * crtc.hw_cols);
    }

    /* this is the "normal" part of the rasterline */
    DRAW(0,
         rl_pos,
         crtc.screen_rel,
         0,
         crtc.rl_visible * crtc.hw_cols,
         (crtc.rl_len + 1) * crtc.hw_cols);
}

static void draw_reverse_line(void)
{
    int rl_pos = crtc.xoffset + crtc.hjitter;

    /* the first part is left of rl_pos. Data is taken from prev. rl */
    if (rl_pos > 8) {
        DRAW(1,
             rl_pos % 8,
             crtc.prev_screen_rel,
             (crtc.prev_rl_len + 1) * crtc.hw_cols - (rl_pos / 8),
             crtc.prev_rl_visible * crtc.hw_cols,
             (crtc.prev_rl_len + 1) * crtc.hw_cols);
    }

    /* this is the "normal" part of the rasterline */
    DRAW(1,
         rl_pos,
         crtc.screen_rel,
         0,
         crtc.rl_visible * crtc.hw_cols,
         (crtc.rl_len + 1) * crtc.hw_cols);
}

/***************************************************************************/

#if 0

#define DRAW_CACHED(l, xs, xe, reverse_flag)                               \
    do {                                                                   \
        BYTE *p = draw_buffer_ptr + SCREEN_BORDERWIDTH + (xs) * 8;         \
        register int i;                                                    \
        register int mempos = ((l->n+1) / screen_charheight) * memptr_inc; \
        register int ypos = (l->n+1) % screen_charheight;                  \
                                                                           \
        for (i = (xs); i <= (xe); i++, p += 8) {                           \
            BYTE d = (l)->fgdata[i];                                       \
            if ((reverse_flag))                                            \
                d = ~d;                                                    \
                                                                           \
            if (crsrmode                                                   \
                && crsrstate                                               \
                && ypos >= crsrstart                                       \
                && ypos <=crsrend                                          \
                && mempos+i==crsrrel                                       \
                ) {                                                        \
                d = ~d;                                                    \
            }                                                              \
            *((DWORD *)p) = dwg_table_0[d];                                \
            *((DWORD *)p + 1) = dwg_table_1[d];                            \
        }                                                                  \
    } while (0)


static void draw_standard_line_cached(raster_cache_t *l, int xs, int xe)
{
    DRAW_CACHED(l, xs, xe, 0);
}

static void draw_reverse_line_cached(raster_cache_t *l, int xs, int xe)
{
    DRAW_CACHED(l, xs, xe, 1);
}


#define DRAW_CACHED_2x(l, xs, xe, reverse_flag)                \
    do {                                                       \
        BYTE *p = (draw_buffer_ptr                             \
                  + 2 * (SCREEN_BORDERWIDTH + (xs) * 8));      \
        register int i;                                        \
        register int mempos = ((l->n + 1) / screen_charheight) \
                              * memptr_inc;                    \
        register int ypos = (l->n+1) % screen_charheight;      \
                                                               \
        for (i = (xs); i <= (xe); i++, p += 16) {              \
            BYTE d = (l)->fgdata[i];                           \
            if ((reverse_flag))                                \
                d = ~d;                                        \
                                                               \
            if (crsrmode                                       \
                && crsrstate                                   \
                && ypos >= crsrstart                           \
                && ypos <=crsrend                              \
                && mempos+i==crsrrel                           \
                ) {                                            \
                d = ~d;                                        \
            }                                                  \
            *((DWORD *)p) = dwg_table2x_0[d];                  \
            *((DWORD *)p + 1) = dwg_table2x_1[d];              \
            *((DWORD *)p + 2) = dwg_table2x_2[d];              \
            *((DWORD *)p + 3) = dwg_table2x_3[d];              \
        }                                                      \
    } while (0)

static void draw_standard_line_cached_2x(raster_cache_t *l, int xs, int xe)
{
    DRAW_CACHED_2x(l, xs, xe, 0);
}

static void draw_reverse_line_cached_2x(raster_cache_t *l, int xs, int xe)
{
    DRAW_CACHED_2x(l, xs, xe, 1);
}

#endif

static void setup_single_size_modes (void)
{
    raster_modes_set(crtc.raster.modes, CRTC_STANDARD_MODE,
                     NULL /* get_std_text */,
                     NULL /* draw_std_text_cached */,
                     draw_standard_line,
                     draw_standard_background,
                     NULL /* draw_std_text_foreground */ );

    raster_modes_set(crtc.raster.modes, CRTC_REVERSE_MODE,
                     NULL /* get_rev_text */,
                     NULL /* draw_rev_text_cached*/,
                     draw_reverse_line,
                     draw_standard_background,
                     NULL /* draw_rev_text_foreground*/);
}


void crtc_draw_init(void)
{
    init_drawing_tables ();

    raster_set_table_refresh_handler(&crtc.raster, init_drawing_tables);

#ifdef CRTC_NEED_2X
    crtc_draw_set_double_size(0);
#endif
}

void crtc_draw_set_double_size(int enabled)
{
    setup_single_size_modes();
}

