/*
 * cbm5x0ui.c - CBM5x0-specific user interface.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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
#include <windows.h>

#include "cartridge.h"
#include "debug.h"
#include "machine.h"
#include "petui.h"
#include "res.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "uiacia.h"
#include "uicart.h"
#include "uicbm5x0model.h"
#include "uicbm5x0set.h"
#include "uicia.h"
#include "uidrivepetcbm2.h"
#include "uijoystick.h"
#include "uikeyboard.h"
#include "uilib.h"
#include "uirom.h"
#include "uisiddtv.h"
#include "uivideo.h"
#include "winmain.h"

static const ui_menu_toggle_t cbm5x0_ui_menu_toggles[] = {
    { "VICIIDoubleSize", IDM_TOGGLE_DOUBLESIZE },
    { "VICIIDoubleScan", IDM_TOGGLE_DOUBLESCAN },
    { "VICIIVideoCache", IDM_TOGGLE_VIDEOCACHE },
    { "CartridgeReset", IDM_TOGGLE_CART_RESET },
    { NULL, 0 }
};

static const uirom_settings_t uirom_settings[] = {
    { UIROM_TYPE_MAIN, TEXT("Kernal"), "KernalName",
      IDC_CBM2ROM_KERNAL_FILE, IDC_CBM2ROM_KERNAL_BROWSE },
    { UIROM_TYPE_MAIN, TEXT("Basic"), "BasicName",
      IDC_CBM2ROM_BASIC_FILE, IDC_CBM2ROM_BASIC_BROWSE },
    { UIROM_TYPE_MAIN, TEXT("Character"), "ChargenName",
      IDC_CBM2ROM_CHARGEN_FILE, IDC_CBM2ROM_CHARGEN_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("2031"), "DosName2031",
      IDC_DRIVEROM_2031_FILE, IDC_DRIVEROM_2031_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("2040"), "DosName2040",
      IDC_DRIVEROM_2040_FILE, IDC_DRIVEROM_2040_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("3040"), "DosName3040",
      IDC_DRIVEROM_3040_FILE, IDC_DRIVEROM_3040_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("4040"), "DosName4040",
      IDC_DRIVEROM_4040_FILE, IDC_DRIVEROM_4040_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("1001"), "DosName1001",
      IDC_DRIVEROM_1001_FILE, IDC_DRIVEROM_1001_BROWSE },
    { 0, NULL, NULL, 0, 0 }
};

static const ui_res_value_list_t cbm5x0_ui_res_values[] = {
    { NULL, NULL, 0 }
};

#define CBM2UI_KBD_NUM_MAP 6

static const uikeyboard_mapping_entry_t mapping_entry[CBM2UI_KBD_NUM_MAP] = {
    { IDC_CBM2KBD_MAPPING_SELECT_UKSYM, IDC_CBM2KBD_MAPPING_UKSYM,
      IDC_CBM2KBD_MAPPING_UKSYM_BROWSE, "KeymapBusinessUKSymFile" },
    { IDC_CBM2KBD_MAPPING_SELECT_UKPOS, IDC_CBM2KBD_MAPPING_UKPOS,
      IDC_CBM2KBD_MAPPING_UKPOS_BROWSE, "KeymapBusinessUKPosFile" },
    { IDC_CBM2KBD_MAPPING_SELECT_GRSYM, IDC_CBM2KBD_MAPPING_GRSYM,
      IDC_CBM2KBD_MAPPING_GRSYM_BROWSE, "KeymapGraphicsSymFile" },
    { IDC_CBM2KBD_MAPPING_SELECT_GRPOS, IDC_CBM2KBD_MAPPING_GRPOS,
      IDC_CBM2KBD_MAPPING_GRPOS_BROWSE, "KeymapGraphicsPosFile" },
    { IDC_CBM2KBD_MAPPING_SELECT_DESYM, IDC_CBM2KBD_MAPPING_DESYM,
      IDC_CBM2KBD_MAPPING_DESYM_BROWSE, "KeymapBusinessDESymFile" },
    { IDC_CBM2KBD_MAPPING_SELECT_DEPOS, IDC_CBM2KBD_MAPPING_DEPOS,
      IDC_CBM2KBD_MAPPING_DEPOS_BROWSE, "KeymapBusinessDEPosFile" }
};

static uilib_localize_dialog_param cbm5x0_kbd_trans[] = {
    { IDC_CBM2KBD_MAPPING_SELECT_GRSYM, IDS_GR_SYM, 0 },
    { IDC_CBM2KBD_MAPPING_SELECT_GRPOS, IDS_GR_POS, 0 },
    { IDC_CBM2KBD_MAPPING_SELECT_UKSYM, IDS_UK_SYM, 0 },
    { IDC_CBM2KBD_MAPPING_SELECT_UKPOS, IDS_UK_POS, 0 },
    { IDC_CBM2KBD_MAPPING_SELECT_DESYM, IDS_DE_SYM, 0 },
    { IDC_CBM2KBD_MAPPING_SELECT_DEPOS, IDS_DE_POS, 0 },
    { IDC_CBM2KBD_MAPPING_GRSYM_BROWSE, IDS_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_GRPOS_BROWSE, IDS_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_UKSYM_BROWSE, IDS_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_UKPOS_BROWSE, IDS_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_DESYM_BROWSE, IDS_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_DEPOS_BROWSE, IDS_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_DUMP, IDS_DUMP_KEYSET, 0 },
    { IDC_KBD_SHORTCUT_DUMP, IDS_DUMP_SHORTCUTS, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group cbm5x0_kbd_left_group[] = {
    { IDC_CBM2KBD_MAPPING_SELECT_GRSYM, 1 },
    { IDC_CBM2KBD_MAPPING_SELECT_GRPOS, 1 },
    { IDC_CBM2KBD_MAPPING_SELECT_UKSYM, 1 },
    { IDC_CBM2KBD_MAPPING_SELECT_UKPOS, 1 },
    { IDC_CBM2KBD_MAPPING_SELECT_DESYM, 1 },
    { IDC_CBM2KBD_MAPPING_SELECT_DEPOS, 1 },
    { 0, 0 }
};

static uilib_dialog_group cbm5x0_kbd_middle_group[] = {
    { IDC_CBM2KBD_MAPPING_GRSYM, 0 },
    { IDC_CBM2KBD_MAPPING_GRPOS, 0 },
    { IDC_CBM2KBD_MAPPING_UKSYM, 0 },
    { IDC_CBM2KBD_MAPPING_UKPOS, 0 },
    { IDC_CBM2KBD_MAPPING_DESYM, 0 },
    { IDC_CBM2KBD_MAPPING_DEPOS, 0 },
    { 0, 0 }
};

static uilib_dialog_group cbm5x0_kbd_right_group[] = {
    { IDC_CBM2KBD_MAPPING_GRSYM_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_GRPOS_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_UKSYM_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_UKPOS_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_DESYM_BROWSE, 0 },
    { IDC_CBM2KBD_MAPPING_DEPOS_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group cbm5x0_kbd_buttons_group[] = {
    { IDC_CBM2KBD_MAPPING_DUMP, 1 },
    { IDC_KBD_SHORTCUT_DUMP, 1 },
    { 0, 0 }
};

static int cbm5x0_kbd_move_buttons_group[] = {
    IDC_CBM2KBD_MAPPING_DUMP,
    IDC_KBD_SHORTCUT_DUMP,
    0
};

static uikeyboard_config_t uikeyboard_config = {
    IDD_CBM2KBD_MAPPING_SETTINGS_DIALOG,
    CBM2UI_KBD_NUM_MAP,
    mapping_entry,
    IDC_CBM2KBD_MAPPING_DUMP,
    cbm5x0_kbd_trans,
    cbm5x0_kbd_left_group,
    cbm5x0_kbd_middle_group,
    cbm5x0_kbd_right_group,
    cbm5x0_kbd_buttons_group,
    cbm5x0_kbd_move_buttons_group
};

static const uicart_params_t cbm5x0_ui_cartridges[] = {
    { IDM_LOAD_CART_1000, CARTRIDGE_CBM2_8KB_1000, IDS_ATTACH_CBM2_CART_1000, UILIB_FILTER_ALL },
    { IDM_LOAD_CART_2000_3000, CARTRIDGE_CBM2_8KB_2000, IDS_ATTACH_CBM2_CART_2000_3000, UILIB_FILTER_ALL },
    { IDM_LOAD_CART_4000_5000, CARTRIDGE_CBM2_16KB_4000, IDS_ATTACH_CBM2_CART_4000_5000, UILIB_FILTER_ALL },
    { IDM_LOAD_CART_6000_7000, CARTRIDGE_CBM2_16KB_6000, IDS_ATTACH_CBM2_CART_6000_7000, UILIB_FILTER_ALL },
    { 0, 0, 0, 0 }
};

ui_menu_translation_table_t cbm5x0ui_menu_translation_table[] = {
    { IDM_EXIT, IDS_MI_EXIT },
    { IDM_ABOUT, IDS_MI_ABOUT },
    { IDM_HELP, IDS_MP_HELP },
    { IDM_PAUSE, IDS_MI_PAUSE },
    { IDM_EDIT_COPY, IDS_MI_EDIT_COPY },
    { IDM_EDIT_PASTE, IDS_MI_EDIT_PASTE },
    { IDM_AUTOSTART, IDS_MI_AUTOSTART },
    { IDM_RESET_HARD, IDS_MI_RESET_HARD },
    { IDM_RESET_SOFT, IDS_MI_RESET_SOFT },
    { IDM_RESET_DRIVE8, IDS_MI_DRIVE8 },
    { IDM_RESET_DRIVE9, IDS_MI_DRIVE9 },
    { IDM_RESET_DRIVE10, IDS_MI_DRIVE10 },
    { IDM_RESET_DRIVE11, IDS_MI_DRIVE11 },
    { IDM_ATTACH_8, IDS_MI_DRIVE8 },
    { IDM_ATTACH_9, IDS_MI_DRIVE9 },
    { IDM_ATTACH_10, IDS_MI_DRIVE10 },
    { IDM_ATTACH_11, IDS_MI_DRIVE11 },
    { IDM_DETACH_8, IDS_MI_DRIVE8 },
    { IDM_DETACH_9, IDS_MI_DRIVE9 },
    { IDM_DETACH_10, IDS_MI_DRIVE10 },
    { IDM_DETACH_11, IDS_MI_DRIVE11 },
    { IDM_ATTACH_TAPE, IDS_MI_ATTACH_TAPE },
    { IDM_DETACH_TAPE, IDS_MI_DETACH_TAPE },
    { IDM_DETACH_ALL, IDS_MI_DETACH_ALL },
    { IDM_TOGGLE_SOUND, IDS_MI_TOGGLE_SOUND },
    { IDM_TOGGLE_DOUBLESIZE, IDS_MI_TOGGLE_DOUBLESIZE },
    { IDM_TOGGLE_DOUBLESCAN, IDS_MI_TOGGLE_DOUBLESCAN },
    { IDM_TOGGLE_DRIVE_TRUE_EMULATION, IDS_MI_DRIVE_TRUE_EMULATION },
    { IDM_TOGGLE_DRIVE_SOUND_EMULATION, IDS_MI_DRIVE_SOUND_EMULATION },
    { IDM_TOGGLE_VIDEOCACHE, IDS_MI_TOGGLE_VIDEOCACHE },
    { IDM_DRIVE_SETTINGS, IDS_MI_DRIVE_SETTINGS },
    { IDM_FLIP_ADD, IDS_MI_FLIP_ADD },
    { IDM_FLIP_REMOVE, IDS_MI_FLIP_REMOVE },
    { IDM_FLIP_NEXT, IDS_MI_FLIP_NEXT },
    { IDM_FLIP_PREVIOUS, IDS_MI_FLIP_PREVIOUS },
    { IDM_FLIP_LOAD, IDS_MI_FLIP_LOAD },
    { IDM_FLIP_SAVE, IDS_MI_FLIP_SAVE },
    { IDM_DATASETTE_CONTROL_STOP, IDS_MI_DATASETTE_STOP },
    { IDM_DATASETTE_CONTROL_START, IDS_MI_DATASETTE_START },
    { IDM_DATASETTE_CONTROL_FORWARD, IDS_MI_DATASETTE_FORWARD },
    { IDM_DATASETTE_CONTROL_REWIND, IDS_MI_DATASETTE_REWIND },
    { IDM_DATASETTE_CONTROL_RECORD, IDS_MI_DATASETTE_RECORD },
    { IDM_DATASETTE_CONTROL_RESET, IDS_MI_DATASETTE_RESET },
    { IDM_DATASETTE_RESET_COUNTER, IDS_MI_DATASETTE_RESET_COUNTER },
    { IDM_LOAD_CART_1000, IDS_MI_LOAD_CART_1000 },
    { IDM_UNLOAD_CART_1000, IDS_MI_UNLOAD_CART_1000 },
    { IDM_LOAD_CART_2000_3000, IDS_MI_LOAD_CART_2000_3000 },
    { IDM_UNLOAD_CART_2000_3000, IDS_MI_UNLOAD_CART_2000_3000 },
    { IDM_LOAD_CART_4000_5000, IDS_MI_LOAD_CART_4000_5000 },
    { IDM_UNLOAD_CART_4000_5000, IDS_MI_UNLOAD_CART_4000_5000 },
    { IDM_LOAD_CART_6000_7000, IDS_MI_LOAD_CART_6000_7000 },
    { IDM_UNLOAD_CART_6000_7000, IDS_MI_UNLOAD_CART_6000_7000 },
    { IDM_TOGGLE_CART_RESET, IDS_MI_TOGGLE_CART_RESET },
    { IDM_MONITOR, IDS_MI_MONITOR },
#ifdef DEBUG
    { IDM_DEBUG_MODE_NORMAL, IDS_MI_DEBUG_MODE_NORMAL },
    { IDM_DEBUG_MODE_SMALL, IDS_MI_DEBUG_MODE_SMALL },
    { IDM_DEBUG_MODE_HISTORY, IDS_MI_DEBUG_MODE_HISTORY },
    { IDM_DEBUG_MODE_AUTOPLAY, IDS_MI_DEBUG_MODE_AUTOPLAY },
    { IDM_TOGGLE_MAINCPU_TRACE, IDS_MI_TOGGLE_MAINCPU_TRACE },
    { IDM_TOGGLE_DRIVE0CPU_TRACE, IDS_MI_TOGGLE_DRIVE0CPU_TRACE },
    { IDM_TOGGLE_DRIVE1CPU_TRACE, IDS_MI_TOGGLE_DRIVE1CPU_TRACE },
#endif
    { IDM_SNAPSHOT_LOAD, IDS_MI_SNAPSHOT_LOAD },
    { IDM_SNAPSHOT_SAVE, IDS_MI_SNAPSHOT_SAVE },
    { IDM_LOADQUICK, IDS_MI_LOADQUICK },
    { IDM_SAVEQUICK, IDS_MI_SAVEQUICK },
    { IDM_EVENT_TOGGLE_RECORD, IDS_MI_EVENT_TOGGLE_RECORD },
    { IDM_EVENT_TOGGLE_PLAYBACK, IDS_MI_EVENT_TOGGLE_PLAYBACK },
    { IDM_EVENT_SETMILESTONE, IDS_MI_EVENT_SETMILESTONE },
    { IDM_EVENT_RESETMILESTONE, IDS_MI_EVENT_RESETMILESTONE },
    { IDM_EVENT_START_MODE_SAVE, IDS_MI_EVENT_START_MODE_SAVE },
    { IDM_EVENT_START_MODE_LOAD, IDS_MI_EVENT_START_MODE_LOAD },
    { IDM_EVENT_START_MODE_RESET, IDS_MI_EVENT_START_MODE_RESET },
    { IDM_EVENT_START_MODE_PLAYBACK, IDS_MI_EVENT_START_MODE_PLAYBCK },
    { IDM_EVENT_DIRECTORY, IDS_MI_EVENT_DIRECTORY },
    { IDM_JAM_ACTION_ASK, IDS_MI_JAM_ACTION_ASK },
    { IDM_JAM_ACTION_CONTINUE, IDS_MI_JAM_ACTION_CONTINUE },
    { IDM_JAM_ACTION_START_MONITOR, IDS_MI_JAM_ACTION_START_MONITOR },
    { IDM_JAM_ACTION_RESET, IDS_MI_JAM_ACTION_RESET },
    { IDM_JAM_ACTION_HARD_RESET, IDS_MI_JAM_ACTION_HARD_RESET },
    { IDM_JAM_ACTION_QUIT_EMULATOR, IDS_MI_JAM_ACTION_QUIT_EMULATOR },
    { IDM_MEDIAFILE, IDS_MI_MEDIAFILE },
    { IDM_SOUND_RECORD_START, IDS_MI_SOUND_RECORD_START },
    { IDM_SOUND_RECORD_STOP, IDS_MI_SOUND_RECORD_STOP },
    { IDM_REFRESH_RATE_AUTO, IDS_MI_REFRESH_RATE_AUTO },
    { IDM_MAXIMUM_SPEED_NO_LIMIT, IDS_MI_MAXIMUM_SPEED_NO_LIMIT },
    { IDM_MAXIMUM_SPEED_CUSTOM, IDS_MI_MAXIMUM_SPEED_CUSTOM },
    { IDM_TOGGLE_WARP_MODE, IDS_MI_TOGGLE_WARP_MODE },
    { IDM_TOGGLE_DX9DISABLE, IDS_MI_TOGGLE_DX9DISABLE },
    { IDM_TOGGLE_ALWAYSONTOP, IDS_MI_TOGGLE_ALWAYSONTOP },
    { IDM_SWAP_JOYSTICK, IDS_MI_SWAP_JOYSTICK },
    { IDM_ALLOW_JOY_OPPOSITE_TOGGLE, IDS_MI_ALLOW_JOY_OPPOSITE },
    { IDM_JOYKEYS_TOGGLE, IDS_MI_JOYKEYS_TOGGLE },
    { IDM_TOGGLE_VIRTUAL_DEVICES, IDS_MI_TOGGLE_VIRTUAL_DEVICES },
    { IDM_CBM2MODEL_SETTINGS, IDS_MI_CBM5X0MODEL_SETTINGS },
    { IDM_AUTOSTART_SETTINGS, IDS_MI_AUTOSTART_SETTINGS },
    { IDM_VIDEO_SETTINGS, IDS_MI_VIDEO_SETTINGS },
    { IDM_DEVICEMANAGER, IDS_MI_DEVICEMANAGER },
    { IDM_JOY_SETTINGS, IDS_MI_JOY_SETTINGS },
    { IDM_KEYBOARD_SETTINGS, IDS_MI_KEYBOARD_SETTINGS },
    { IDM_SOUND_SETTINGS, IDS_MI_SOUND_SETTINGS },
    { IDM_ROM_SETTINGS, IDS_MI_ROM_SETTINGS },
    { IDM_RAM_SETTINGS, IDS_MI_RAM_SETTINGS },
    { IDM_DATASETTE_SETTINGS, IDS_MI_DATASETTE_SETTINGS },
    { IDM_SID_SETTINGS, IDS_MI_SID_SETTINGS },
    { IDM_CIA_SETTINGS, IDS_MI_CIA_SETTINGS },
    { IDM_RS232_SETTINGS, IDS_MI_RS232_SETTINGS },
    { IDM_ACIA_SETTINGS, IDS_MI_ACIA_SETTINGS },
    { IDM_SETTINGS_SAVE_FILE, IDS_MI_SETTINGS_SAVE_FILE },
    { IDM_SETTINGS_LOAD_FILE, IDS_MI_SETTINGS_LOAD_FILE },
    { IDM_SETTINGS_SAVE, IDS_MI_SETTINGS_SAVE },
    { IDM_SETTINGS_LOAD, IDS_MI_SETTINGS_LOAD },
    { IDM_SETTINGS_DEFAULT, IDS_MI_SETTINGS_DEFAULT },
    { IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT, IDS_MI_SAVE_SETTINGS_ON_EXIT },
    { IDM_TOGGLE_CONFIRM_ON_EXIT, IDS_MI_CONFIRM_ON_EXIT },
    { IDM_LANG_EN, IDS_MI_LANG_EN },
    { IDM_LANG_DA, IDS_MI_LANG_DA },
    { IDM_LANG_DE, IDS_MI_LANG_DE },
    { IDM_LANG_ES, IDS_MI_LANG_ES },
    { IDM_LANG_FR, IDS_MI_LANG_FR },
    { IDM_LANG_HU, IDS_MI_LANG_HU },
    { IDM_LANG_IT, IDS_MI_LANG_IT },
    { IDM_LANG_KO, IDS_MI_LANG_KO },
    { IDM_LANG_NL, IDS_MI_LANG_NL },
    { IDM_LANG_PL, IDS_MI_LANG_PL },
    { IDM_LANG_RU, IDS_MI_LANG_RU },
    { IDM_LANG_SV, IDS_MI_LANG_SV },
    { IDM_LANG_TR, IDS_MI_LANG_TR },
    { IDM_CMDLINE, IDS_MI_CMDLINE },
    { IDM_CONTRIBUTORS, IDS_MI_CONTRIBUTORS },
    { IDM_LICENSE, IDS_MI_LICENSE },
    { IDM_WARRANTY, IDS_MI_WARRANTY },
    { IDM_TOGGLE_FULLSCREEN, IDS_MI_TOGGLE_FULLSCREEN },
    { IDM_CBM2_SETTINGS, IDS_MI_CBM5X0_SETTINGS },
    { 0, 0 }
};

ui_popup_translation_table_t cbm5x0ui_popup_translation_table[] = {
    { 1, IDS_MP_FILE },
    { 2, IDS_MP_ATTACH_DISK_IMAGE },
    { 2, IDS_MP_DETACH_DISK_IMAGE },
    { 2, IDS_MP_FLIP_LIST },
    { 2, IDS_MP_DATASETTE_CONTROL },
    { 2, IDS_MP_ATTACH_CARTRIDGE_IMAGE },
    { 2, IDS_MP_RESET },
    { 2, IDS_MP_DEFAULT_CPU_JAM_ACTION },
#ifdef DEBUG
    { 2, IDS_MP_DEBUG },
    { 3, IDS_MP_MODE },
#endif
    { 1, IDS_MP_EDIT },
    { 1, IDS_MP_SNAPSHOT },
    { 2, IDS_MP_RECORDING_START_MODE },
/*    { 1, IDS_MP_OPTIONS },*/
    { 1, IDS_MP_SETTINGS },
    { 2, IDS_MP_REFRESH_RATE },
    { 2, IDS_MP_MAXIMUM_SPEED },
    { 2, IDS_MP_SOUND_SETTINGS },
    { 2, IDS_MP_DRIVE_SETTINGS },
    { 2, IDS_MP_JOYSTICK_SETTINGS },
