/*
 * plus4.c
 *
 * Written by
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

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>

#include "asm.h"
#include "attach.h"
#include "autostart.h"
#include "clkguard.h"
#include "console.h"
#include "datasette.h"
#include "debug.h"
#include "drive-cmdline-options.h"
#include "drive-resources.h"
#include "drive-snapshot.h"
#include "drive.h"
#include "drivecpu.h"
#include "iecdrive.h"
#include "interrupt.h"
#include "kbdbuf.h"
#include "keyboard.h"
#include "log.h"
#include "machine.h"
#include "maincpu.h"
#include "mon.h"
#include "plus4-cmdline-options.h"
#include "plus4-resources.h"
#include "plus4-snapshot.h"
#include "plus4.h"
#include "plus4acia.h"
#include "plus4mem.h"
#include "plus4ui.h"
#include "printer.h"
#include "screenshot.h"
#include "serial.h"
#include "snapshot.h"
#include "sound.h"
#include "tape.h"
#include "ted-cmdline-options.h"
#include "ted-resources.h"
#include "ted.h"
#include "traps.h"
#include "types.h"
#include "utils.h"
#include "video.h"
#include "vsync.h"


#define NUM_KEYBOARD_MAPPINGS 2

const char *machine_keymap_res_name_list[NUM_KEYBOARD_MAPPINGS] = {
    "KeymapSymFile", "KeymapPosFile"
};

char *machine_keymap_file_list[NUM_KEYBOARD_MAPPINGS] = {
    NULL, NULL
};

const char machine_name[] = "PLUS4";
int machine_class = VICE_MACHINE_PLUS4;

static void machine_vsync_hook(void);

/* ------------------------------------------------------------------------- */

static trap_t plus4_serial_traps[] = {
    {
        "SerialListen",
        0xE16B,
        0xE1E7,
        { 0x20, 0xC6, 0xE2 },
        serialattention,
        kernal_read,
        kernal_store
    },
    {
        "SerialSaListen",
        0xE177,
        0xE1E7,
        { 0x78, 0x20, 0xBF },
        serialattention,
        kernal_read,
        kernal_store
    },
    {
        "SerialSendByte",
        0xE181,
        0xE1E7,
        { 0x78, 0x20, 0xC6 },
        serialsendbyte,
        kernal_read,
        kernal_store
    },
/*
    {
        "SerialSendByte2",
        0xE158,
        0xE1E7,
        { 0x48, 0x24, 0x94 },
        serialsendbyte,
        kernal_read,
        kernal_store
    },
*/
    {
        "SerialReceiveByte",
        0xE252,
        0xE1E7,
        { 0x78, 0xA9, 0x00 },
        serialreceivebyte,
        kernal_read,
        kernal_store
    },
    {
        "SerialReady",
        0xE216,
        0xE1E7,
        { 0x24, 0x01, 0x70 },
        trap_serial_ready,
        kernal_read,
        kernal_store
    },
    {
        "SerialReady2",
        0xE2D4,
        0xE1E7,
        { 0xA5, 0x01, 0xC5 },
        trap_serial_ready,
        kernal_read,
        kernal_store
    },
    {
        NULL,
        0,
        0,
        { 0, 0, 0 },
        NULL,
        NULL,
        NULL
    }
};

/* Tape traps.  */
static trap_t plus4_tape_traps[] = {
    {
        "TapeFindHeader",
        0xE9CC,
        0xE9CF,
        { 0x20, 0xD3, 0xE8 },
        tape_find_header_trap_plus4,
        rom_read,
        rom_store
    },
    {
        "TapeReceive",
        0xE74B,
        0xE8C7,
        { 0xBA, 0x8E, 0xBE },
        tape_receive_trap_plus4,
        rom_read,
        rom_store
    },
    {
        NULL,
        0,
        0,
        { 0, 0, 0 },
        NULL,
        NULL,
        NULL
    }
};

static tape_init_t tapeinit = {
    0x0333,
    0x90,
    0x93,
    0x0000,
    0,
    0xb4,
    0x9d,
    0x527,
    0xef,
    plus4_tape_traps,
    36 * 8,
    54 * 8,
    55 * 8,
    73 * 8,
    74 * 8,
    100 * 8
};

static log_t plus4_log = LOG_ERR;

static long cycles_per_sec = PLUS4_PAL_CYCLES_PER_SEC;
static long cycles_per_rfsh = PLUS4_PAL_CYCLES_PER_RFSH;
static double rfsh_per_sec = PLUS4_PAL_RFSH_PER_SEC;

/* Plus4-specific resource initialization.  This is called before initializing
   the machine itself with `machine_init()'.  */
