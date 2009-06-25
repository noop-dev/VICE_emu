/*
 * vic-cycle.c - Cycle based VIC-I emulation.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Daniel Kahlin <daniel@kahlin.net>
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

#ifdef WATCOM_COMPILE
#include "../mem.h"
#else
#include "mem.h"
#endif

#include "raster.h"
#include "types.h"
#include "vic-cycle.h"
#include "vic.h"
#include "vic20mem.h"
#include "vic20memrom.h"

/* ------------------------------------------------------------------------- */

/* Open vertical flipflop */
static inline void vic_cycle_open_v(void)
{
    vic.area = 1;
    vic.raster.display_ystart = vic.raster_line;
    vic.raster.geometry->gfx_position.y = vic.raster_line - vic.first_displayed_line;
    vic.raster.display_ystop = vic.screen_height - 1;
}

/* Close vertical flipflop */
static inline void vic_cycle_close_v(void)
{
    vic.area = 2;
    vic.raster.display_ystop = vic.raster_line;
}

/* Open horizontal flipflop */
static inline void vic_cycle_open_h(void)
{
    int xstart, xstop;

    xstart = MIN((unsigned int)(vic.raster_cycle * 4), vic.screen_width);
    xstop = xstart + vic.text_cols * 8;

    if (xstop >= (int)vic.screen_width) {
        xstop = vic.screen_width - 1;
        /* FIXME: SCREEN-MIXUP not handled */
    }

    xstart *= VIC_PIXEL_WIDTH;
    xstop *= VIC_PIXEL_WIDTH;

    vic.raster.display_xstart = xstart;
    vic.raster.geometry->gfx_position.x = xstart;
    vic.raster.display_xstop = xstop;

    if (xstop != xstart) {
        vic.raster.blank_this_line = 0;
        vic.fetch_state = VIC_FETCH_MATRIX;
        vic.buf_offset = 0;
    } else {
        vic.fetch_state = VIC_FETCH_DONE;
    }
}

/* Close horizontal flipflop */
static inline void vic_cycle_close_h(void)
{
    vic.fetch_state = VIC_FETCH_DONE;
}

/* Handle end of line */
static inline void vic_cycle_end_of_line(void)
{
    vic.raster_cycle = 0;
    vic_raster_draw_handler();
    vic.raster_line++;
    vic.fetch_state = VIC_FETCH_IDLE;
    vic.raster.blank_this_line = 1;
}

/* Handle end of frame */
static inline void vic_cycle_end_of_frame(void)
{
    /* Close h-flipflop on end of frame */
    if (vic.area == 1) {
        vic_cycle_close_v();
    }

    vic.raster_line = 0;
    vic.area = 0;
    vic.raster.display_ystart = -1;
    vic.raster.display_ystop = -1;
}

/* Latch number of columns */
static inline void vic_cycle_latch_columns(void)
{
    int new_text_cols = MIN(vic.regs[2] & 0x7f, (int)vic.max_text_cols);
    vic.text_cols = new_text_cols;
    vic.raster.geometry->gfx_size.width = new_text_cols * 8 * VIC_PIXEL_WIDTH;
    vic.raster.geometry->text_size.width = new_text_cols;
}

/* Latch number of rows */
static inline void vic_cycle_latch_rows(void)
{
    int new_text_lines = MIN((vic.regs[3] & 0x7e) >> 1, (int)vic.max_text_cols);
    vic.text_lines = new_text_lines;
    vic.raster.geometry->gfx_size.height = new_text_lines * 8;
    vic.raster.geometry->text_size.height = new_text_lines;
}

/* ------------------------------------------------------------------------- */
/* Fetch hendling */

