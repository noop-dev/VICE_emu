/*
 * joystick.c - Common joystick emulation.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *
 * Based on old code by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Jouko Valta <jopi@stekt.oulu.fi>
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
 *  Bernhard Kuhn <kuhn@eikon.e-technik.tu-muenchen.de>
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

#include <stdlib.h>
#include <string.h>

#include "alarm.h"
#include "event.h"
#include "keyboard.h"
#include "joystick.h"
#include "kbd.h"
#include "maincpu.h"
#include "network.h"
#include "snapshot.h"
#include "ui.h"
#include "types.h"
#include "log.h"
#include "resources.h"
#include "cmdline.h"
#include "lib.h"

#define JOYSTICK_RAND() (rand() & 0x3fff)

#define JOYSTICK_NUM 3

/* Global joystick value.  */
BYTE joystick_value[JOYSTICK_NUM] = { 0, 0, 0 };

/* Latched joystick status.  */
static BYTE latch_joystick_value[JOYSTICK_NUM] = { 0, 0, 0 };
static BYTE network_joystick_value[JOYSTICK_NUM] = { 0, 0, 0 };

/* to prevent illegal direction combinations */
static const BYTE joystick_opposite_direction[] = 
    { 0, 2, 1, 3, 8, 10, 9, 11, 4, 6, 5, 7, 12, 14, 13, 15 };

static alarm_t *joystick_alarm = NULL;

static CLOCK joystick_delay;

static int joykeys[3][9];

static int joystick_port_map[2];

static void joystick_latch_matrix(CLOCK offset)
{
    BYTE idx;

    if (network_connected()) {
        idx = network_joystick_value[0];
        if (idx > 0)
            joystick_value[idx] = network_joystick_value[idx];
        else
            memcpy(joystick_value, network_joystick_value, sizeof(joystick_value));
    } else {
        memcpy(joystick_value, latch_joystick_value, sizeof(joystick_value));
    }
    ui_display_joyport(joystick_value);
}

/*-----------------------------------------------------------------------*/

static void joystick_event_record(void)
{
    event_record(EVENT_JOYSTICK_VALUE, (void *)joystick_value,
                 sizeof(joystick_value));
}

void joystick_event_playback(CLOCK offset, void *data)
{
    memcpy(latch_joystick_value, data, sizeof(joystick_value));

    joystick_latch_matrix(offset);
}

static void joystick_latch_handler(CLOCK offset, void *data)
{
    alarm_unset(joystick_alarm);
    alarm_context_update_next_pending(joystick_alarm->context);

    joystick_latch_matrix(offset);

    joystick_event_record(); 
}

void joystick_event_delayed_playback(void *data)
{
    memcpy(network_joystick_value, data, sizeof(latch_joystick_value));
    alarm_set(joystick_alarm, maincpu_clk + joystick_delay);
}

void joystick_register_delay(unsigned int delay)
{
    joystick_delay = delay;
}
/*-----------------------------------------------------------------------*/
static void joystick_process_latch(void)
{
    if (network_connected()) {
        CLOCK joystick_delay = JOYSTICK_RAND();
        network_event_record(EVENT_JOYSTICK_DELAY,
                (void *)&joystick_delay, sizeof(joystick_delay));
        network_event_record(EVENT_JOYSTICK_VALUE, 
                (void *)latch_joystick_value, sizeof(latch_joystick_value));
    } 
    else
    {
        alarm_set(joystick_alarm, maincpu_clk + JOYSTICK_RAND());
    }
}

void joystick_set_value_absolute(unsigned int joyport, BYTE value)
{
    if (event_playback_active())
        return;

    if (latch_joystick_value[joyport] != value) {
        latch_joystick_value[joyport] = value;
        latch_joystick_value[0] = (BYTE)joyport;
        joystick_process_latch();
    }
}

void joystick_set_value_or(unsigned int joyport, BYTE value)
{
    if (event_playback_active())
        return;

    latch_joystick_value[joyport] |= value;
    latch_joystick_value[joyport] &= ~joystick_opposite_direction[value & 0xf];
    latch_joystick_value[0] = (BYTE)joyport;
    joystick_process_latch();
}

void joystick_set_value_and(unsigned int joyport, BYTE value)
{
    if (event_playback_active())
        return;

    latch_joystick_value[joyport] &= value;
    latch_joystick_value[0] = (BYTE)joyport;
    joystick_process_latch();
}

void joystick_clear(unsigned int joyport)
{
    latch_joystick_value[joyport] = 0;
    latch_joystick_value[0] = (BYTE)joyport;
    joystick_latch_matrix(0);
}

