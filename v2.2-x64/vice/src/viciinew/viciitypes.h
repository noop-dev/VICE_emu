/*
 * viciitypes.h - A cycle-exact event-driven MOS6569 (VIC-II) emulation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef VICE_VICIITYPES_H
#define VICE_VICIITYPES_H

#include "raster.h"
#include "types.h"

/* Screen constants.  */
#define VICII_SCREEN_XPIX                  320
#define VICII_SCREEN_YPIX                  200
#define VICII_SCREEN_TEXTCOLS              40
#define VICII_SCREEN_TEXTLINES             25
#define VICII_SCREEN_CHARHEIGHT            8

#define VICII_40COL_START_PIXEL vicii.screen_leftborderwidth
#define VICII_40COL_STOP_PIXEL  (vicii.screen_leftborderwidth + VICII_SCREEN_XPIX)
#define VICII_38COL_START_PIXEL (vicii.screen_leftborderwidth + 7)
#define VICII_38COL_STOP_PIXEL  (vicii.screen_leftborderwidth + 311)

#define VICII_NUM_SPRITES      8
#define VICII_MAX_SPRITE_WIDTH 56  /* expanded sprite in bug area */
#define VICII_NUM_COLORS       16

/* drawing constants. */
#define VICII_DRAW_BUFFER_SIZE (65 * 8)

/* Available video modes.  The number is given by
   ((vicii.regs[0x11] & 0x60) | (vicii.regs[0x16] & 0x10)) >> 4.  */
enum vicii_video_mode_s {
    VICII_NORMAL_TEXT_MODE,
    VICII_MULTICOLOR_TEXT_MODE,
    VICII_HIRES_BITMAP_MODE,
    VICII_MULTICOLOR_BITMAP_MODE,
    VICII_EXTENDED_TEXT_MODE,
    VICII_ILLEGAL_TEXT_MODE,
    VICII_ILLEGAL_BITMAP_MODE_1,
    VICII_ILLEGAL_BITMAP_MODE_2,
    VICII_IDLE_MODE,           /* Special mode for idle state.  */
    VICII_NUM_VMODES
};
typedef enum vicii_video_mode_s vicii_video_mode_t;

#define VICII_DUMMY_MODE VICII_NORMAL_TEXT_MODE

#define VICII_IS_ILLEGAL_MODE(x) ((x) >= VICII_ILLEGAL_TEXT_MODE \
                                 && (x) <= VICII_ILLEGAL_BITMAP_MODE_2)

#define VICII_IS_BITMAP_MODE(x)  ((x) & 0x02)

#define VICII_IS_TEXT_MODE(x)    ((x) == VICII_NORMAL_TEXT_MODE \
                                 || (x) == VICII_MULTICOLOR_TEXT_MODE \
                                 || (x) == VICII_EXTENDED_TEXT_MODE)

/* These timings are taken from the ``VIC Article'' by Christian Bauer
   <bauec002@goofy.zdv.uni-mainz.de>.  Thanks Christian!
   Note: we measure cycles from 0 to 62, not from 1 to 63 as he does.  */

/* Cycle # at which the VIC takes the bus in a bad line (BA goes low).  */
#define VICII_FETCH_CYCLE          11

/* Delay for the raster line interrupt.  This is not due to the VIC-II, since
   it triggers the IRQ line at the beginning of the line, but to the 6510
   that needs at least 2 cycles to detect it.  */
#define VICII_RASTER_IRQ_DELAY     2

/* Current char being drawn by the raster.  < 0 or >= VICII_SCREEN_TEXTCOLS
   if outside the visible range.  */
#define VICII_RASTER_CHAR(cycle)   ((int)(cycle) - 15)

/* Current horizontal position (in pixels) of the raster.  < 0 or >=
   SCREEN_WIDTH if outside the visible range.  */
#define VICII_RASTER_X(cycle)      (((int)(cycle) - 17) * 8 + vicii.screen_leftborderwidth)

/* Current vertical position of the raster.  Unlike `rasterline', which is
   only accurate if a pending drawing event has been served, this is
   guarranteed to be always correct.  It is a bit slow, though.  */
#define VICII_RASTER_Y(clk)        ((unsigned int)((clk) \
                                   / vicii.cycles_per_line) \
                                   % vicii.screen_height)