/* Perform actual fetch */
/* TODO optimize */
static inline BYTE vic_cycle_do_fetch(int addr, BYTE *color)
{
    BYTE b, c;
    int color_addr = 0x9400 + (vic.regs[0x2] & 0x80 ? 0x200 : 0x0) + (addr & 0x1ff);

    if ((addr & 0x9000) == 0x8000) {
        /* chargen */
        b = vic20memrom_chargen_rom[addr & 0xfff];
        c = mem_ram[color_addr];
    } else if (addr >= 0x9400 && addr < 0x9800) {
        /* color RAM */
        b = mem_ram[addr];
        c = b; /* FIXME is this correct? */
    } else if ((addr < 0x0400) || ((addr >= 0x1000) && (addr < 0x2000))) {
        /* RAM */
        b = mem_ram[addr];
        c = mem_ram[color_addr];
    } else {
        /* unconnected */
        b = vic20_v_bus_last_data & (0xf0 | vic20_v_bus_last_high);
        c = mem_ram[color_addr]; /* FIXME is this correct? */
    }
    *color = vic20_v_bus_last_high = c;
    vic20_v_bus_last_data = b;

    return b;
}

/* Make a "real" 16b address from the 14b VIC address */
static inline int vic_cycle_fix_addr(int addr)
{
    int msb = ~((addr & 0x2000) << 2) & 0x8000;
    return (addr & 0x1fff) | msb;
}

/* Fetch handler */
static inline void vic_cycle_fetch(void)
{
    int addr;
    BYTE b;

    switch (vic.fetch_state) {
        /* fetch has not started yet */
        case VIC_FETCH_IDLE:
        /* fetch done on current line */
        case VIC_FETCH_DONE:
        default:
            /* TODO verify idle fetch address */
            vic_cycle_do_fetch(0x001c, &b);
            break;

        /* fetch from screen/color memomy */
        case VIC_FETCH_MATRIX:
            addr = (((vic.regs[5] & 0xf0) << 6)
                 | ((vic.regs[2] & 0x80) << 2))
                 + ((vic.memptr + vic.buf_offset));

            vic.vbuf[vic.buf_offset] = vic_cycle_do_fetch(vic_cycle_fix_addr(addr), &b);
            vic.cbuf[vic.buf_offset] = b;

            vic.fetch_state = VIC_FETCH_CHARGEN;
            break;

        /* fetch from chargen */
        case VIC_FETCH_CHARGEN:
            b = vic.vbuf[vic.buf_offset];
            addr = ((vic.regs[5] & 0xf) << 10)
                 + ((b * vic.char_height + ((vic.raster.ycounter - 1) & ((vic.char_height >> 1) | 7))));

            vic.gbuf[vic.buf_offset] = vic_cycle_do_fetch(vic_cycle_fix_addr(addr), &b);

            vic.buf_offset++;

            if (vic.buf_offset >= vic.text_cols) {
                vic_cycle_close_h();
            } else {
                vic.fetch_state = VIC_FETCH_MATRIX;
            }
            break;
    }
}

/* ------------------------------------------------------------------------- */

void vic_cycle(void)
{
    if (vic.area == 0) {
        /* Check for vertical flipflop */
        /* TODO 1 cycle too early? */
        if (vic.regs[1] == (vic.raster_line >> 1)) {
            vic_cycle_open_v();
        }
    }

    if (vic.area == 1) {
        /* Check for horizontal flipflop */
        if (vic.fetch_state == VIC_FETCH_IDLE && (vic.regs[0] & 0x7f) == vic.raster_cycle) {
            vic_cycle_open_h();
        }
    }

    /* Latch number of rows */
    if ((vic.raster_line == 0) && (vic.raster_cycle == 2)) {
        vic_cycle_latch_rows();
    }

    /* Latch number of columns */
    if (vic.raster_cycle == 1) {
        vic_cycle_latch_columns();
    }

    /* Perform fetch */
    vic_cycle_fetch();

    /* Next cycle */
    vic.raster_cycle++;
    if (vic.raster_cycle == vic.cycles_per_line) {
        vic_cycle_end_of_line();
        if (vic.raster_line == vic.screen_height) {
            vic_cycle_end_of_frame();
        }
    }
}

