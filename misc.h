/*
 *  ApoctoObj - Converts Apocalypse graphics to Wavefront format
 *  Miscellaneous macro definitions
 *  Copyright (C) 2020 Christopher Bazley
 */

#ifndef MISC_H
#define MISC_H

/* Modify these definitions for Unix or Windows file paths. */
#ifndef PATH_SEPARATOR
#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#elif defined(ACORN_C)
#define PATH_SEPARATOR '.'
#else
#define PATH_SEPARATOR '/'
#endif
#endif

#ifndef EXT_SEPARATOR
#ifdef ACORN_C
#define EXT_SEPARATOR '/' /* e.g. ADFS::4.$.Star3000.Graphics.Earth1/obj */
#else
#define PATH_SEPARATOR '.'
#endif
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

/* Suppress compiler warnings about an unused function argument. */
#define NOT_USED(x) ((void)(x))

#ifdef FORTIFY
#include "fortify.h"
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
#include "Optional.h"
#else
#define _Optional
#endif

#endif /* MISC_H */
