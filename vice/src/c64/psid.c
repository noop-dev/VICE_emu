/*
 * psid.c - PSID file handling.
 *
 * Written by
 *  Dag Lem <resid@nimrod.no>
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "archdep.h"
#include "c64mem.h"
#include "cmdline.h"
#include "drive.h"
#include "interrupt.h"
#include "log.h"
#include "machine.h"
#include "psid.h"
#include "resources.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "vsidui.h"
#include "vsync.h"
#include "zfile.h"

typedef struct psid_s {
  /* PSID data */
  WORD version;
  WORD data_offset;
  WORD load_addr;
  WORD init_addr;
  WORD play_addr;
  WORD songs;
  WORD start_song;
  DWORD speed;
  BYTE name[32];
  BYTE author[32];
  BYTE copyright[32];
  WORD flags;
  BYTE start_page;
  BYTE max_pages;
  WORD reserved;
  WORD data_size;
  BYTE data[65536];

  /* Non-PSID data */
  DWORD frames_played;
} psid_t;


#define PSID_V1_DATA_OFFSET 0x76
#define PSID_V2_DATA_OFFSET 0x7c

int psid_ui_set_tune(resource_value_t tune, void *param);

static psid_t* psid = NULL;
static int psid_tune = 0;    /* app_resources.PSIDTune */

static resource_t resources[] = {
    { "PSIDTune", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &psid_tune, psid_ui_set_tune, NULL },
    { NULL }
};

int psid_init_resources(void)
{
    return resources_register(resources);
}

static int cmdline_vsid_mode(const char *param, void *extra_param)
{
    vsid_mode = 1;
    return 0;
}

static int cmdline_psid_tune(const char *param, void *extra_param)
{
  psid_tune = atoi(param);
  return 0;
}

static cmdline_option_t cmdline_options[] =
{
  /* The Video Standard options are copied from drive-cmdline-options.c */
  { "-pal", SET_RESOURCE, 0, NULL, NULL, "VideoStandard",
    (resource_value_t) DRIVE_SYNC_PAL,
    NULL, "Use PAL sync factor" },
  { "-ntsc", SET_RESOURCE, 0, NULL, NULL, "VideoStandard",
    (resource_value_t) DRIVE_SYNC_NTSC,
    NULL, "Use NTSC sync factor" },
  { "-ntscold", SET_RESOURCE, 0, NULL, NULL, "VideoStandard",
    (resource_value_t) DRIVE_SYNC_NTSCOLD,
    NULL, "Use old NTSC sync factor" },
  { "-vsid", CALL_FUNCTION, 0, cmdline_vsid_mode, NULL, NULL, NULL,
    NULL, "SID player mode" },
  { "-tune", CALL_FUNCTION, 1, cmdline_psid_tune, NULL, NULL, NULL,
    "<number>", "Specify PSID tune <number>" },
  { NULL }
};

int psid_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}


static WORD psid_extract_word(BYTE** buf)
{
  WORD word = (*buf)[0] << 8 | (*buf)[1];
  *buf += 2;
  return word;
}


