/*
 * finalplus.c - Cartridge handling, Final cart.
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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
#include <stdlib.h>
#include <string.h>

#include "c64cart.h"
#include "c64cartmem.h"
#include "c64export.h"
#include "c64io.h"
#include "c64mem.h"
#include "cartridge.h"
#include "finalplus.h"
#include "machine.h"
#include "maincpu.h"
#include "types.h"
#include "util.h"

/* #define DEBUGFC */

#ifdef DEBUGFC
#define DBG(x) printf x
#else
#define DBG(x)
#endif

/*
    Final Cartridge Plus

    - reset button, cart enable switch
    - 32K ROM, of which 24K are used

        EPROM $2000-$4000 is visible at $e000-$ffff, if enabled
        EPROM $4000-$5fff is visible at $8000-$9fff, if enabled
        EPROM $6000-$7fff is visible at $a000-$bfff, if enabled

An NMI can is triggered by the cart, if address $0001 is written to and the cartridge is enabled.

The cart can also be disabled of by software, writing clearing bit 4 when writing to $df00-$dfff.
cart ROM at $e000-$ffff can be disabled by setting bit 5 to "zero" when writing to $df00-$dfff.
cart ROM at $8000-$bfff can be disabled by setting bit 6 to "one" when writing to $df00-$dfff.
Bit 7 of a byte written to $df00-$dfff can be read back from the cartridge if enabled (kind of a memory cell).

*/

static int fcplus_enabled;
static int fcplus_bit;
static int fcplus_roml;
static int fcplus_romh;

/* some prototypes are needed */
static BYTE REGPARM1 final_plus_io2_read(WORD addr);
static void REGPARM2 final_plus_io2_store(WORD addr, BYTE value);

static io_source_t final_plus_io2_device = {
    "Final Plus",
    IO_DETACH_CART,
    NULL,
    0xdf00, 0xdfff, 0xff,
    1, /* read is always valid */
    final_plus_io2_store,
    final_plus_io2_read,
    NULL, /* TODO: peek */
    NULL, /* TODO: dump */
    CARTRIDGE_FINAL_PLUS
};

static io_source_list_t *final_plus_io2_list_item = NULL;

static const c64export_resource_t export_res_plus = {
    "Final Plus", 1, 1, NULL, &final_plus_io2_device, CARTRIDGE_FINAL_PLUS
};

/* ---------------------------------------------------------------------*/

BYTE REGPARM1 final_plus_io2_read(WORD addr)
{
    DBG(("io2 r %04x\n",addr));
    return ((fcplus_bit << 7) || (fcplus_roml << 6) || (fcplus_romh << 5) || (fcplus_enabled << 4));
}

void REGPARM2 final_plus_io2_store(WORD addr, BYTE value)
{
    if (fcplus_enabled == 1) {
        fcplus_bit = (value >> 7) & 1;
        fcplus_roml = ((value >> 6) & 1) ^ 1;
        fcplus_romh = ((value >> 5) & 1);
        /* fcplus_enabled = ((value >> 4) & 1) ^ 1; */
        DBG(("io2 w %04x %02x (bit:%d, rom8000:%d, romE000:%d, enabled: %d)\n",addr,value,fcplus_bit,fcplus_roml,fcplus_romh,fcplus_enabled));

        if ((fcplus_roml == 0) && (fcplus_romh == 0)) {
            cartridge_config_changed(2, 2, CMODE_WRITE);
        } else {
            cartridge_config_changed(0, 3, CMODE_WRITE | CMODE_PHI2_RAM);
        }
    }
}

/* ---------------------------------------------------------------------*/

BYTE REGPARM1 final_plus_roml_read(WORD addr)
{
    if (fcplus_roml == 1) {
        return roml_banks[(addr & 0x1fff)];
    } else {
        return mem_read_without_ultimax(addr);
    }
}

BYTE REGPARM1 final_plus_romh_read(WORD addr)
{
    if ((fcplus_enabled == 1) && (fcplus_romh == 1)) {
        return romh_banks[(addr & 0x1fff)];
    } else {
        return mem_read_without_ultimax(addr);
    }
}

BYTE REGPARM1 final_plus_a000_bfff_read(WORD addr)
{
    if ((fcplus_enabled == 1) && (fcplus_roml == 1)) {
        return roml_banks[0x2000 + (addr & 0x1fff)];
    } else {
        return mem_read_without_ultimax(addr);
    }
}

/* ---------------------------------------------------------------------*/

void final_plus_freeze(void)
{
    DBG(("fc+ freeze\n"));
    cartridge_config_changed(0, 3, CMODE_READ | CMODE_RELEASE_FREEZE | CMODE_PHI2_RAM);
    fcplus_enabled = 1;
    fcplus_roml = 1;
    fcplus_romh = 1;
}

void final_plus_config_init(void)
{
    DBG(("fc+ config init\n"));
    cartridge_config_changed(0, 3, CMODE_READ | CMODE_PHI2_RAM);
    fcplus_enabled = 1;
    fcplus_roml = 1;
    fcplus_romh = 1;
}
/*
EPROM $2000-$4000 is visible at $e000-$ffff, if enabled
EPROM $4000-$5fff is visible at $8000-$9fff, if enabled
EPROM $6000-$7fff is visible at $a000-$bfff, if enabled
*/
void final_plus_config_setup(BYTE *rawcart)
{
    DBG(("fc+ config setup\n"));
    memcpy(roml_banks, &rawcart[0x4000], 0x4000);
    memcpy(romh_banks, &rawcart[0x2000], 0x2000);
    cartridge_config_changed(0, 3, CMODE_READ | CMODE_PHI2_RAM);
}

/* ---------------------------------------------------------------------*/

static int final_plus_common_attach(void)
{
    if (c64export_add(&export_res_plus) < 0) {
        return -1;
    }

    final_plus_io2_list_item = c64io_register(&final_plus_io2_device);

    return 0;
}

int final_plus_bin_attach(const char *filename, BYTE *rawcart)
{
    DBG(("fc+ bin attach\n"));

    /* accept 32k and 24k binaries */
    if (util_file_load(filename, rawcart, 0x8000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        if (util_file_load(filename, rawcart, 0x6000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
            return -1;
        }
        memmove(&rawcart[0x2000],rawcart,0x6000);
    }

    return final_plus_common_attach();
}

int final_plus_crt_attach(FILE *fd, BYTE *rawcart)
{
    BYTE chipheader[0x10];

    DBG(("fc+ crt attach\n"));
    if (fread(chipheader, 0x10, 1, fd) < 1) {
        return -1;
    }

    if (chipheader[0xe] != 0x80) {
        return -1;
    }

    if (fread(rawcart, chipheader[0xe] << 8, 1, fd) < 1) {
        return -1;
    }

    return final_plus_common_attach();
}

void final_plus_detach(void)
{
    c64export_remove(&export_res_plus);
    c64io_unregister(final_plus_io2_list_item);
    final_plus_io2_list_item = NULL;
}
