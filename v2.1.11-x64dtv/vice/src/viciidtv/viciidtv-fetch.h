/*
 * viciidtv-fetch.h - Phi2 data fetch for the VIC-II DTV emulation.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_VICIIDTV_FETCH_H
#define VICE_VICIIDTV_FETCH_H

#include "types.h"

extern void viciidtv_fetch_start(void);
extern void viciidtv_fetch_stop(void);
extern void viciidtv_fetch_linear_a(void);
extern int viciidtv_fetch_matrix(void);
extern void viciidtv_fetch_graphics(void);

#endif