int machine_resources_init(void)
{
    if (traps_resources_init() < 0
        || vsync_resources_init() < 0
        || video_resources_init(VIDEO_RESOURCES_PAL_NOFAKE) < 0
        || plus4_resources_init() < 0
        || ted_resources_init() < 0
        || sound_resources_init() < 0
#ifdef HAVE_RS232
        || acia_resources_init() < 0
#endif
        || printer_resources_init() < 0
#ifndef COMMON_KBD
        || kbd_resources_init() < 0
#endif
        || drive_resources_init() < 0
        || datasette_resources_init() < 0
        )
        return -1;

    return 0;
}

/* Plus4-specific command-line option initialization.  */
int machine_cmdline_options_init(void)
{
    if (traps_cmdline_options_init() < 0
        || vsync_cmdline_options_init() < 0
        || video_init_cmdline_options() < 0
        || plus4_cmdline_options_init() < 0
        || ted_cmdline_options_init() < 0
        || sound_cmdline_options_init() < 0
#ifdef HAVE_RS232
        || acia_cmdline_options_init() < 0
#endif
        || printer_cmdline_options_init() < 0
#ifndef COMMON_KBD
        || kbd_cmdline_options_init() < 0
#endif
        || drive_cmdline_options_init() < 0
        || datasette_cmdline_options_init() < 0
        )
        return -1;

    return 0;
}

static void plus4_monitor_init(void)
{
    monitor_cpu_type_t asm6502;
    monitor_cpu_type_t *asmarray[2];

    asm6502_init(&asm6502);

    asmarray[0] = &asm6502;
    asmarray[1] = NULL;

    /* Initialize the monitor.  */
    monitor_init(&maincpu_monitor_interface, drive0_get_monitor_interface_ptr(),
                 drive1_get_monitor_interface_ptr(), asmarray);
}

/* Plus4-specific initialization.  */
int machine_init(void)
{
    plus4_log = log_open("Plus4");

    if (mem_load() < 0)
        return -1;

    /* Setup trap handling.  */
    traps_init();

    /* Initialize serial traps.  */
    if (serial_init(plus4_serial_traps, 0xa8) < 0)
        return -1;

    /* Initialize drives. */
    file_system_init();

    /* Initialize print devices.  */
    printer_init();

    /* Initialize the tape emulation.  */
    tape_init(&tapeinit);

    /* Initialize the datasette emulation.  */
    datasette_init();

    /* Fire up the hardware-level drive emulation.  */
    drive_init(PLUS4_PAL_CYCLES_PER_SEC, PLUS4_PAL_CYCLES_PER_SEC);

    /* Initialize autostart.  */
    autostart_init((CLOCK)(2 * rfsh_per_sec * cycles_per_rfsh),
                   0, 0, 0xc8, 0xca, -40);

    if (!ted_init())
        return -1;

#ifdef HAVE_RS232
    acia_init();
#endif

#ifndef COMMON_KBD
    if (plus4_kbd_init() < 0)
        return -1;
#endif

    plus4_monitor_init();

    /* Initialize vsync and register our hook function.  */
    vsync_init(machine_vsync_hook);
    vsync_set_machine_parameter(rfsh_per_sec, cycles_per_sec);

    /* Initialize sound.  Notice that this does not really open the audio
       device yet.  */
    sound_init(cycles_per_sec, cycles_per_rfsh);

    /* Initialize keyboard buffer.  */
    kbd_buf_init(1319, 239, 8,
                 (CLOCK)(rfsh_per_sec * cycles_per_rfsh));

    plus4_ui_init();

    iec_init();

    return 0;
}

/* PLUS4-specific reset sequence.  */
void machine_specific_reset(void)
{
    serial_reset();

#ifdef HAVE_RS232
    acia_reset();
#endif

    printer_reset();

    ted_reset();

    drive_reset();
    datasette_reset();
}

void machine_powerup(void)
{
    mem_powerup();
    ted_reset_registers();
    maincpu_trigger_reset();
}

void machine_shutdown(void)
{
    file_system_detach_disk_shutdown();

    tape_image_detach(1);

    console_close_all();

    ted_free();
}

void machine_handle_pending_alarms(int num_write_cycles)
{
     ted_handle_pending_alarms(num_write_cycles);
}

/* ------------------------------------------------------------------------- */

/* This hook is called at the end of every frame.  */
static void machine_vsync_hook(void)
{
    CLOCK sub;

    drive_vsync_hook();

    autostart_advance();

    sub = clk_guard_prevent_overflow(&maincpu_clk_guard);

    /* The drive has to deal both with our overflowing and its own one, so
       it is called even when there is no overflowing in the main CPU.  */
    /* FIXME: Do we have to check drive_enabled here?  */
    drive_prevent_clk_overflow(sub, 0);
    drive_prevent_clk_overflow(sub, 1);
}

