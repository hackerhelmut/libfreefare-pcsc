/*-
 * Copyright (C) 2010, Romain Tartiere
 * Copyright (C) 2013, Romuald Conty
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 * 
 * $Id$
 */

#include <cutter.h>
#include <freefare.h>

#include "mifare_desfire_fixture.h"

static nfc_context *context;
static nfc_device *device = NULL;
static MifareTag *tags = NULL;
MifareTag tag = NULL;

void
cut_setup (void)
{
  int res;
  nfc_connstring devices[8];
  size_t device_count;
  
  nfc_init (&context);
  cut_assert_not_null (context, cut_message ("Unable to init libnfc (malloc)"));

  device_count = nfc_list_devices (context, devices, 8);
  if (device_count <= 0)
    cut_omit ("No device found");

  for (size_t i = 0; i < device_count; i++) {

    device = nfc_open (context, devices[i]);
    if (!device) 
      cut_omit ("nfc_open() failed.");

    tags = freefare_get_tags (device);
    cut_assert_not_null (tags, cut_message ("freefare_get_tags() failed"));

    tag = NULL;
    for (int i=0; tags[i]; i++) {
      if (freefare_get_tag_type(tags[i]) == DESFIRE) {
        tag = tags[i];
        res = mifare_desfire_connect (tag);
        cut_assert_equal_int (0, res, cut_message ("mifare_desfire_connect() failed"));

        struct mifare_desfire_version_info version_info;
        res = mifare_desfire_get_version (tag, &version_info);
        cut_assert_equal_int (0, res, cut_message ("mifare_desfire_get_version"));

        if (version_info.hardware.storage_size < 0x18) {
          cut_omit ("DESFire tests require at least a 4K card");
        }

        return;
      }
    }
    nfc_close (device);
    device = NULL;
    freefare_free_tags (tags);
    tags = NULL;
  }
  cut_omit ("No MIFARE DESFire tag on NFC device");
}

void
cut_teardown (void)
{
  if (tag)
      mifare_desfire_disconnect (tag);

  if (tags) {
      freefare_free_tags (tags);
      tags = NULL;
  }

  if (device)
      nfc_close (device);
  
  nfc_exit (context);
}

