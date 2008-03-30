/*
 * ieeevia2.c - IEEE488 interface VIA2 emulation in the VIC-1112.
 *
 * Written by
 *   Andr� Fachat <a.fachat@physik.tu-chemnitz.de>
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

#define mycpu maincpu
#define myclk maincpu_clk
#define myvia ieeevia2
#define myvia_init ieeevia2_init

#define via_set_int maincpu_set_irq
#define MYVIA_INT IK_IRQ
#define MYVIA_NAME "IeeeVia2"

#define mycpu_rmw_flag maincpu_rmw_flag
#define mycpu_int_status maincpu_int_status
#define mycpu_alarm_context maincpu_alarm_context
#define mycpu_clk_guard maincpu_clk_guard

#define myvia_reset ieeevia2_reset
#define myvia_store ieeevia2_store
#define myvia_read ieeevia2_read
#define myvia_peek ieeevia2_peek

#define myvia_log ieeevia2_log
#define myvia_signal ieeevia2_signal
#define myvia_prevent_clk_overflow ieeevia2_prevent_clk_overflow
#define myvia_snapshot_read_module ieeevia2_snapshot_read_module
#define myvia_snapshot_write_module ieeevia2_snapshot_write_module

#include "vice.h"

#include "drive.h"
#include "drivecpu.h"
#include "maincpu.h"
#include "parallel.h"
#include "types.h"
#include "viacore.h"


static void via_set_ca2(int state)
{
    parallel_cpu_set_atn((char)(state ? 0 : 1));
}

static void via_set_cb2(int state)
{
    parallel_cpu_set_eoi((BYTE)(state ? 0 : 1));
}

/* #define VIA1_TIMER_DEBUG */

static char snap_module_name[] = MYVIA_NAME;

static void undump_acr(BYTE byte)
{
}

inline void static store_acr(BYTE byte)
{
}

inline void static store_sr(BYTE byte)
{
}

inline void static store_t2l(BYTE byte)
{
}

static void undump_pra(BYTE byte)
{
    parallel_cpu_set_bus(byte);
}

inline static void store_pra(BYTE byte, BYTE myoldpa, WORD addr)
{
    parallel_cpu_set_bus(byte);
}

static void undump_prb(BYTE byte)
{
}

inline static void store_prb(BYTE byte, BYTE myoldpb, WORD addr)
{
}

static void undump_pcr(BYTE byte)
{
}

static void res_via(void)
{
    parallel_cpu_set_bus(0xff); /* all data lines high, because of input mode */
}

inline static BYTE store_pcr(BYTE byte, WORD addr)
{
#if 0
    if (byte != myvia[VIA_PCR]) {
        register BYTE tmp = byte;
        /* first set bit 1 and 5 to the real output values */
        if((tmp & 0x0c) != 0x0c) tmp |= 0x02;
        if((tmp & 0xc0) != 0xc0) tmp |= 0x20;
        parallel_cpu_set_atn( (byte & 2) ? 0 : 1 );
        parallel_cpu_set_eoi( (byte & 0x20) ? 0 : 1 );
    }
#endif
    return byte;
}

inline static BYTE read_prb(void)
{
    BYTE byte;

    if (drive[0].enable)
        drive0_cpu_execute(maincpu_clk);
    if (drive[1].enable)
        drive1_cpu_execute(maincpu_clk);

    byte = (parallel_bus & ~myvia[VIA_DDRB])
           | (myvia[VIA_PRB] & myvia[VIA_DDRB]);
    return byte;
}

inline static BYTE read_pra(WORD addr)
{
    return 0xff;
}

#include "viacore.c"