int psid_load_file(const char* filename)
{
  FILE* f;
  BYTE buf[PSID_V2_DATA_OFFSET + 2];
  BYTE* ptr = buf;
  unsigned int length;

  if (!(f = zfopen(filename, MODE_READ))) {
    return -1;
  }

  free(psid);
  psid = xmalloc(sizeof(psid_t));

  if (fread(ptr, 1, 6, f) != 6 || memcmp(ptr, "PSID", 4) != 0) {
    goto fail;
  }

  ptr += 4;
  psid->version = psid_extract_word(&ptr);

  if (psid->version < 1 || psid->version > 2) {
    log_error(LOG_DEFAULT, "VSID: Unknown PSID version number: %d.",
	      (int)psid->version);
    goto fail;
  }

  length = (unsigned int)((psid->version == 1
           ? PSID_V1_DATA_OFFSET : PSID_V2_DATA_OFFSET) - 6);

  if (fread(ptr, 1, length, f) != length) {
    log_message(LOG_DEFAULT, "VSID: Error reading PSID header.");
    goto fail;
  }

  psid->data_offset = psid_extract_word(&ptr);
  psid->load_addr = psid_extract_word(&ptr);
  psid->init_addr = psid_extract_word(&ptr);
  psid->play_addr = psid_extract_word(&ptr);
  psid->songs = psid_extract_word(&ptr);
  psid->start_song = psid_extract_word(&ptr);
  psid->speed = psid_extract_word(&ptr) << 16;
  psid->speed |= psid_extract_word(&ptr);
  psid->frames_played = 0;
  memcpy(psid->name, ptr, 32);
  psid->name[31] = '\0';
  ptr += 32;
  memcpy(psid->author, ptr, 32);
  psid->author[31] = '\0';
  ptr += 32;
  memcpy(psid->copyright, ptr, 32);
  psid->copyright[31] = '\0';
  ptr += 32;
  if (psid->version == 2) {
    psid->flags = psid_extract_word(&ptr);
    psid->start_page = *ptr++;
    psid->max_pages = *ptr++;
    psid->reserved = psid_extract_word(&ptr);
  }
  else {
    psid->flags = 0;
    psid->start_page = 0;
    psid->max_pages = 0;
    psid->reserved = 0;
  }

  /* Check for SIDPLAYER MUS files. */
  if (psid->flags & 0x01) {
    log_error(LOG_DEFAULT, "VSID: SIDPLAYER MUS files not supported.");
    goto fail;
  }

  /* Zero load address => the load address is stored in the
     first two bytes of the binary C64 data. */
  if (psid->load_addr == 0) {
    if (fread(ptr, 1, 2, f) != 2) {
      log_message(LOG_DEFAULT, "VSID: Error reading PSID load address.");
      goto fail;
    }
    psid->load_addr = ptr[0] | ptr[1] << 8;
  }

  /* Zero init address => use load address. */
  if (psid->init_addr == 0) {
    psid->init_addr = psid->load_addr;
  }

  /* Read binary C64 data. */
  psid->data_size = fread(psid->data, 1, sizeof(psid->data), f);

  if (ferror(f)) {
    log_message(LOG_DEFAULT, "VSID: Error reading PSID data.");
    goto fail;
  }

  if (!feof(f)) {
    log_message(LOG_DEFAULT, "VSID: More than 64K PSID data.");
    goto fail;
  }

  /* Relocation setup. */
  if (psid->start_page == 0x00) {
    /* Start and end pages. */
    int startp = psid->load_addr >> 8;
    int endp = (psid->load_addr + psid->data_size - 1) >> 8;

    /* Used memory ranges. */
    int used[] = { 0x00, 0x03,
		   0xa0, 0xbf,
		   0xd0, 0xff,
		   startp, endp };
    int pages[256];
    int last_page = 0;
    int i, page, tmp;

    /* Mark used pages in table. */
    memset(pages, 0, sizeof(pages));
    for (i = 0; i < sizeof(used)/sizeof(*used); i += 2) {
      for (page = used[i]; page <= used[i + 1]; page++) {
	pages[page] = 1;
      }
    }

    /* Find largest free range. */
    psid->max_pages = 0x00;
    for (page = 0; page < sizeof(pages)/sizeof(*pages); page++) {
      if (!pages[page]) continue;
      tmp = page - last_page;
      if (tmp > psid->max_pages) {
	psid->start_page = last_page;
	psid->max_pages = tmp;
      }
      last_page = page + 1;
    }

    if (psid->max_pages == 0x00) {
      psid->start_page = 0xff;
    }
  }

  if (psid->start_page == 0xff || psid->max_pages < 2)
  {
    log_error(LOG_DEFAULT, "VSID: No space for driver.");
    goto fail;
  }

  zfclose(f);

  return 0;

fail:
  zfclose(f);
  free(psid);
  psid = NULL;
  return -1;
}


/* Use CBM80 vector to start PSID driver. This is a simple method to
   transfer control to the PSID driver while running in a pure C64
   environment. */
int psid_set_cbm80(ADDRESS vec, ADDRESS addr)
{
  int i;
  BYTE cbm80[] = { 0x00, 0x00, 0x00, 0x00, 0xc3, 0xc2, 0xcd, 0x38, 0x30 };

  cbm80[0] = vec & 0xff;
  cbm80[1] = vec >> 8;

  for (i = 0; i < sizeof(cbm80); i++) {
    ram_store(addr + i, ram_read(0x8000 + i));
    ram_store(0x8000 + i, cbm80[i]);
  }

  return i;
}


void psid_init_tune(void)
{
  int start_song = psid_tune;
  resource_value_t sync, sid_model;
  int i;
  ADDRESS reloc_addr;
  ADDRESS addr;
  int speedbit;
  char* irq;
  char irq_str[20];

  if (!psid) {
    return;
  }

  psid->frames_played = 0;

  reloc_addr = psid->start_page << 8;

  log_message(LOG_DEFAULT, "\nVSID: driver=$%04x, image=$%04x-$%04x, init=$%04x, play=$%04x",
	      reloc_addr,
	      psid->load_addr, psid->load_addr + psid->data_size - 1,
	      psid->init_addr, psid->play_addr);

  /* PAL/NTSC. */
  resources_get_value("VideoStandard", &sync);

  /* MOS6581/MOS8580. */
  resources_get_value("SidModel", &sid_model);

  /* Check tune number. */
  if (start_song == 0) {
    start_song = psid->start_song;
  }
  else if (start_song < 1 || start_song > psid->songs) {
    log_message(LOG_DEFAULT,
		"Warning: Tune out of range.");
    start_song = psid->start_song;
  }

  /* Check for PSID specific file. */
  if (psid->flags & 0x08) {
    log_message(LOG_DEFAULT, "Warning: Image contains PlaySID samples - trying anyway.");
  }

  /* Check tune speed. */
  speedbit = 1;
  for (i = 1; i < start_song && i < 32; i++) {
    speedbit <<= 1;
  }

  irq = psid->speed & speedbit ? "CIA 1" : "VICII";

  if (psid->play_addr) {
      vsid_ui_display_irqtype(irq);
      log_message(LOG_DEFAULT, "Using %s interrupt", irq);
  }
  else {
    log_message(LOG_DEFAULT, "Using custom (%s ?) interrupt", irq);
  }
  
  /*
  vsid_ui_display_irq(psid->play_addr, psid->speed & speedbit);
  */
  vsid_ui_display_tune_nr(start_song);
  vsid_ui_set_default_tune(psid->start_song);
  vsid_ui_display_nr_of_tunes(psid->songs);
  vsid_ui_display_time(0);

  /* Store parameters for psid player. */
  ram_store(0x0306, (BYTE)(psid->init_addr & 0xff));
  ram_store(0x0307, (BYTE)(psid->init_addr >> 8));
  ram_store(0x0308, (BYTE)(psid->play_addr & 0xff));
  ram_store(0x0309, (BYTE)(psid->play_addr >> 8));
  ram_store(0x030a, (BYTE)(psid->songs));
  ram_store(0x030b, (BYTE)(start_song));
  ram_store(0x030c, (BYTE)(psid->speed & 0x0f));
  ram_store(0x030d, (BYTE)((psid->speed >> 8) & 0x0f));
  ram_store(0x030e, (BYTE)((psid->speed >> 16) & 0x0f));
  ram_store(0x030f, (BYTE)(psid->speed >> 24));

  /* Store binary C64 data. */
  for (i = 0; i < psid->data_size; i++) {
    ram_store((ADDRESS)(psid->load_addr + i), (BYTE)(psid->data[i]));
  }
}

