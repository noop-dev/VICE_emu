/*
 * uifilereq.c - Common SDL file selection dialog functions.
 *
 * Written by
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
#include "types.h"

#include <SDL/SDL.h>
#include <string.h>

#include "archdep.h"
#include "ioutil.h"
#include "lib.h"
#include "ui.h"
#include "uimenu.h"
#include "uifilereq.h"
#include "util.h"

static ui_menu_filereq_mode_t filereq_mode;
static menu_draw_t *menu_draw;

/* ------------------------------------------------------------------ */
/* static functions */

static char* sdl_ui_get_file_selector_entry(ioutil_dir_t *directory, int offset, int *isdir)
{
    if (offset >= (directory->dir_amount + directory->file_amount + 2))
    {
        return NULL;
    }

    if ((filereq_mode == FILEREQ_MODE_CHOOSE_FILE) && (offset == 0))
    {
        *isdir = 0;
        return "<enter filename>";
    }

    if ((filereq_mode == FILEREQ_MODE_CHOOSE_DIR) && (offset == 0))
    {
        *isdir = 0;
        return "<choose current directory>";
    }

    if (offset == 1)
    {
        *isdir = 0;
        return "<enter path>";
    }

    if (offset >= (directory->dir_amount + 2))
    {
        *isdir = 0;
        return directory->files[offset - (directory->dir_amount + 2)].name;
    }
    else
    {
        *isdir = 1;
        return directory->dirs[offset - 2].name;
    }
}

static void sdl_ui_file_selector_redraw(ioutil_dir_t *directory, const char *title, int offset, int num_items, int more)
{
    int i, j, isdir = 0;
    char* title_string;
    char* name;

    title_string = lib_malloc(strlen(title)+8);
    sprintf(title_string, "%s %s", title, (offset) ? ((more) ? "(<- ->)" : "(<-)") : ((more) ? "(->)" : ""));

    sdl_ui_clear();
    sdl_ui_display_title(title_string);
    lib_free(title_string);

    for (i = 0; i < num_items; ++i)
    {
        j = menu_draw->first_x;
        name = sdl_ui_get_file_selector_entry(directory, offset+i, &isdir);
        if (isdir)
        {
            j += 1 + sdl_ui_print("(D)", menu_draw->first_x, i+menu_draw->first_y);
        }
        sdl_ui_print(name, j, i+menu_draw->first_y);
    }
}

/* ------------------------------------------------------------------ */
/* External UI interface */

char* sdl_ui_file_selection_dialog(const char* title, ui_menu_filereq_mode_t mode)
{
    int total, dirs, files, menu_max;
    int active = 1;
    int offset = 0;
    int redraw = 1;
    char *retval = NULL;
    int cur = 0, cur_old = -1;
    ioutil_dir_t *directory;
    char *current_dir;
    char *backup_dir;
    char *inputstring;
    unsigned int maxpathlen;

    menu_draw = sdl_ui_get_menu_param();
    filereq_mode = mode;

    maxpathlen = ioutil_maxpathlen();

    current_dir = (char *)lib_malloc(maxpathlen);

    ioutil_getcwd(current_dir, maxpathlen);
    backup_dir = lib_stralloc(current_dir);

    directory = ioutil_opendir(".");
    if (directory == NULL)
    {
        return NULL;
    }

    dirs = directory->dir_amount;
    files = directory->file_amount;
    total = dirs + files + 2;
    menu_max = menu_draw->max_text_y - menu_draw->first_y;

    while(active)
    {
        if (redraw)
        {
            sdl_ui_file_selector_redraw(directory, title, offset,
                                        (total-offset > menu_max) ? menu_max : total-offset,
                                        (total-offset > menu_max) ? 1 : 0);
            redraw = 0;
        }
        sdl_ui_display_cursor(cur, cur_old);
        sdl_ui_refresh();

        switch(sdl_ui_menu_poll_input())
        {
            case MENU_ACTION_UP:
                if (cur > 0)
                {
                    cur_old = cur;
                    --cur;
                }
                else
                {
                    if (offset > 0)
                    {
                        offset--;
                        redraw = 1;
                    }
                }
                break;
            case MENU_ACTION_LEFT:
                offset -= menu_max;
                if (offset < 0)
                {
                    offset = 0;
                    cur_old = -1;
                    cur = 0;
                }
                redraw = 1;
                break;
            case MENU_ACTION_DOWN:
                if (cur < (menu_max - 1))
                {
                    if ((cur+offset) < total-1)
                    {
                        cur_old = cur;
                        ++cur;
                    }
                }
                else
                {
                    if (offset < (total-menu_max))
                    {
                        offset++;
                        redraw = 1;
                    }
                }
                break;
            case MENU_ACTION_RIGHT:
                offset += menu_max;
                if (offset >= total)
                {
                    offset = total - 1;
                    cur_old = -1;
                    cur = 0;
                }
                else if ((cur+offset) >= total)
                {
                    cur_old = -1;
                    cur = total-offset-1;
                }
                redraw = 1;
                break;
            case MENU_ACTION_SELECT:
                switch (offset+cur)
                {
                    case 0:
                        if (filereq_mode == FILEREQ_MODE_CHOOSE_FILE)
                        {
                            inputstring = sdl_ui_text_input_dialog("Enter filename", NULL);
                            if (inputstring == NULL)
                            {
                                redraw = 1;
                            }
                            else
                            {
                                if (archdep_path_is_relative(inputstring))
                                {
                                    retval = inputstring;
                                }
                                else
                                {
                                    retval = util_concat(current_dir, FSDEV_DIR_SEP_STR, inputstring, NULL);
                                    lib_free(inputstring);
                                }
                            }
                        }
                        else
                        {
                            retval = lib_stralloc(current_dir);
                        }
                        active = 0;
                        break;
                    case 1:
                        inputstring = sdl_ui_text_input_dialog("Enter path", NULL);
                        if (inputstring != NULL)
                        {
                            ioutil_chdir(inputstring);
                            lib_free(inputstring);
                            ioutil_closedir(directory);
                            ioutil_getcwd(current_dir, maxpathlen);
                            directory = ioutil_opendir(".");
                            offset = 0;
                            cur_old = -1;
                            cur = 0;
                            dirs = directory->dir_amount;
                            files = directory->file_amount;
                            total = dirs + files + 2;
                        }
                        redraw = 1;
                        break;
                    default:
                        if (offset+cur < (dirs + 2))
                        {
                            ioutil_chdir(directory->dirs[offset + cur - 2].name);
                            ioutil_closedir(directory);
                            ioutil_getcwd(current_dir, maxpathlen);
                            directory = ioutil_opendir(".");
                            offset = 0;
                            cur_old = -1;
                            cur = 0;
                            dirs = directory->dir_amount;
                            files = directory->file_amount;
                            total = dirs + files + 2;
                            redraw = 1;
                        }
                        else
                        {
                            retval = util_concat(current_dir, FSDEV_DIR_SEP_STR, directory->files[offset+cur-dirs-2].name, NULL);
                            active = 0;
                        }
                        break;
                }
                break;
            case MENU_ACTION_CANCEL:
            case MENU_ACTION_EXIT:
                retval = NULL;
                active = 0;
                ioutil_chdir(backup_dir);
                break;
            default:
                SDL_Delay(10);
                break;
        }
    }
    ioutil_closedir(directory);
    lib_free(current_dir);
    lib_free(backup_dir);

    return retval;
}
