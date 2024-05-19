/*
 *  ApoctoObj - Converts Apocalypse graphics to Wavefront format
 *  Apocalypse object names
 *  Copyright (C) 2020 Christopher Bazley
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public Licence as published by
 *  the Free Software Foundation; either version 2 of the Licence, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public Licence for more details.
 *
 *  You should have received a copy of the GNU General Public Licence
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* ISO library header files */
#include <stdio.h>

/* Local header files */
#include "names.h"
#include "misc.h"

const char *get_flat_name(const int index)
{
  static char buffer[64];
  sprintf(buffer, "flat_%d", index);
  return buffer;
}

const char *get_obj_name(const int index)
{
  static char buffer[64];
  static struct {
    int first;
    int last;
    const char *string;
  } names[] = {
    /* Although other object meshes are recognizable, these are
       the only named targets in 'Apocalypse'. */
    {   1,   1, "guard_tower_1"},
    {   4,   5, "guard_tower_top"},
    {   6,   6, "guard_tower_2"},
    {  14,  14, "processing_factory"},
    {  15,   7, "obelisk_1"},
    {  22,  22, "saucer_1"}, /* has missing vertex data */
    {  23,  23, "simtaal_quak_1"},
    {  24,  24, "simtaal_quak_2"},
    {  25,  25, "simtaal_quak_3"},
    {  26,  26, "snail_rider"},
    {  31,  31, "saucer_2"},
    {  36,  35, "rakonan_barftub"},
    {  40,  42, "ground_wasp"},
    {  44,  46, "tilexu_floater"},
    {  55,  55, "silicon_vat"},
    {  56,  56, "static_release"},
    {  57,  57, "ground_scanner"},
    {  58,  61, "pumping_station"},
    {  62,  65, "seismic_hammer"},
    {  66,  69, "krypton_breather"},
    {  70,  70, "ground_transport_1"},
    {  71,  71, "ground_transport_2"},
    {  72,  74, "wind_generator_1"},
    {  75,  78, "wind_generator_2"},
    {  80,  87, "rakonan_gomjabba"},
    {  88,  95, "proton_flapper"},
    {  96, 103, "electron_grinder"},
    { 104, 111, "wave_generator"},
    { 112, 119, "postal_teleport"},
    { 120, 127, "climatic_ticker"},
    { 128, 135, "weirding_flasher"},
    { 136, 143, "aldebran_linkbat"},
    { 144, 151, "argon_storehouse"},
    { 152, 159, "thermal_riser"},
    { 166, 167, "obelisk_2"},
    { 181, 187, "snailherd"},
    { 192, 199, "lhaktal_gourd"},
  };
  const char *n = NULL;
  assert(index >= 0);

  for (size_t i = 0; (n == NULL) && (i < ARRAY_SIZE(names)); ++i) {
    if ((names[i].first == names[i].last) && (index == names[i].first)) {
      n = names[i].string;
    } else {
      int frame = -1;
      if (names[i].first > names[i].last) {
        if (names[i].first == index) {
          frame = 0;
        } else if (names[i].last == index) {
          frame = 1;
        }
      } else if (index >= names[i].first && index <= names[i].last) {
        frame = index - names[i].first;
      }
      if (frame >= 0) {
        sprintf(buffer, "%s_f%d", names[i].string, frame);
        n = buffer;
      }
    }
  }

  if (n == NULL) {
    sprintf(buffer, "apocalypse_%d", index);
    n = buffer;
  }

  return n;
}
