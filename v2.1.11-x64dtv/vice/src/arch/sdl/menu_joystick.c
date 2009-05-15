/*
 * menu_joystick.c - Joystick menu for SDL UI.
 *
 * Written by
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
#include "types.h"

#include <SDL/SDL.h>

#include "joy.h"
#include "lib.h"
#include "menu_common.h"
#include "menu_joystick.h"
#include "resources.h"
#include "uimenu.h"
#include "uipoll.h"

UI_MENU_DEFINE_RADIO(JoyDevice1)

static const ui_menu_entry_t joystick_port1_device_menu[] = {
    { "None",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_NONE },
    { "Numpad",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_NUMPAD },
    { "Keyset 1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_KEYSET1 },
    { "Keyset 2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_KEYSET2 },
    { "Joystick",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_JOYSTICK },
    { NULL }
};

UI_MENU_DEFINE_RADIO(JoyDevice2)

static const ui_menu_entry_t joystick_port2_device_menu[] = {
    { "None",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_NONE },
    { "Numpad",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_NUMPAD },
    { "Keyset 1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_KEYSET1 },
    { "Keyset 2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_KEYSET2 },
    { "Joystick",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_JOYSTICK },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(KeySetEnable)

static UI_MENU_CALLBACK(custom_swap_ports_callback)
{
    if (activated) {
        sdljoy_swap_ports();
    }
    return NULL;
}

static UI_MENU_CALLBACK(custom_keyset_callback)
{
    SDL_Event e;
    int previous;

    if(resources_get_int((const char *)param, &previous)) {
        return sdl_menu_text_unknown;
    }
            
    if (activated) {
        e = sdl_ui_poll_event("key", (const char *)param, SDL_POLL_KEYBOARD | SDL_POLL_MODIFIER, 5);

        if(e.type == SDL_KEYDOWN) {
            resources_set_int((const char *)param, (int)e.key.keysym.sym);
        }
    } else {
        return SDL_GetKeyName(previous);
    }
    return NULL;
}

static const ui_menu_entry_t define_keyset_menu[] = {
    { "Keyset 1 Up",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1North" },
    { "Keyset 1 Down",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1South" },
    { "Keyset 1 Left",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1West" },
    { "Keyset 1 Right",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1East" },
    { "Keyset 1 Fire",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1Fire" },
    SDL_MENU_ITEM_SEPARATOR,
    { "Keyset 2 Up",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2North" },
    { "Keyset 2 Down",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2South" },
    { "Keyset 2 Left",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2West" },
    { "Keyset 2 Right",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2East" },
    { "Keyset 2 Fire",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2Fire" },
    { NULL }
};

static const char *joy_pin[] = {
    "Up",
    "Down",
    "Left",
    "Right",
    "Fire"
};

static UI_MENU_CALLBACK(custom_joymap_callback)
{
    char *target = NULL;
    SDL_Event e;
    int pin, port;

    if (activated) {
        pin = (vice_ptr_to_int(param)) & 7;
        port = (vice_ptr_to_int(param)) >> 4;

        target = lib_msprintf("Port %i %s", port+1, joy_pin[pin]);
        e = sdl_ui_poll_event("joystick", target, SDL_POLL_JOYSTICK, 5);
        lib_free(target);

        switch(e.type) {
            case SDL_JOYAXISMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYHATMOTION:
                sdljoy_set_joystick(e, port, (1<<pin));
                break;
            default:
                break;
        }
    }

    return NULL;
}

static const ui_menu_entry_t define_joy1_menu[] = {
    { "Joystick 1 Up",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)0 },
    { "Joystick 1 Down",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)1 },
    { "Joystick 1 Left",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)2 },
    { "Joystick 1 Right",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)3 },
    { "Joystick 1 Fire",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)4 },
    { NULL }
};

static const ui_menu_entry_t define_joy2_menu[] = {
    { "Joystick 2 Up",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)0x10 },
    { "Joystick 2 Down",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)0x11 },
    { "Joystick 2 Left",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)0x12 },
    { "Joystick 2 Right",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)0x13 },
    { "Joystick 2 Fire",
      MENU_ENTRY_DIALOG,
      custom_joymap_callback,
      (ui_callback_data_t)0x14 },
    { NULL }
};

static UI_MENU_CALLBACK(custom_joy_misc_callback)
{
    char *target = NULL;
    SDL_Event e;

    if (activated) {
        e = sdl_ui_poll_event("joystick", (vice_ptr_to_int(param))?"Map":"Menu activate", SDL_POLL_JOYSTICK, 5);
        lib_free(target);

        switch(e.type) {
            case SDL_JOYAXISMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYHATMOTION:
                sdljoy_set_extra(e, vice_ptr_to_int(param));
                break;
            default:
                break;
        }
    }

    return NULL;
}

UI_MENU_DEFINE_INT(JoyThreshold)
UI_MENU_DEFINE_INT(JoyFuzz)

static const ui_menu_entry_t define_joy_misc_menu[] = {
    { "Menu activate",
      MENU_ENTRY_DIALOG,
      custom_joy_misc_callback,
      (ui_callback_data_t)0 },
    { "Map",
      MENU_ENTRY_DIALOG,
      custom_joy_misc_callback,
      (ui_callback_data_t)1 },
    SDL_MENU_ITEM_SEPARATOR,
    { "Threshold",
      MENU_ENTRY_RESOURCE_INT,
      int_JoyThreshold_callback,
      (ui_callback_data_t)"Set joystick threshold (0 - 32767)" },
    { "Fuzz",
      MENU_ENTRY_RESOURCE_INT,
      int_JoyFuzz_callback,
      (ui_callback_data_t)"Set joystick fuzz (0 - 32767)" },
    { NULL }
};

const ui_menu_entry_t joystick_menu[] = {
    { "Joystick device in port 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port1_device_menu },
    { "Joystick device in port 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port2_device_menu },
    { "Swap joystick ports",
      MENU_ENTRY_OTHER,
      custom_swap_ports_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { "Define keysets",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_keyset_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Joystick 1 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy1_menu },
    { "Joystick 2 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy2_menu },
    { "Joystick extra options", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy_misc_menu },
    { NULL }
};

const ui_menu_entry_t joystick_single_menu[] = {
    { "Joystick device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port1_device_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { "Define keysets",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_keyset_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Joystick mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy1_menu },
    { "Joystick extra options", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy_misc_menu },
    { NULL }
};
