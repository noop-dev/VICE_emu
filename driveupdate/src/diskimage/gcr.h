/*
 * grc.h - GCR handling.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Daniel Sladic <sladic@eecg.toronto.edu>
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

#ifndef VICE_GCR_H
#define VICE_GCR_H

#include "types.h"

struct disk_track_s;
typedef struct gcr_header_s {
    BYTE sector, track, id2, id1;
} gcr_header_t;

#define SECTOR_GCR_SIZE_WITH_HEADER 340

extern void gcr_convert_sector_to_GCR(BYTE *buffer, BYTE *ptr, gcr_header_t *header,
                                      int gap, int sync, BYTE error_code);
extern int gcr_read_sector(struct disk_track_s *raw, BYTE *data, BYTE sector);
extern int gcr_write_sector(struct disk_track_s *raw, BYTE *data, BYTE sector);
#endif