void joystick_clear_all(void)
{
    memset(latch_joystick_value, 0, JOYSTICK_NUM);
    joystick_latch_matrix(0);
}

/*-----------------------------------------------------------------------*/

#ifdef COMMON_KBD
/* the order of values in joypad_bits is the same as in joystick_direction_t */
static int joypad_bits[9] = {
    0x10, 0x06, 0x02, 0x0a, 0x04, 0x08, 0x05, 0x01, 0x09
};

static int joypad_status[3][9];

typedef enum {
    KEYSET_FIRE,
    KEYSET_SW,
    KEYSET_S,
    KEYSET_SE,
    KEYSET_W,
    KEYSET_E,
    KEYSET_NW,
    KEYSET_N,
    KEYSET_NE
} joystick_direction_t;

static int joyreleaseval(int column, int *status)
{
    int val = 0;

    switch (column) {
      case KEYSET_SW:
        val = (status[KEYSET_S] ? 0 : joypad_bits[KEYSET_S]) | 
              (status[KEYSET_W] ? 0 : joypad_bits[KEYSET_W]);
        break;
      case KEYSET_SE:
        val = (status[KEYSET_S] ? 0 : joypad_bits[KEYSET_S]) | 
              (status[KEYSET_E] ? 0 : joypad_bits[KEYSET_E]);
        break;
      case KEYSET_NW:
        val = (status[KEYSET_N] ? 0 : joypad_bits[KEYSET_N]) | 
              (status[KEYSET_W] ? 0 : joypad_bits[KEYSET_W]);
        break;
      case KEYSET_NE:
        val = (status[KEYSET_N] ? 0 : joypad_bits[KEYSET_N]) | 
              (status[KEYSET_E] ? 0 : joypad_bits[KEYSET_E]);
        break;
      default:
        val = joypad_bits[column];
        break;
    }
    return ~val;
}

static joy_data_t* joy_devices[MAX_HW_JOY_DRIVERS];

int set_joystick_device(int val, void *param)
{
    joy_data_t* dev;
    int port = (int)param;
    
    if (joystick_port_map[port] >= JOYDEV_HW_BASE)
    {
        dev = joy_devices[joystick_port_map[port] - JOYDEV_HW_BASE];
        if (dev->device->close) {
            dev->device->close(dev->priv);
        }
    }
    

    if (val < JOYDEV_HW_BASE) {
        joystick_port_map[port] = val;
        return 0;
    }

    joystick_port_map[port] = JOYDEV_NONE;
    
    dev = joy_devices[val - JOYDEV_HW_BASE];
    
    if (dev == NULL)
        return 0;

    if (!dev->device->open || dev->device->open(dev->priv)) {
        joystick_port_map[port] = val;
    }

    return 0;
}

#define DEFINE_SET_KEYSET(num)                       \
    static int set_keyset##num(int val, void *param) \
    {                                                \
        joykeys[num][(int)param] = val;              \
                                                     \
        return 0;                                    \
    }

DEFINE_SET_KEYSET(1)
DEFINE_SET_KEYSET(2)