/* Cycle # within the current line.  */
#define VICII_RASTER_CYCLE(clk)    ((unsigned int)(((clk) + 1) \
                                   % vicii.cycles_per_line))
/* `clk' value for the beginning of the current line.  */
#define VICII_LINE_START_CLK(clk)  (((clk) / vicii.cycles_per_line) \
                                   * vicii.cycles_per_line)

/* # of the previous and next raster line.  Handles wrap over.  */
#define VICII_PREVIOUS_LINE(line)  (((line) > 0) \
                                   ? (line) - 1 : vicii.screen_height - 1)
#define VICII_NEXT_LINE(line)      (((line) + 1) % vicii.screen_height)

/* VIC-II structures.  This is meant to be used by VIC-II modules
   *exclusively*!  */

struct vicii_light_pen_s {
    int triggered;
    int x, y, x_extra_bits;
};
typedef struct vicii_light_pen_s vicii_light_pen_t;

struct vicii_sprite_s {
    /* Sprite data to display */
    DWORD data;
    /* 6 bit counters */
    BYTE mc;
    BYTE mcbase;
    /* 8 bit pointer */
    BYTE pointer;
    /* Expansion flop */
    int exp_flop;
    /* Flag: is sprite DMA active? */
    int dma;
    /* X coordinate */
    int x;
};
typedef struct vicii_sprite_s vicii_sprite_t;

struct video_chip_cap_s;

struct vicii_s {
    /* Flag: Are we initialized?  */
    int initialized;            /* = 0; */

    /* VIC-II raster.  */
    raster_t raster;

    /* VIC-II registers.  */
    int regs[0x40];

    /* Cycle # within the current line.  */
    unsigned int raster_cycle;
                                   
    /* Current line.  */
    unsigned int raster_line;

    /* Interrupt register.  */
    int irq_status;             /* = 0; */

    /* Line for raster compare IRQ.  */
    unsigned int raster_irq_line;

    /* Pointer to the base of RAM seen by the VIC-II.  */
    /* address is base of 64k bank. vbank adds 0/16k/32k/48k to get actual
       video address */
    BYTE *ram_base_phi1;                /* = VIC-II address during Phi1; */
    BYTE *ram_base_phi2;                /* = VIC-II address during Phi2; */

    /* valid VIC-II address bits for Phi1 and Phi2. After masking
       the address, it is or'd with the offset value to set always-1 bits */
    WORD vaddr_mask_phi1;            /* mask of valid address bits */
    WORD vaddr_mask_phi2;            /* mask of valid address bits */
    WORD vaddr_offset_phi1;          /* mask of address bits always set */
    WORD vaddr_offset_phi2;          /* mask of address bits always set */

    /* Those two values determine where in the address space the chargen
       ROM is mapped. Use mask=0x7000, value=0x1000 for the C64. */
    WORD vaddr_chargen_mask_phi1;    /* address bits to comp. for chargen */
    WORD vaddr_chargen_mask_phi2;    /* address bits to comp. for chargen */
    WORD vaddr_chargen_value_phi1;   /* compare value for chargen */
    WORD vaddr_chargen_value_phi2;   /* compare value for chargen */

    /* Video memory pointers.  Changed for drawing.  */
    BYTE *screen_ptr;
    BYTE *chargen_ptr;

    /* Pointer to the bitmap (lower part)  */
    BYTE *bitmap_low_ptr;

    /* Pointer to the bitmap (higher part)  */
    BYTE *bitmap_high_ptr;

    /* Video memory pointers.  Changed immediately.  */
    BYTE *screen_base_phi1;
    BYTE *screen_base_phi2;

    /* Offset to the vbuf/cbuf buffer */
    int buf_offset;

    /* Screen memory buffers (chars and color).  */
    BYTE vbuf[VICII_SCREEN_TEXTCOLS];
    BYTE cbuf[VICII_SCREEN_TEXTCOLS];

    /* Graphics buffer (bitmap/LinearB) */
    BYTE gbuf;

    /* Current rendering position into the draw buffer */
    int dbuf_offset;

    /* Draw buffer for a full line (one byte per pixel) */
    BYTE dbuf[VICII_DRAW_BUFFER_SIZE];

    /* Flags related to top/bottom border (previously in raster) */
    unsigned int display_ystart;
    unsigned int display_ystop;

    /* parsed vicii register fields */
    unsigned int ysmooth;

