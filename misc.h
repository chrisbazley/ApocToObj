/*
 *  ApoctoObj - Converts Apocalypse graphics to Wavefront format
 *  Miscellaneous macro definitions
 *  Copyright (C) 2020 Christopher Bazley
 */

#ifndef MISC_H
#define MISC_H

/* Modify these definitions for Unix or Windows file paths. */
#define PATH_SEPARATOR '.'
#define EXT_SEPARATOR '/'

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

/* Suppress compiler warnings about an unused function argument. */
#define NOT_USED(x) ((void)(x))

#ifdef FORTIFY
#include "Fortify.h"
#endif

#ifdef USE_CBDEBUG

#include "Debug.h"
#include "PseudoIO.h"

#else /* USE_CBDEBUG */

#include <stdio.h>
#include <assert.h>

#define DEBUG_SET_OUTPUT(output_mode, log_name)

#ifdef DEBUG_OUTPUT
#define DEBUGF if (1) printf
#else
#define DEBUGF if (0) printf
#endif /* DEBUG_OUTPUT */

#endif /* USE_CBDEBUG */

#ifdef USE_OPTIONAL
#include <stdio.h>

#undef NULL
#define NULL ((_Optional void *)0)

static inline _Optional FILE *optional_fopen(const char *name, const char *mode)
{
    return fopen(name, mode);
}
#undef fopen
#define fopen(p, n) optional_fopen(p, n)

#else
#define _Optional
#endif

#endif /* MISC_H */