static const resource_int_t resources_int[] = {
    { "KeySet1NorthWest", 0, RES_EVENT_NO, NULL,
      &joykeys[1][KEYSET_NW], set_keyset1, (void *)KEYSET_NW },
    { "KeySet1North", 0, RES_EVENT_NO, NULL,
      &joykeys[1][KEYSET_N], set_keyset1, (void *)KEYSET_N },
    { "KeySet1NorthEast", 0, RES_EVENT_NO, NULL,
      &joykeys[1][KEYSET_NE], set_keyset1, (void *)KEYSET_NE },
    { "KeySet1East", 0, RES_EVENT_NO, NULL,
      &joykeys[1][KEYSET_E], set_keyset1, (void *)KEYSET_E },
    { "KeySet1SouthEast", 0, RES_EVENT_NO, NULL,
      &joykeys[1][KEYSET_SE], set_keyset1, (void *)KEYSET_SE },
    { "KeySet1South", 0, RES_EVENT_NO, NULL,
      &joykeys[1][KEYSET_S], set_keyset1, (void *)KEYSET_S },
    { "KeySet1SouthWest", 0, RES_EVENT_NO, NULL,
      &joykeys[1][KEYSET_SW], set_keyset1, (void *)KEYSET_SW },
    { "KeySet1West", 0, RES_EVENT_NO, NULL,
      &joykeys[1][KEYSET_W], set_keyset1, (void *)KEYSET_W },
    { "KeySet1Fire", 0, RES_EVENT_NO, NULL,
      &joykeys[1][KEYSET_FIRE], set_keyset1, (void *)KEYSET_FIRE },
    { "KeySet2NorthWest", 0, RES_EVENT_NO, NULL,
      &joykeys[2][KEYSET_NW], set_keyset2, (void *)KEYSET_NW },
    { "KeySet2North", 0, RES_EVENT_NO, NULL,
      &joykeys[2][KEYSET_N], set_keyset2, (void *)KEYSET_N },
    { "KeySet2NorthEast", 0, RES_EVENT_NO, NULL,
      &joykeys[2][KEYSET_NE], set_keyset2, (void *)KEYSET_NE },
    { "KeySet2East", 0, RES_EVENT_NO, NULL,
      &joykeys[2][KEYSET_E], set_keyset2, (void *)KEYSET_E },
    { "KeySet2SouthEast", 0, RES_EVENT_NO, NULL,
      &joykeys[2][KEYSET_SE], set_keyset2, (void *)KEYSET_SE },
    { "KeySet2South", 0, RES_EVENT_NO, NULL,
      &joykeys[2][KEYSET_S], set_keyset2, (void *)KEYSET_S },
    { "KeySet2SouthWest", 0, RES_EVENT_NO, NULL,
      &joykeys[2][KEYSET_SW], set_keyset2, (void *)KEYSET_SW },
    { "KeySet2West", 0, RES_EVENT_NO, NULL,
      &joykeys[2][KEYSET_W], set_keyset2, (void *)KEYSET_W },
    { "KeySet2Fire", 0, RES_EVENT_NO, NULL,
      &joykeys[2][KEYSET_FIRE], set_keyset2, (void *)KEYSET_FIRE },
    { "JoyDevice1", JOYDEV_NONE, RES_EVENT_NO, NULL,
      &joystick_port_map[0], set_joystick_device, (void *)0 },
    { "JoyDevice2", JOYDEV_NONE, RES_EVENT_NO, NULL,
      &joystick_port_map[1], set_joystick_device, (void *)1 },
    { NULL }
};

static int joystick_check_set(signed long key, int keysetnum, unsigned int joyport)
{
    int column;

    for (column = 0; column < 9; column++) {
        if (key == joykeys[keysetnum][column]) {
            if (joypad_bits[column]) {
                /*joystick_value[joyport] |= joypad_bits[column];*/
                joystick_set_value_or(joyport, (BYTE)joypad_bits[column]);
                joypad_status[keysetnum][column] = 1;
            } else {
                /*joystick_value[joyport] = 0;*/
                joystick_set_value_absolute(joyport, 0);
                memset(joypad_status[keysetnum], 0, sizeof(joypad_status[keysetnum]));
            }
            return 1;
        }
    }
    return 0;
}

int joystick_check_clr(signed long key, int keysetnum, unsigned int joyport)
{
    int column;

    for (column = 0; column < 9; column++) {
        if (key == joykeys[keysetnum][column]) {
            /*joystick_value[joyport] &= joyreleaseval(column,
                                                     joypad_status[joynum]);*/
            joystick_set_value_and(joyport, (BYTE)joyreleaseval(column,
                                   joypad_status[keysetnum]));
            joypad_status[keysetnum][column] = 0;
            return 1;
        }
    }
    return 0;
}

void joystick_joypad_clear(void)
{
    memset(joypad_status, 0, sizeof(joypad_status));
}

/*-----------------------------------------------------------------------*/

int joystick_init_resources(void)
{
    resources_register_int(resources_int);

    return 1;
}
#endif

int joystick_arch_init_resources(void)
{
    return resources_register_int(resources_int);
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1, NULL, NULL,
      "JoyDevice1", NULL,
      "", N_("Set device for joystick port 1") },
    { "-joydev2", SET_RESOURCE, 1, NULL, NULL,
      "JoyDevice2", NULL,
      "", N_("Set device for joystick port 2")},
    { NULL }
};

int joystick_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

int joystick_init(void)
{
    joystick_alarm = alarm_new(maincpu_alarm_context, "Joystick",
                               joystick_latch_handler, NULL);

#ifdef COMMON_KBD
    kbd_initialize_numpad_joykeys(joykeys[0]);
#endif

#ifdef USE_UNIX_JOY
    if (!joy_unix_init())
#endif
    {
#ifdef USE_UNIX_OLD_JOY
        old_joystick_init();
#endif
#ifdef HAS_DIGITAL_JOYSTICK
        old_digital_joystick_init();
#endif
    }

    {
    }
    
#ifdef WIN32
#if HAVE_DINPUT
    if (!joy_di_init())
#endif
    {
        joy_winmm_init();
    }
#endif
    
#ifdef MSDOS
    joystick_allegro_init();
#endif

    return 1;
}

