/*
 * fsimage-gcr.c
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
#include <string.h>

#include "diskconstants.h"
#include "diskimage.h"
#include "fsimage-gcr.h"
#include "fsimage.h"
#include "gcr.h"
#include "log.h"
#include "types.h"
#include "util.h"


static log_t fsimage_gcr_log = LOG_ERR;
static const BYTE gcr_image_header_expected[] =
    { 0x47, 0x43, 0x52, 0x2D, 0x31, 0x35, 0x34, 0x31 };
/* Hardcoded/expected values VICE works with:
 * 0x1EF8 - 7928: Maximum container size for a raw GCR track
 * 0x54   -   84: Maximum container size for (half) track pointers
 * 0x00   -    0: GCR image file version number
 */
static const DWORD gcr_container_sizes_expected = 0x1EF85400;

/*-----------------------------------------------------------------------*/
/* Read an entire GCR track from the disk image.  */

int fsimage_gcr_read_track(disk_image_t *image, unsigned int track,
                           unsigned int head, disk_track_t *raw)
{
    int track_len;
    int num_tracks;
    BYTE len[2];
    DWORD gcr_track_p = 0;
    DWORD gcr_speed_p = 0;
    int gap;
    fsimage_t *fsimage;

    fsimage = image->media.fsimage;

    memset(raw->data, 0x00, NUM_MAX_BYTES_TRACK);
    raw->size = 6250;

    if (fsimage->fd == NULL) {
        log_error(fsimage_gcr_log, "Attempt to read without disk image.");
        return -1;
    }

    num_tracks = image->ptracks;
    track -= 2;

    if (track < num_tracks && head == 0) {
        if (fseek(fsimage->fd, 12 + track * 4, SEEK_SET) < 0
                || util_dword_read(fsimage->fd, &gcr_track_p, 1) < 0) {
            log_error(fsimage_gcr_log, "Could not read GCR disk image.");
            return -1;
        }

        if (fseek(fsimage->fd, 12 + (num_tracks + track) * 4, SEEK_SET) < 0
                || util_dword_read(fsimage->fd, &gcr_speed_p, 1) < 0) {
            log_error(fsimage_gcr_log, "Could not read GCR disk image.");
            return -1;
        }
    }

    if (gcr_speed_p > 3) {
        log_error(fsimage_gcr_log, "Variable rate not supported yet.");
        return -1;
    }
    raw->rate = gcr_speed_p;

    if (gcr_track_p != 0) {
        if (fseek(fsimage->fd, gcr_track_p, SEEK_SET) < 0
            || fread(len, 2, 1, fsimage->fd) < 1) {
            log_error(fsimage_gcr_log, "Could not read GCR disk image.");
            return -1;
        }

        track_len = len[0] + len[1] * 256;

        if (track_len < 5000 || track_len > 7928) {
            log_error(fsimage_gcr_log,
                      "Track field length %i is not supported.",
                      track_len);
            return -1;
        }

        raw->size = track_len;

        if (fread(raw->data, track_len, 1, fsimage->fd) < 1) {
            log_error(fsimage_gcr_log, "Could not read GCR disk image.");
            return -1;
        }
        gap = NUM_MAX_BYTES_TRACK - track_len;
        if (gap > 0) {
            memset(raw->data + track_len, 0x55, gap);
        }
    }

    return 0;
}

/*-----------------------------------------------------------------------*/
/* Write an entire GCR track to the disk image.  */

int fsimage_gcr_write_track(disk_image_t *image, unsigned int track,
                            unsigned int head, disk_track_t *raw)
{
    int gap;
    unsigned int num_tracks;
    BYTE len[2];
    DWORD gcr_track_p = 0;
    DWORD gcr_speed_p;
    fsimage_t *fsimage = image->media.fsimage;

    if (head != 0) {
        log_error(fsimage_gcr_log,
                  "Attempt to write to non-existing side.");
        return -1;
    }

    num_tracks = image->ptracks;
    track -= 2;

    if (track >= num_tracks) {
        log_error(fsimage_gcr_log, "Could not extend GCR disk image.");
        return -1;
    }

    if (fseek(fsimage->fd, 12 + track * 4, SEEK_SET) < 0
        || util_dword_read(fsimage->fd, &gcr_track_p, 1) < 0) {
        log_error(fsimage_gcr_log, "Could not read GCR disk image header.");
        return -1;
    }

    if (gcr_track_p == 0) {
        if (fseek(fsimage->fd, 0, SEEK_END) < 0) {
            log_error(fsimage_gcr_log, "Could not extend GCR disk image.");
            return -1;
        }
        gcr_track_p = ftell(fsimage->fd);
    } else if (fseek(fsimage->fd, gcr_track_p, SEEK_SET) < 0) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image.");
        return -1;
    }

    len[0] = raw->size % 256;
    len[1] = raw->size / 256;
    if (fwrite(len, 2, 1, fsimage->fd) < 1) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image.");
        return -1;
    }

    /* Clear gap between the end of the actual track and the start of
       the next track.  */
    gap = NUM_MAX_BYTES_TRACK - raw->size;
    if (gap > 0) {
        memset(raw->data + raw->size, 0, gap);
    }

    if (fwrite(raw->data, NUM_MAX_BYTES_TRACK, 1,
        fsimage->fd) < 1) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image.");
        return -1;
    }

    fseek(fsimage->fd, 12 + track * 4, SEEK_SET);
    if (util_dword_write(fsimage->fd, &gcr_track_p, 1) < 0) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image header.");
        return -1;
    }

    gcr_speed_p = raw->rate;
    fseek(fsimage->fd, 12 + (num_tracks + track) * 4, SEEK_SET);
    if (util_dword_write(fsimage->fd, &gcr_speed_p, 1) < 0) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image header.");
        return -1;
    }

    /* Make sure the stream is visible to other readers.  */
    fflush(fsimage->fd);

    return 0;
}

