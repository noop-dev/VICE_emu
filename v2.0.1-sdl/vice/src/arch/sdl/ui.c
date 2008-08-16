/*
 * ui.c - Common UI routines.
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
#include "types.h"

#include "color.h"
#include "palette.h"
#include "ui.h"
#include "uiapi.h"
#include "uicolor.h"
#include "video.h"
#include "videoarch.h"

#include <stdio.h>

/* ----------------------------------------------------------------- */
/* ui.h */

void ui_display_speed(float percent, float framerate, int warp_flag){}
void ui_display_paused(int flag){}
void ui_dispatch_next_event(void){}
void ui_dispatch_events(void){}
extern void ui_check_mouse_cursor(void){}

void archdep_ui_init(int argc, char *argv[])
{
fprintf(stderr,"%s\n",__func__);
}

/* ----------------------------------------------------------------- */
/* uiapi.h */

/* Initialization  */
int ui_resources_init(void)
{
fprintf(stderr,"%s\n",__func__);
}

void ui_resources_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

int ui_cmdline_options_init(void)
{
fprintf(stderr,"%s\n",__func__);
}

int ui_init(int *argc, char **argv)
{
fprintf(stderr,"%s\n",__func__);
}

int ui_init_finish(void)
{
fprintf(stderr,"%s\n",__func__);
}

int ui_init_finalize(void)
{
fprintf(stderr,"%s\n",__func__);
}

void ui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

/* Print an error message.  */
void ui_error(const char *format,...){}

/* Display a mesage without interrupting emulation */
void ui_display_statustext(const char *text, int fade_out){}

/* Drive related UI.  */
void ui_enable_drive_status(ui_drive_enable_t state,
                            int *drive_led_color){}
void ui_display_drive_track(unsigned int drive_number,
                            unsigned int drive_base,
                            unsigned int half_track_number){}
/* The pwm value will vary between 0 and 1000.  */
void ui_display_drive_led(int drive_number, unsigned int pwm1,
                          unsigned int led_pwm2){}
void ui_display_drive_current_image(unsigned int drive_number,
                                    const char *image){}
int ui_extend_image_dialog(void){}

/* Tape related UI */
void ui_set_tape_status(int tape_status){}
void ui_display_tape_motor_status(int motor){}
void ui_display_tape_control_status(int control){}
void ui_display_tape_counter(int counter){}
void ui_display_tape_current_image(const char *image){}

/* Show a CPU JAM dialog.  */
ui_jam_action_t ui_jam_dialog(const char *format, ...){}

/* Update all menu entries.  */
void ui_update_menus(void){}

/* Recording UI */
void ui_display_playback(int playback_status, char *version){}
void ui_display_recording(int recording_status){}
void ui_display_event_time(unsigned int current, unsigned int total){}

/* Joystick UI */
void ui_display_joyport(BYTE *joyport){}

/* Volume UI */
void ui_display_volume(int vol){}

/* ----------------------------------------------------------------- */
/* uicolor.h */

int uicolor_alloc_color(unsigned int red, unsigned int green,
                        unsigned int blue, unsigned long *color_pixel,
                        BYTE *pixel_return)
{
fprintf(stderr,"%s\n",__func__);
}

void uicolor_free_color(unsigned int red, unsigned int green,
                        unsigned int blue, unsigned long color_pixel)
{
fprintf(stderr,"%s\n",__func__);
}

void uicolor_convert_color_table(unsigned int colnr, BYTE *data,
                                 long color_pixel, void *c)
{
fprintf(stderr,"%s\n",__func__);
}

int uicolor_set_palette(struct video_canvas_s *c,
                        const struct palette_s *palette)
{
    unsigned int i, col;
    SDL_PixelFormat *fmt;

    fmt = c->screen->format;

fprintf(stderr,"%s\n",__func__);

    for (i = 0; i < c->palette->num_entries; i++) {
        if (c->depth == 8) {
            col = 0;
        } else {
            col = SDL_MapRGB(fmt, c->palette->entries[i].red, c->palette->entries[i].green, c->palette->entries[i].blue);
        }
        video_render_setphysicalcolor(c->videoconfig, i, col, c->depth);
    }

    if (c->depth > 8) {
        for (i = 0; i < 256; i++) {
            video_render_setrawrgb(i, SDL_MapRGB(fmt, i, 0, 0), SDL_MapRGB(fmt, 0, i, 0), SDL_MapRGB(fmt, 0, 0, i));
        }
    video_render_initraw();
    }

    return 0;
}
