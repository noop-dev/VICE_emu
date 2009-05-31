/*
 * c64dtvcpu.c - Emulation of the C64DTV processor.
 *
 * Written by
 *  M.Kiesel <mayne@users.sourceforge.net>
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#include "alarm.h"
#include "c64mem.h"
#include "c64dtvblitter.h"
#include "c64dtvdma.h"
#include "monitor.h"
#include "viciidtv.h"

/* ------------------------------------------------------------------------- */

/* MACHINE_STUFF should define/undef

 - NEED_REG_PC
 - TRACE

 The following are optional:

 - PAGE_ZERO
 - PAGE_ONE
 - STORE_IND
 - LOAD_IND
 - DMA_FUNC
 - DMA_ON_RESET

*/

/* ------------------------------------------------------------------------- */

#ifndef C64DTV
#define C64DTV
#endif

/* TODO this is a hack */
#undef C64_RAM_SIZE
#define C64_RAM_SIZE 0x200000

BYTE dtv_registers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
BYTE dtvrewind;

/* Global clock counter.  */
CLOCK maincpu_clk = 0L;

#define REWIND_FETCH_OPCODE(clock) /*clock-=dtvrewind;*/

/* Flag: BA low */
int maincpu_ba_low_flag = 0;

/* Burst mode implementation */

#define BURST_MODE (dtv_registers[9]&2)

BYTE burst_status, burst_idx, burst_fetch, burst_broken;
BYTE burst_cache[] = {0, 0, 0, 0};
WORD burst_addr, burst_last_addr;


static void c64dtvcpu_clock_inc(void)
{
    maincpu_clk++;
    maincpu_ba_low_flag = viciidtv_cycle_1_2();

    if (!maincpu_ba_low_flag) {
        if (blitter_active) {
            c64dtvblitter_perform_blitter();
        } else if (dma_active) {
            c64dtvdma_perform_dma();
        }
    }

    viciidtv_cycle_3();
}

#define CLK_INC() c64dtvcpu_clock_inc()

#define CLK_ADD(clock, amount) c64dtvcpu_clock_add(&clock, amount)

/* This is an optimization making x64dtv consume less host cycles in burst mode. */
DWORD mem_burst_read(WORD addr)
{
    read_func_ptr_t mrtf;
    int paddr = ((((int) dtv_registers[12 + (addr >> 14)]) << 14) + (addr & 0x3fff)) & (C64_RAM_SIZE - 1);

    if(paddr <= 0xffff) {
        mrtf = _mem_read_tab_ptr[paddr >> 8];
        if(mrtf != ram_read) {
#ifdef WORDS_BIGENDIAN
            return (((DWORD) mrtf(paddr + 0)) << 24) +
                   (((DWORD) mrtf(paddr + 1)) << 16) +
                   (((DWORD) mrtf(paddr + 2)) << 8) +
                   (((DWORD) mrtf(paddr + 3)) << 0);
#else
            return (((DWORD) mrtf((WORD)(paddr + 3))) << 24) +
                   (((DWORD) mrtf((WORD)(paddr + 2))) << 16) +
                   (((DWORD) mrtf((WORD)(paddr + 1))) << 8) +
                   (((DWORD) mrtf((WORD)(paddr + 0))) << 0);
#endif
        }
    }
    return *((DWORD *) (&mem_ram[paddr]));
}

/* Burst mode & skip cycle helper table */
/* format: SBDDDFFF */
/*  FFF = "fetch", opcode length-1 */
/*  DDD = "diff", opcode execution time difference */
/*    B = "break", breaks burst execution (memory access, jump, ...) */
/*    S = "skip", skip loading operand (for implied<>immediate in skip cycle) */