/*--------------------------------------------------------------------------*/
int joystick_snapshot_write_module(snapshot_t *s)
{
    snapshot_module_t *m;

    m = snapshot_module_create(s, "JOYSTICK", 1, 0);
    if (m == NULL)
       return -1;

    if (0
        || SMW_BA(m, joystick_value, JOYSTICK_NUM) < 0)
    {
        snapshot_module_close(m);
        return -1;
    }

    if (snapshot_module_close(m) < 0)
        return -1;

    return 0;
}

int joystick_snapshot_read_module(snapshot_t *s)
{
    BYTE major_version, minor_version;
    snapshot_module_t *m;

    m = snapshot_module_open(s, "JOYSTICK",
                             &major_version, &minor_version);
    if (m == NULL) {
        return 0;
    }

    if (0
        || SMR_BA(m, joystick_value, JOYSTICK_NUM) < 0)
    {
        snapshot_module_close(m);
        return -1;
    }

    snapshot_module_close(m);
    return 0;
}

static void joystick_close_one(joy_data_t* joydata) {
    if (joydata->device->close)
        joydata->device->close(joydata->priv);
    if (joydata->device->destroy)
        joydata->device->destroy(joydata->priv);
    lib_free(joydata->name);
    lib_free(joydata->priv);
    lib_free(joydata);
}

void joystick_register_device(joy_data_t* joydata) {
    unsigned int i = joystick_device_num();

    if (i < MAX_HW_JOY_DRIVERS) {
        joy_devices[i] = joydata;
    } else {
        joystick_close_one(joydata);
    }
}

unsigned int joystick_device_num(void)
{
    unsigned int i;

    for (i = 0; joy_devices[i] && i < MAX_HW_JOY_DRIVERS; i++);

    return i;
}

const char* joystick_device_name(unsigned int i) {
    return joy_devices[i]->name;
}

void joystick_close(void) {
    int i;

    for (i = 0; i < MAX_HW_JOY_DRIVERS; i++) {
        if (joy_devices[i]) {
            joystick_close_one(joy_devices[i]);
            joy_devices[i]=NULL;
        }
    }
}

void joystick_update(void) {
    if (joystick_port_map[0] >= JOYDEV_HW_BASE)
        joy_devices[joystick_port_map[0] - JOYDEV_HW_BASE]->device->poll(1, joy_devices[joystick_port_map[0] - JOYDEV_HW_BASE]->priv);
    if (joystick_port_map[1] >= JOYDEV_HW_BASE)
        joy_devices[joystick_port_map[1] - JOYDEV_HW_BASE]->device->poll(2, joy_devices[joystick_port_map[1] - JOYDEV_HW_BASE]->priv);
}

int joystick_kbd_update_set(signed long key) {
    if (joystick_port_map[0] == JOYDEV_NUMPAD
     || joystick_port_map[0] == JOYDEV_KEYSET1
     || joystick_port_map[0] == JOYDEV_KEYSET2) {
        if (joystick_check_set(key, joystick_port_map[0] - JOYDEV_NUMPAD, 1))
            return 1;
    }
    if (joystick_port_map[1] == JOYDEV_NUMPAD
     || joystick_port_map[1] == JOYDEV_KEYSET1
     || joystick_port_map[1] == JOYDEV_KEYSET2) {
        if (joystick_check_set(key, joystick_port_map[1] - JOYDEV_NUMPAD, 2))
            return 1;
    }

    return 0;
}

int joystick_kbd_update_clr(signed long key) {
    if (joystick_port_map[0] == JOYDEV_NUMPAD
     || joystick_port_map[0] == JOYDEV_KEYSET1
     || joystick_port_map[0] == JOYDEV_KEYSET2) {
        if (joystick_check_clr(key, joystick_port_map[0] - JOYDEV_NUMPAD, 1))
            return 1;
    }
    if (joystick_port_map[1] == JOYDEV_NUMPAD
     || joystick_port_map[1] == JOYDEV_KEYSET1
     || joystick_port_map[1] == JOYDEV_KEYSET2) {
        if (joystick_check_clr(key, joystick_port_map[1] - JOYDEV_NUMPAD, 2))
            return 1;
    }

    return 0;
}