int machine_set_restore_key(int v)
{
    maincpu_set_nmi(I_RESTORE, v ? 1 : 0);
    return 1;
}

/* ------------------------------------------------------------------------- */

long machine_get_cycles_per_second(void)
{
    return cycles_per_sec;
}

void machine_change_timing(int timeval)
{
    unsigned int cycles_per_line = 0, screen_lines = 0;

    maincpu_trigger_reset();

    switch (timeval) {
      case MACHINE_SYNC_PAL:
        cycles_per_sec = PLUS4_PAL_CYCLES_PER_SEC;
        cycles_per_rfsh = PLUS4_PAL_CYCLES_PER_RFSH;
        rfsh_per_sec = PLUS4_PAL_RFSH_PER_SEC;
        cycles_per_line = PLUS4_PAL_CYCLES_PER_LINE;
        screen_lines = PLUS4_PAL_SCREEN_LINES;
        break;
      case MACHINE_SYNC_NTSC:
        cycles_per_sec = PLUS4_NTSC_CYCLES_PER_SEC;
        cycles_per_rfsh = PLUS4_NTSC_CYCLES_PER_RFSH;
        rfsh_per_sec = PLUS4_NTSC_RFSH_PER_SEC;
        cycles_per_line = PLUS4_NTSC_CYCLES_PER_LINE;
        screen_lines = PLUS4_NTSC_SCREEN_LINES;
        break;
      default:
        log_error(plus4_log, "Unknown machine timing.");
    }

    vsync_set_machine_parameter(rfsh_per_sec, cycles_per_sec);
    sound_set_machine_parameter(cycles_per_sec, cycles_per_rfsh);
    debug_set_machine_parameter(cycles_per_line, screen_lines);
}

/* ------------------------------------------------------------------------- */

#define SNAP_MAJOR 1
#define SNAP_MINOR 0

int machine_write_snapshot(const char *name, int save_roms, int save_disks)
{
    snapshot_t *s;

    s = snapshot_create(name, ((BYTE)(SNAP_MAJOR)), ((BYTE)(SNAP_MINOR)),
                        machine_name);
    if (s == NULL)
        return -1;

    /* Execute drive CPUs to get in sync with the main CPU.  */
    if (drive[0].enable)
        drive0_cpu_execute(maincpu_clk);
    if (drive[1].enable)
        drive1_cpu_execute(maincpu_clk);

    if (maincpu_snapshot_write_module(s) < 0
        || plus4_snapshot_write_module(s, save_roms) < 0
        || drive_snapshot_write_module(s, save_disks, save_roms) < 0
        || ted_snapshot_write_module(s) < 0) {
        snapshot_close(s);
        util_file_remove(name);
        return -1;
    }

    snapshot_close(s);
    return 0;
}

int machine_read_snapshot(const char *name)
{
    snapshot_t *s;
    BYTE minor, major;

    s = snapshot_open(name, &major, &minor, machine_name);
    if (s == NULL)
        return -1;

    if (major != SNAP_MAJOR || minor != SNAP_MINOR) {
        log_error(plus4_log,
                  "Snapshot version (%d.%d) not valid: expecting %d.%d.",
                  major, minor, SNAP_MAJOR, SNAP_MINOR);
        goto fail;
    }

    ted_prepare_for_snapshot();

    if (maincpu_snapshot_read_module(s) < 0
        || plus4_snapshot_read_module(s) < 0
        || drive_snapshot_read_module(s) < 0
        || ted_snapshot_read_module(s) < 0)
        goto fail;

    snapshot_close(s);

    return 0;

fail:
    if (s != NULL)
        snapshot_close(s);
    maincpu_trigger_reset();
    return -1;
}

/* ------------------------------------------------------------------------- */

int machine_autodetect_psid(const char *name)
{
    return -1;
}

int machine_screenshot(screenshot_t *screenshot, unsigned int wn)
{
    if (wn != 0)
        return -1;

    ted_screenshot(screenshot);
    return 0;
}

int machine_canvas_screenshot(screenshot_t *screenshot,
                              struct video_canvas_s *canvas)
{
    if (canvas != ted_get_canvas())
        return -1;

    ted_screenshot(screenshot);
    return 0;
}

int machine_canvas_async_refresh(struct canvas_refresh_s *refresh,
                                 struct video_canvas_s *canvas)
{
    if (canvas != ted_get_canvas())
        return -1;

    ted_async_refresh(refresh);
    return 0;
}

unsigned int machine_num_keyboard_mappings(void)
{
    return NUM_KEYBOARD_MAPPINGS;
}

/* ------------------------------------------------------------------------- */
/* Temporary kluge: */

void printer_interface_userport_set_busy(int flank)
{
}

