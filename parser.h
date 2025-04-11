/*
 *  ApoctoObj - Converts Apocalypse graphics to Wavefront format
 *  Apocalypse object mesh parser
 *  Copyright (C) 2020 Christopher Bazley
 */

#ifndef PARSER_H
#define PARSER_H

/* ISO C library headers */
#include <stdio.h>
#include <stdbool.h>

/* StreamLib headers */
#include "Reader.h"

bool apoc_to_obj(Reader *in, _Optional FILE *out,
                 const int first, const int last, _Optional const char *name,
                 const long int index_offset,
                 const char *mtl_file, const unsigned int flags);

#endif /* PARSER_H */