/*    { 2, IDS_MP_MOUSE_SETTINGS }, */
    { 2, IDS_MP_VICII_SETTINGS },
    { 2, IDS_MP_DRIVE_SYNC_FACTOR },
    { 2, IDS_MP_CARTRIDGE_IO_SETTINGS },
    { 1, IDS_MP_LANGUAGE },
    { 1, IDS_MP_HELP },
    { 0, 0 }
};

static uilib_localize_dialog_param cbm5x0_main_trans[] = {
    { IDC_KERNAL, IDS_KERNAL, 0 },
    { IDC_CBM2ROM_KERNAL_BROWSE, IDS_BROWSE, 0 },
    { IDC_BASIC, IDS_BASIC, 0 },
    { IDC_CBM2ROM_BASIC_BROWSE, IDS_BROWSE, 0 },
    { IDC_CHARACTER, IDS_CHARACTER, 0 },
    { IDC_CBM2ROM_CHARGEN_BROWSE, IDS_BROWSE, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param cbm5x0_drive_trans[] = {
    { IDC_DRIVEROM_2031_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_2040_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_3040_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_4040_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_1001_BROWSE, IDS_BROWSE, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group cbm5x0_main_left_group[] = {
    { IDC_KERNAL, 0 },
    { IDC_BASIC, 0 },
    { IDC_CHARACTER, 0 },
    { 0, 0 }
};

static uilib_dialog_group cbm5x0_main_middle_group[] = {
    { IDC_CBM2ROM_KERNAL_FILE, 0 },
    { IDC_CBM2ROM_BASIC_FILE, 0 },
    { IDC_CBM2ROM_CHARGEN_FILE, 0 },
    { 0, 0 }
};

static uilib_dialog_group cbm5x0_main_right_group[] = {
    { IDC_CBM2ROM_KERNAL_BROWSE, 0 },
    { IDC_CBM2ROM_BASIC_BROWSE, 0 },
    { IDC_CBM2ROM_CHARGEN_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group cbm5x0_drive_left_group[] = {
    { IDC_2031, 0 },
    { IDC_2040, 0 },
    { IDC_3040, 0 },
    { IDC_4040, 0 },
    { IDC_1001, 0 },
    { 0, 0 }
};

static uilib_dialog_group cbm5x0_drive_middle_group[] = {
    { IDC_DRIVEROM_2031_FILE, 0 },
    { IDC_DRIVEROM_2040_FILE, 0 },
    { IDC_DRIVEROM_3040_FILE, 0 },
    { IDC_DRIVEROM_4040_FILE, 0 },
    { IDC_DRIVEROM_1001_FILE, 0 },
    { 0, 0 }
};

static uilib_dialog_group cbm5x0_drive_right_group[] = {
    { IDC_DRIVEROM_2031_BROWSE, 0 },
    { IDC_DRIVEROM_2040_BROWSE, 0 },
    { IDC_DRIVEROM_3040_BROWSE, 0 },
    { IDC_DRIVEROM_4040_BROWSE, 0 },
    { IDC_DRIVEROM_1001_BROWSE, 0 },
    { 0, 0 }
};

static generic_trans_table_t cbm5x0_generic_trans[] = {
    { IDC_2031, "2031" },
    { IDC_2040, "2040" },
    { IDC_3040, "3040" },
    { IDC_4040, "4040" },
    { IDC_1001, "1001" },
    { 0, NULL }
};

static void cbm5x0_ui_specific(WPARAM wparam, HWND hwnd)
{
    switch (wparam) {
        case IDM_LOAD_CART_1000:
        case IDM_LOAD_CART_2000_3000:
        case IDM_LOAD_CART_4000_5000:
        case IDM_LOAD_CART_6000_7000:
            uicart_attach(wparam, hwnd, cbm5x0_ui_cartridges);
            break;
        case IDM_UNLOAD_CART_1000:
            cartridge_detach_image(CARTRIDGE_CBM2_8KB_1000);
            break;
        case IDM_UNLOAD_CART_2000_3000:
            cartridge_detach_image(CARTRIDGE_CBM2_8KB_2000);
            break;
        case IDM_UNLOAD_CART_4000_5000:
            cartridge_detach_image(CARTRIDGE_CBM2_16KB_4000);
            break;
        case IDM_UNLOAD_CART_6000_7000:
            cartridge_detach_image(CARTRIDGE_CBM2_16KB_6000);
            break;
        case IDM_CBM2MODEL_SETTINGS:
            ui_cbm5x0model_settings_dialog(hwnd);
            break;
        case IDM_CBM2_SETTINGS:
            ui_cbm5x0_settings_dialog(hwnd);
            break;
        case IDM_JOY_SETTINGS:
            ui_joystick_settings_dialog(hwnd);
            break;
        case IDM_SID_SETTINGS:
            ui_siddtv_settings_dialog(hwnd);
            break;
        case IDM_CIA_SETTINGS:
            ui_cia_settings_dialog(hwnd, 1);
            break;
        case IDM_ROM_SETTINGS:
            uirom_settings_dialog(hwnd, IDD_CBM2ROM_SETTINGS_DIALOG, IDD_CBM2DRIVEROM_SETTINGS_DIALOG,
                                  uirom_settings,
                                  cbm5x0_main_trans, cbm5x0_drive_trans, cbm5x0_generic_trans,
                                  cbm5x0_main_left_group, cbm5x0_main_middle_group, cbm5x0_main_right_group,
                                  cbm5x0_drive_left_group, cbm5x0_drive_middle_group, cbm5x0_drive_right_group);
            break;
        case IDM_VIDEO_SETTINGS:
            ui_video_settings_dialog(hwnd, UI_VIDEO_CHIP_VICII, UI_VIDEO_CHIP_NONE);
            break;
        case IDM_DRIVE_SETTINGS:
            uidrivepetcbm2_settings_dialog(hwnd);
            break;
        case IDM_ACIA_SETTINGS:
            ui_acia_settings_dialog(hwnd);
            break;
        case IDM_KEYBOARD_SETTINGS:
            uikeyboard_settings_dialog(hwnd, &uikeyboard_config);
            break;
    }
}

int cbm5x0ui_init(void)
{
    ui_register_machine_specific(cbm5x0_ui_specific);
    ui_register_menu_toggles(cbm5x0_ui_menu_toggles);
    ui_register_res_values(cbm5x0_ui_res_values);
    ui_register_translation_tables(cbm5x0ui_menu_translation_table, cbm5x0ui_popup_translation_table);

    return 0;
}

void cbm5x0ui_shutdown(void)
{
}