void psid_set_tune(int tune)
{
  if (tune == -1) {
    psid_tune = 0;
    free(psid);
    psid = NULL;
  }
  else {
    psid_tune = tune;
  }
}

int psid_ui_set_tune(resource_value_t tune, void *param)
{
  psid_tune = (int)tune == -1 ? 0 : (int)tune;

  psid_set_tune(psid_tune);
  vsync_suspend_speed_eval();
  maincpu_trigger_reset();

  return 0;
}

int psid_tunes(int* default_tune)
{
  *default_tune = psid ? psid->start_song : 0;
  return psid ? psid->songs : 0;
}


void psid_init_driver(void)
{
  BYTE psid_driver[] = {
#include "psiddrv.h"
  };
  char* psid_reloc;
  int psid_size;

  ADDRESS reloc_addr;
  ADDRESS addr;
  int i;
  int sync;

  if (!psid) {
    return;
  }

  /* C64 PAL/NTSC flag. */
  sync = psid->flags & 0x02 ? DRIVE_SYNC_NTSC : DRIVE_SYNC_PAL;
  resources_set_value("VideoStandard", (resource_value_t)sync);

  /* MOS6581/MOS8580 flag. */
  resources_set_value("SidModel", (resource_value_t)(psid->flags & 0x04 ? 1 : 0));

  /* Clear low memory to minimize the damage of PSIDs doing bad reads. */
  for (addr = 0; addr < 0x0800; addr++) {
    ram_store(addr, (BYTE)0x00);
  }

  /* Relocation of C64 PSID driver code. */
  reloc_addr = psid->start_page << 8;
  psid_size = sizeof(psid_driver);
  psid_reloc = memcpy(xmalloc(psid_size), psid_driver, psid_size);

  if (!reloc65(&psid_reloc, &psid_size, reloc_addr)) {
    log_error(LOG_DEFAULT, "VSID: Relocation error.");
    free(psid_reloc);
    psid_set_tune(-1);
    return;
  }

  for (i = 0; i < psid_size; i++) {
    ram_store(reloc_addr + i, psid_reloc[i]);
  }

  free(psid_reloc);

  /* Store binary C64 data. */
  for (i = 0; i < psid->data_size; i++) {
    ram_store(psid->load_addr + i, psid->data[i]);
  }

  /* Skip JMP and CBM80 reset vector. */
  addr = reloc_addr + 3 + 9;

  /* Store parameters for PSID player. */
  ram_store(addr++, (BYTE)(0));
  ram_store(addr++, (BYTE)(psid->songs));
  ram_store(addr++, (BYTE)(psid->load_addr & 0xff));
  ram_store(addr++, (BYTE)(psid->load_addr >> 8));
  ram_store(addr++, (BYTE)(psid->init_addr & 0xff));
  ram_store(addr++, (BYTE)(psid->init_addr >> 8));
  ram_store(addr++, (BYTE)(psid->play_addr & 0xff));
  ram_store(addr++, (BYTE)(psid->play_addr >> 8));
  ram_store(addr++, (BYTE)(psid->speed & 0x0f));
  ram_store(addr++, (BYTE)((psid->speed >> 8) & 0x0f));
  ram_store(addr++, (BYTE)((psid->speed >> 16) & 0x0f));
  ram_store(addr++, (BYTE)(psid->speed >> 24));
  ram_store(addr++, (BYTE)(sync == DRIVE_SYNC_PAL ? 1 : 0));
}


unsigned int psid_increment_frames(void)
{
  if (!psid)
    return 0;

  (psid->frames_played)++;

  return (unsigned int)(psid->frames_played);
}