static const BYTE burst_status_tab[] = {
            /* 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F */
    /* $00 */  0310, 0121, 0100, 0121, 0121, 0121, 0121, 0121, 0310, 0021, 0210, 0021, 0132, 0132, 0132, 0132, /* $00 */
    /* $10 */  0021, 0121, 0021, 0121, 0121, 0121, 0121, 0121, 0210, 0132, 0210, 0132, 0132, 0132, 0132, 0132, /* $10 */
    /* $20 */  0021, 0121, 0100, 0121, 0121, 0121, 0121, 0121, 0310, 0021, 0210, 0021, 0132, 0132, 0132, 0132, /* $20 */
    /* $30 */  0021, 0121, 0021, 0121, 0121, 0121, 0121, 0121, 0210, 0132, 0210, 0132, 0132, 0132, 0132, 0132, /* $30 */
    /* $40 */  0310, 0121, 0021, 0121, 0121, 0121, 0121, 0121, 0310, 0021, 0210, 0021, 0132, 0132, 0132, 0132, /* $40 */
    /* $50 */  0021, 0121, 0100, 0121, 0121, 0121, 0121, 0121, 0210, 0132, 0210, 0132, 0132, 0132, 0132, 0132, /* $50 */
    /* $60 */  0310, 0121, 0100, 0121, 0121, 0121, 0121, 0121, 0310, 0021, 0210, 0021, 0132, 0132, 0132, 0132, /* $60 */
    /* $70 */  0021, 0121, 0100, 0121, 0121, 0121, 0121, 0121, 0210, 0132, 0210, 0132, 0132, 0132, 0132, 0132, /* $70 */
    /* $80 */  0021, 0121, 0021, 0121, 0121, 0121, 0121, 0121, 0210, 0021, 0210, 0021, 0132, 0132, 0132, 0132, /* $80 */
    /* $90 */  0021, 0121, 0100, 0121, 0121, 0121, 0121, 0121, 0210, 0132, 0210, 0132, 0132, 0132, 0132, 0132, /* $90 */
    /* $A0 */  0021, 0121, 0021, 0121, 0121, 0121, 0121, 0121, 0210, 0021, 0210, 0021, 0132, 0132, 0132, 0132, /* $A0 */
    /* $B0 */  0021, 0121, 0100, 0121, 0121, 0121, 0121, 0121, 0210, 0132, 0210, 0132, 0132, 0132, 0132, 0132, /* $B0 */
    /* $C0 */  0021, 0121, 0021, 0121, 0121, 0121, 0121, 0121, 0210, 0021, 0210, 0021, 0132, 0132, 0132, 0132, /* $C0 */
    /* $D0 */  0021, 0121, 0100, 0121, 0121, 0121, 0121, 0121, 0210, 0132, 0210, 0132, 0132, 0132, 0132, 0132, /* $D0 */
    /* $E0 */  0021, 0121, 0021, 0121, 0121, 0121, 0121, 0121, 0210, 0021, 0210, 0021, 0132, 0132, 0132, 0132, /* $E0 */
    /* $F0 */  0021, 0121, 0100, 0121, 0121, 0121, 0121, 0121, 0210, 0132, 0210, 0132, 0132, 0132, 0132, 0132  /* $F0 */
};


/* Skip cycle implementation */

#define SKIP_CYCLE (dtv_registers[9]&1)


/* Override optimizations in maincpu.c that directly access mem_ram[] */
/* We need to channel everything through mem_read/mem_store to */
/* let the DTV segment mapper (register 12-15) do its work */

#define STORE(addr, value) \
    mem_store((WORD)(addr), (BYTE)(value))

#define LOAD(addr) \
    mem_read((WORD)(addr))

/* Route zero page operations through register 10 (zeropage mapper) */

#define STORE_ZERO(addr, value) \
    mem_store((WORD)((((WORD) dtv_registers[10]) << 8) + ((WORD)(addr) & 0xff)), (BYTE)(value))

#define LOAD_ZERO(addr) \
    mem_read((WORD)((((WORD) dtv_registers[10]) << 8) + ((WORD)((addr) & 0xff))))

/* Route stack operations through register 11 (stack mapper) */

#define PUSH(val) (STORE((((WORD) dtv_registers[11]) << 8) + ((reg_sp--) & 0xff), val))

#define PULL()    (LOAD((((WORD) dtv_registers[11]) << 8) + ((++reg_sp) & 0xff)))

#define STACK_PEEK()  (LOAD((((WORD) dtv_registers[11]) << 8) + ((reg_sp) & 0xff)))

/* opcode_t etc */

#if defined ALLOW_UNALIGNED_ACCESS

#define opcode_t DWORD

#define p0 (opcode & 0xff)
#define p1 ((opcode >> 8) & 0xff)
#define p2 (opcode >> 8)

#else /* !ALLOW_UNALIGNED_ACCESS */

#define opcode_t          \
    struct {              \
        BYTE ins;         \
        union {           \
            BYTE op8[2];  \
            WORD op16;    \
        } op;             \
    }

#define p0 (opcode.ins)
#define p2 (opcode.op.op16)

#ifdef WORDS_BIGENDIAN
#  define p1 (opcode.op.op8[1])
#else
#  define p1 (opcode.op.op8[0])
#endif

#endif /* !ALLOW_UNALIGNED_ACCESS */

/*  SET_OPCODE for traps */
#if defined ALLOW_UNALIGNED_ACCESS
#define SET_OPCODE(o) (opcode) = o;
#else 
#if !defined WORDS_BIGENDIAN
#define SET_OPCODE(o)                          \
    do {                                       \
        opcode.ins = (o) & 0xff;               \
        opcode.op.op8[0] = ((o) >> 8) & 0xff;  \
        opcode.op.op8[1] = ((o) >> 16) & 0xff; \
    } while (0)
#else
#define SET_OPCODE(o)                          \
    do {                                       \
        opcode.ins = (o) & 0xff;               \
        opcode.op.op8[1] = ((o) >> 8) & 0xff;  \
        opcode.op.op8[0] = ((o) >> 16) & 0xff; \
    } while (0)
#endif
#endif


/* FETCH_OPCODE_DTV implementation(s) */