/*-----------------------------------------------------------------------*/
/* Read a sector from the GCR disk image.  */

int fsimage_gcr_read_sector(disk_image_t *image, BYTE *buf,
                               unsigned int track, unsigned int sector)
{
    BYTE data[NUM_MAX_BYTES_TRACK];
    disk_track_t raw;

    if (track > image->ltracks || track < 1) {
        log_error(fsimage_gcr_log,
                  "Track %i out of bounds.  Cannot read GCR track.",
                  track);
        return -1;
    }

    raw.data = data;

    if (fsimage_gcr_read_track(image, (track - 1) * 2, 0,
        &raw) < 0) {
        log_error(fsimage_gcr_log,
                  "Cannot read track %i from GCR image.", track);
        return -1;
    }

    if (gcr_read_sector(raw.data, raw.size, buf, track, sector) < 0) {
        log_error(fsimage_gcr_log,
                  "Cannot find track: %i sector: %i within GCR image.",
                  track, sector);
        return -1;
    }
    return 0;
}


/*-----------------------------------------------------------------------*/
/* Write a sector to the GCR disk image.  */

int fsimage_gcr_write_sector(disk_image_t *image, BYTE *buf,
                                unsigned int track, unsigned int sector)
{
    disk_track_t raw;
    BYTE data[NUM_MAX_BYTES_TRACK];

    if (track > image->ltracks || track < 1) {
        log_error(fsimage_gcr_log,
                  "Track %i out of bounds.  Cannot write GCR sector",
                  track);
        return -1;
    }

    raw.data = data;

    if (fsimage_gcr_read_track(image, (track - 1) * 2, 0, &raw) < 0) {
        log_error(fsimage_gcr_log,
                  "Cannot read track %i from GCR image.", track);
        return -1;
    }

    if (gcr_write_sector(raw.data, raw.size, buf, track, sector) < 0) {
        log_error(fsimage_gcr_log,
                  "Could not find track %i sector %i in disk image",
                  track, sector);
        return -1;
    }

    if (disk_image_write_track(image, (track - 1) * 2, 0, &raw) < 0) {
        log_error(fsimage_gcr_log,
                  "Failed writing track %i to disk image.", track);
        return -1;
    }

    return 0;
}

int fsimage_gcr_create(disk_image_t *image, unsigned int type)
{
    fsimage_t *fsimage = image->media.fsimage;
    BYTE gcr_header[12], gcr_track[NUM_MAX_BYTES_TRACK + 2], *gcrptr;
    DWORD gcr_track_p[MAX_TRACKS_1541 * 2];
    DWORD gcr_speed_p[MAX_TRACKS_1541 * 2];
    unsigned int track, sector;
    BYTE rawdata[256];
    int i;

    strcpy((char *)gcr_header, "GCR-1541");

    gcr_header[8] = 0;
    gcr_header[9] = MAX_TRACKS_1541 * 2;
    gcr_header[10] = NUM_MAX_BYTES_TRACK % 256;
    gcr_header[11] = NUM_MAX_BYTES_TRACK / 256;

    if (fwrite((char *)gcr_header, sizeof(gcr_header), 1, fsimage->fd) < 1) {
        log_error(fsimage_gcr_log, "Cannot write GCR header.");
        return -1;
    }

    memset(gcr_track_p, 0, sizeof(gcr_track_p));
    memset(gcr_speed_p, 0, sizeof(gcr_speed_p));

    for (track = 0; track < NUM_TRACKS_1541; track++) {
        gcr_track_p[track * 2] = 12 + MAX_TRACKS_1541 * 16 + track * 7930;
        gcr_speed_p[track * 2] = disk_image_speed_map_1541(track);
    }

    if (util_dword_write(fsimage->fd, gcr_track_p, util_arraysize(gcr_track_p)) < 0) {
        log_error(fsimage_gcr_log, "Cannot write track header.");
        return -1;
    }
    if (util_dword_write(fsimage->fd, gcr_speed_p, util_arraysize(gcr_speed_p)) < 0) {
        log_error(fsimage_gcr_log, "Cannot write speed header.");
        return -1;
    }
    memset(rawdata, 0, sizeof(rawdata));
    for (track = 1; track <= NUM_TRACKS_1541; track++) {
        gcrptr = gcr_track;
        i = disk_image_raw_track_size_1541(track);
        *gcrptr++ = i % 256;
        *gcrptr++ = i / 256;
        memset(gcrptr, 0x55, NUM_MAX_BYTES_TRACK);

        for (sector = 0;
        sector < disk_image_sector_per_track(image, track);
        sector++) {

            gcr_convert_sector_to_GCR(rawdata, gcrptr, track, sector,
                                      image->diskID[0], image->diskID[1], 0);

            gcrptr += SECTOR_GCR_SIZE_WITH_HEADER + disk_image_gap_size_1541(track);
        }
        if (fwrite((char *)gcr_track, sizeof(gcr_track), 1, fsimage->fd) < 1 ) {
            log_error(fsimage_gcr_log, "Cannot write track data.");
            return -1;
        }
    }

   return 0;
}
/*-----------------------------------------------------------------------*/

void fsimage_gcr_init(void)
{
    fsimage_gcr_log = log_open("Filesystem Image GCR");
}