    /* If this flag is set, bad lines (DMA's) can happen.  */
    int allow_bad_lines;

    /* Sprite-sprite and sprite-background collision registers.  */
    BYTE sprite_sprite_collisions;
    BYTE sprite_background_collisions;

    /* Extended background colors (1, 2 and 3).  */
    int ext_background_color[3];

    /* Current video mode.  */
    int video_mode;

    /* Flag: are we in idle state? */
    int idle_state;

    /* Flag: should we force display (i.e. non-idle) state for the following
       line? */
    int force_display_state;

    /* Internal memory pointer (VCBASE).  */
    int memptr;

    /* Internal memory counter (VC).  */
    int mem_counter;

    /* Flag: is the current line a `bad' line? */
    int bad_line;

    /* Flag: Does the currently selected video mode force the overscan
       background color to be black?  (This happens with the hires bitmap and
       illegal modes.)  */
    int force_black_overscan_background_color;

    /* Background color source */
    int background_color_source;

    /* Light pen.  */
    vicii_light_pen_t light_pen;

    /* Start of the memory bank seen by the VIC-II.  */
    int vbank_phi1;                     /* = 0; */
    int vbank_phi2;                     /* = 0; */

    /* All the VIC-II logging goes here.  */
    signed int log;

    /* Fetch state */
    int fetch_active;
    int prefetch_cycles;

    /* Number of sprite being DMA fetched.  */
    unsigned int sprite_fetch_idx;

    /* Mask for sprites being fetched at DMA.  */
    unsigned int sprite_fetch_msk;

    /* Mask for sprites being displayed.  */
    unsigned int sprite_display_bits;

    /* State of sprites. */
    vicii_sprite_t sprite[VICII_NUM_SPRITES];

    /* Geometry and timing parameters of the selected VIC-II emulation.  */
    unsigned int screen_height;
    int first_displayed_line;
    int last_displayed_line;

    unsigned int row_25_start_line;
    unsigned int row_25_stop_line;
    unsigned int row_24_start_line;
    unsigned int row_24_stop_line;

    int screen_leftborderwidth;
    int screen_rightborderwidth;
    int cycles_per_line;
    int draw_cycle;
    int sprite_fetch_cycle;
    int sprite_wrap_x;

    unsigned int first_dma_line;
    unsigned int last_dma_line;

    /* Flag backgroundcolor in hires mode or extended text mode.  */
    int get_background_from_vbuf;

    /* Last value read from VICII (used for RMW access).  */
    BYTE last_read;

    /* Last value read by VICII during phi1.  */
    BYTE last_read_phi1;

    /* Counter used for DRAM refresh accesses.  */
    BYTE refresh_counter;

    /* Video chip capabilities.  */
    struct video_chip_cap_s *video_chip_cap;

    unsigned int int_num;

    /* Pointer to color ram */
    BYTE *color_ram_ptr;
};
typedef struct vicii_s vicii_t;

extern vicii_t vicii;

/* Private function calls, used by the other VIC-II modules.  */
extern void vicii_update_memory_ptrs(unsigned int cycle);
extern void vicii_update_video_mode(unsigned int cycle);
extern void vicii_raster_draw_alarm_handler(CLOCK offset, void *data);
extern void vicii_handle_pending_alarms(int num_write_cycles);
extern void vicii_delay_clk(void);
extern void vicii_delay_oldclk(CLOCK num);

/* Debugging options.  */

/* #define VICII_VMODE_DEBUG */
/* #define VICII_RASTER_DEBUG */
/* #define VICII_REGISTERS_DEBUG */
#define VICII_CYCLE_DEBUG

#ifdef VICII_VMODE_DEBUG
#define VICII_DEBUG_VMODE(x) log_debug x
#else
#define VICII_DEBUG_VMODE(x)
#endif

#ifdef VICII_RASTER_DEBUG
#define VICII_DEBUG_RASTER(x) log_debug x
#else
#define VICII_DEBUG_RASTER(x)
#endif

#ifdef VICII_REGISTERS_DEBUG
#define VICII_DEBUG_REGISTER(x) log_debug x
#else
#define VICII_DEBUG_REGISTER(x)
#endif

#ifdef VICII_CYCLE_DEBUG
#define VICII_DEBUG_CYCLE(x) log_debug x
#else
#define VICII_DEBUG_CYCLE(x)
#endif

#endif