#if defined ALLOW_UNALIGNED_ACCESS
#define FETCH_OPCODE(o) \
    do { \
        dtvrewind = 0; \
        if (BURST_MODE && (((dtv_registers[8] >> ((reg_pc >> 13)&6)) & 0x03) == 0x01)) { \
            burst_last_addr = burst_addr; \
            burst_addr = reg_pc & 0xfffc; \
            if ((burst_addr != burst_last_addr) || burst_broken) { \
                *((DWORD *)burst_cache) = mem_burst_read(burst_addr); \
                CLK_INC(); \
            } \
            burst_idx = reg_pc & 3; \
            o = burst_cache[burst_idx++]; \
            burst_status = burst_status_tab[o]; \
            burst_fetch = burst_status & 7; \
            burst_broken = (burst_status >> 6)&1; \
            if (burst_fetch--) { \
                if (burst_idx>3) { \
                    burst_addr += 4; \
                    burst_addr &= 0xfffc; \
                    burst_last_addr = burst_addr; \
                    burst_idx = 0; \
                    *((DWORD *)burst_cache) = mem_burst_read(burst_addr); \
                    CLK_INC(); \
                } \
                o |= (burst_cache[burst_idx++] << 8); \
                if (burst_fetch--) { \
                    if (burst_idx>3) { \
                        burst_addr += 4; \
                        burst_addr &= 0xfffc; \
                        burst_last_addr = burst_addr; \
                        burst_idx = 0; \
                        *((DWORD *)burst_cache) = mem_burst_read(burst_addr); \
                        CLK_INC(); \
                    } \
                    o |= (burst_cache[burst_idx] << 16); \
                } \
            } \
        } else { \
            burst_broken = 1; \
            o = LOAD(reg_pc);                                   \
            CLK_INC(); dtvrewind++;                             \
            o |= LOAD(reg_pc + 1) << 8;                         \
            if (!(((burst_status_tab[o & 0xff]&0x80))           \
               && SKIP_CYCLE)) {                                \
                CLK_INC(); dtvrewind++;                         \
            }                                                   \
            if (fetch_tab[o & 0xff]) {                          \
                 o |= (LOAD(reg_pc + 2) << 16);                 \
                 CLK_INC();                                     \
            }                                                   \
        } \
    } while(0)

#else /* !ALLOW_UNALIGNED_ACCESS */
/* TODO optimize */
#define FETCH_OPCODE(o) \
    do { \
        dtvrewind = 0; \
        if((dtv_registers[9]&2)&&(((dtv_registers[8] >> ((reg_pc >> 13)&6)) & 0x03) == 0x01)) { \
            burst_last_addr = burst_addr; \
            burst_addr = reg_pc & 0xfffc; \
            if((burst_addr != burst_last_addr)||burst_broken) { \
                burst_cache[0] = LOAD(burst_addr+0); \
                burst_cache[1] = LOAD(burst_addr+1); \
                burst_cache[2] = LOAD(burst_addr+2); \
                burst_cache[3] = LOAD(burst_addr+3); \
                CLK_INC(); \
            } \
            burst_idx = reg_pc & 3; \
            (o).ins = burst_cache[burst_idx++]; \
            burst_status = burst_status_tab[(o).ins]; \
            burst_fetch = burst_status & 7; \
            burst_broken = (burst_status >> 6)&1; \
            if(burst_fetch--) { \
                if(burst_idx>3) { \
                    burst_addr += 4; \
                    burst_addr &= 0xfffc; \
                    burst_last_addr = burst_addr; \
                    burst_idx = 0; \
                    burst_cache[0] = LOAD(burst_addr+0); \
                    burst_cache[1] = LOAD(burst_addr+1); \
                    burst_cache[2] = LOAD(burst_addr+2); \
                    burst_cache[3] = LOAD(burst_addr+3); \
                    CLK_INC(); \
                } \
                (o).op.op16 = burst_cache[burst_idx++]; \
                if(burst_fetch--) { \
                    if(burst_idx>3) { \
                        burst_addr += 4; \
                        burst_addr &= 0xfffc; \
                        burst_last_addr = burst_addr; \
                        burst_idx = 0; \
                        burst_cache[0] = LOAD(burst_addr+0); \
                        burst_cache[1] = LOAD(burst_addr+1); \
                        burst_cache[2] = LOAD(burst_addr+2); \
                        burst_cache[3] = LOAD(burst_addr+3); \
                        CLK_INC(); \
                    } \
                    (o).op.op16 |= (burst_cache[burst_idx] << 8); \
                } \
            } \
        } else { \
            burst_broken=1; \
            (o).ins = LOAD(reg_pc);                             \
            CLK_INC(); dtvrewind++;                             \
            (o).op.op16 = LOAD(reg_pc + 1);                     \
            if (!(((burst_status_tab[(o).ins]&0x80))            \
               && SKIP_CYCLE)) {                                \
                CLK_INC(); dtvrewind++;                         \
            }                                                   \
            if (fetch_tab[(o).ins]) {                           \
                 (o).op.op16 |= (LOAD(reg_pc + 2) << 8);        \
                 CLK_INC();                                     \
            }                                                   \
        } \
    } while(0)

#endif /* !ALLOW_UNALIGNED_ACCESS */


#include "../maindtvcpu.c"

