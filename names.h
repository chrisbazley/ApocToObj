/*
 *  ApoctoObj - Converts Apocalypse graphics to Wavefront format
 *  Apocalypse object names
 *  Copyright (C) 2020 Christopher Bazley
 */

#ifndef APOCNAMES_H
#define APOCNAMES_H

#include "strb.h"

/* Write the object name at the current position; return EOF on failure, zero on success. */
int get_flat_name(strb_t *name, int index);
int get_obj_name(strb_t *name, int index);

#endif /* APOCNAMES_H */
