/*
 *  ApoctoObj - Converts Apocalypse graphics to Wavefront format
 *  Command-line parser
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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>

/* CBUtilLib headers */
#include "StrExtra.h"
#include "ArgUtils.h"
#include "StringBuff.h"

/* StreamLib headers */
#include "Reader.h"
#include "ReaderRaw.h"

/* Local headers */
#include "flags.h"
#include "parser.h"
#include "version.h"
#include "misc.h"

enum {
  LoadAddress = 0x8f00,
  FlatIndexAddress = 0x18b64,
  MeshIndexAddress = 0x19b6c,
  FlatIndexOffset = FlatIndexAddress - LoadAddress,
  MeshIndexOffset = MeshIndexAddress - LoadAddress,
  MaxNumObjects = 200,
};

static bool process_file(_Optional const char * const in_file,
                         _Optional const char * const output_file,
                         const int first, const int last,
                         _Optional const char * const name,
                         const long int index_offset,
                         const char * const mtl_file,
                         const unsigned int flags, const bool time)
{
  _Optional FILE *out = NULL, *in = NULL;
  bool success = true;

  assert(!(flags & ~FLAGS_ALL));

  if (in_file != NULL) {
    /* An explicit input file name was specified, so open it */
    if (flags & FLAGS_VERBOSE)
      printf("Opening input file '%s'\n", in_file);

    in = fopen(&*in_file, "rb");
    if (in == NULL) {
      fprintf(stderr, "Failed to open input file '%s': %s\n",
                      in_file, strerror(errno));
      success = false;
    }
  } else {
    /* Default index is from standard input stream */
    fprintf(stderr, "Reading from stdin...\n");
    in = stdin;
  }

  if (success) {
    if (flags & FLAGS_LIST) {
      out = NULL; /* No OBJ-format output */
    } else if (output_file != NULL) {
      if (flags & FLAGS_VERBOSE)
        printf("Opening output file '%s'\n", output_file);

      out = fopen(&*output_file, "w");
      if (out == NULL) {
        fprintf(stderr, "Failed to open output file '%s': %s\n",
                        output_file, strerror(errno));
        success = false;
      }
    } else {
      /* Default output is to standard output stream */
      out = stdout;
    }
  }

  if (success && in) {
    const clock_t start_time = time ? clock() : 0;

    Reader reader;
    reader_raw_init(&reader, &*in);

    if (success) {
      success = apoc_to_obj(&reader, out, first, last, name,
                            index_offset, mtl_file, flags);
    }

    reader_destroy(&reader);

    if (success && time)
    {
      printf("Time taken: %.2f seconds\n",
             (double)(clock_t)(clock() - start_time) / CLOCKS_PER_SEC);
    }
  }

  if (in != NULL && in != stdin) {
    if (flags & FLAGS_VERBOSE)
      puts("Closing input file");
    fclose(&*in);
  }

  if (out != NULL && out != stdout) {
    if (flags & FLAGS_VERBOSE)
      puts("Closing output file");

    if (fclose(&*out)) {
      fprintf(stderr, "Failed to close output file '%s': %s\n",
                      output_file, strerror(errno));
      success = false;
    }
  }

  /* Delete malformed output unless debugging is enabled or
     it may actually be the index (still intact) */
  if (!success && !(flags & FLAGS_VERBOSE) && out != NULL && out != stdout &&
      output_file) {
    remove(&*output_file);
  }

  return success;
}

static int syntax_msg(FILE * const f, const char * const path)
{
  assert(f != NULL);
  assert(path != NULL);

  const char * const leaf = strtail(path, PATH_SEPARATOR, 1);
  fprintf(f,
          "usage: %s [switches] [<input-file> [<output-file>]]\n"
          "or     %s -batch [switches] <file1> [<file2> .. <fileN>]\n"
          "If no input file is specified, it reads from stdin.\n"
          "If no output file is specified, it writes to stdout.\n"
          "In batch processing mode, output file names are generated by appending\n"
          "extension 'obj' to the input file names.\n"
          "If a material library file is specified then a reference to it will be\n"
          "inserted in the output. This file is not created, read or written.\n",
          leaf, leaf);

  fputs("Switches (names may be abbreviated):\n"
        "  -help               Display this text\n"
        "  -batch              Process a batch of files (see above)\n"
        "  -flats              Convert or list flats instead of polygon meshes\n"
        "  -list               List objects instead of converting them\n"
        "  -index N            Object number to convert or list (default is all)\n"
        "  -first N            First object number to convert or list\n"
        "  -last N             Last object number to convert or list\n"
        "  -name <name>        Object name to convert or list (default is all)\n"
        "  -offset N           Byte offset to object address table in input\n"
        "  -outfile <name>     Write output to the named file instead of stdout\n"
        "  -time               Show the total time for each file processed\n"
        "  -verbose or -debug  Emit debug information (and keep bad output)\n", f);

  fputs("Switches to customize the output:\n"
        "  -mtllib name        Specify a material library file (default sf3k.mtl)\n"
        "  -human              Output readable material names\n"
        "  -false              Assign false colours for visualization\n"
        "  -unused             Include unused vertices in the output\n"
        "  -duplicate          Include duplicate vertices in the output\n"
        "  -negative           Output negative vertex indices\n"
        "  -clip               Clip overlapping coplanar polygons\n"
        "  -flip               Flip back-facing flats\n"
        "  -fans               Split complex polygons into triangle fans\n"
        "  -strips             Split complex polygons into triangle strips\n", f);

  return EXIT_FAILURE;
}

#ifdef FORTIFY
int real_main(int argc, const char *argv[]);

int main(int argc, const char *argv[])
{
  unsigned long limit;
  int rtn = EXIT_FAILURE;
  for (limit = 0; rtn != EXIT_SUCCESS; ++limit)
  {
    rewind(stdin);
    clearerr(stdout);
    printf("------ Allocation limit %ld ------\n", limit);
    Fortify_SetNumAllocationsLimit(limit);
    Fortify_EnterScope();
    rtn = real_main(argc, argv);
    Fortify_LeaveScope();
    Fortify_SetNumAllocationsLimit(ULONG_MAX);
  }
  return rtn;
}

int real_main(int argc, const char *argv[])
#else
int main(int argc, const char *argv[])
#endif
{
  int n, first = -1, last = -1;
  long int index_offset = -1;
  unsigned int flags = 0;
  _Optional const char *name = NULL;
  bool time = false, batch = false;
  int rtn = EXIT_SUCCESS;
  _Optional const char *in_file = NULL, *output_file = NULL;
  const char *mtl_file = "sf3k.mtl";

  assert(argc > 0);
  assert(argv != NULL);

  DEBUG_SET_OUTPUT(DebugOutput_StdErr, "");

  /* Parse any options specified on the command line */
  for (n = 1; n < argc && argv[n][0] == '-'; n++) {
    const char *opt = argv[n] + 1;

    if (is_switch(opt, "batch", 1)) {
      /* Enable batch processing mode */
      batch = true;
    } else if (is_switch(opt, "clip", 1)) {
      /* Enable clipping of coplanar polygons */
      flags |= FLAGS_CLIP_POLYGONS;
    } else if (is_switch(opt, "debug", 2)) {
      /* Enable debugging output */
      flags |= FLAGS_VERBOSE;
    } else if (is_switch(opt, "duplicate", 2)) {
      /* Enable output of duplicate vertices */
      flags |= FLAGS_DUPLICATE;
    } else if (is_switch(opt, "false", 3)) {
      /* Enable false primitive colours */
      flags |= FLAGS_FALSE_COLOUR;
    } else if (is_switch(opt, "fans", 3)) {
      /* Enable decomposition of complex polygons into triangle fans */
      flags |= FLAGS_TRIANGLE_FANS;
    } else if (is_switch(opt, "first", 2)) {
      /* First object number to convert was specified */
      long int objnum;
      if (!get_long_arg("first", &objnum, 0, MaxNumObjects - 1, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
      first = (int)objnum;
    } else if (is_switch(opt, "flats", 3)) {
      /* Convert ground polygons */
      flags |= FLAGS_FLATS;
    } else if (is_switch(opt, "flip", 3)) {
      /* Flip backfacing ground polygons */
      flags |= FLAGS_FLIP_BACKFACING;
    } else if (is_switch(opt, "help", 2)) {
      /* Output usage information */
      (void)syntax_msg(stdout, argv[0]);
      return EXIT_SUCCESS;
    } else if (is_switch(opt, "human", 2)) {
      /* Enable human-readable material names */
      flags |= FLAGS_HUMAN_READABLE;
    } else if (is_switch(opt, "index", 1)) {
      /* Object number to convert was specified */
      long int objnum;
      if (!get_long_arg("index", &objnum, 0, MaxNumObjects - 1, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
      first = last = (int)objnum;
    } else if (is_switch(opt, "last", 2)) {
      /* Last object number to convert was specified */
      long int objnum;
      if (!get_long_arg("last", &objnum, 0, MaxNumObjects - 1, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
      last = (int)objnum;
    } else if (is_switch(opt, "list", 2)) {
      /* List contents of file */
      flags |= FLAGS_LIST;
    } else if (is_switch(opt, "mtllib", 1)) {
      /* Materials library file path was specified */
      if (++n >= argc || argv[n][0] == '-') {
        fputs("Missing materials library file name\n", stderr);
        return syntax_msg(stderr, argv[0]);
      }
      mtl_file = argv[n];
    } else if (is_switch(opt, "name", 2)) {
      /* Object name to convert was specified */
      if (++n >= argc || argv[n][0] == '-') {
         fputs("Missing object name\n", stderr);
         return syntax_msg(stderr, argv[0]);
      } else {
        name = argv[n];
      }
    } else if (is_switch(opt, "negative", 2)) {
      /* Enable negative vertex indices */
      flags |= FLAGS_NEGATIVE_INDICES;
    } else if (is_switch(opt, "outfile", 2)) {
      /* Output file path was specified */
      if (++n >= argc || argv[n][0] == '-') {
        fputs("Missing output file name\n", stderr);
        return syntax_msg(stderr, argv[0]);
      }
      output_file = argv[n];
    } else if (is_switch(opt, "offset", 2)) {
      /* Offset of the model data index was specified */
      if (!get_long_arg("offset", &index_offset, 0, LONG_MAX, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
    } else if (is_switch(opt, "strips", 1)) {
      /* Enable decomposition of complex polygons into triangle strips */
      flags |= FLAGS_TRIANGLE_STRIPS;
    } else if (is_switch(opt, "time", 1)) {
      /* Enable timing */
      time = true;
    } else if (is_switch(opt, "unused", 1)) {
      /* Enable output of unused vertices */
      flags |= FLAGS_UNUSED;
    } else if (is_switch(opt, "verbose", 1)) {
      /* Enable debugging output */
      flags |= FLAGS_VERBOSE;
    } else {
      fprintf(stderr, "Unrecognised switch '%s'\n", opt);
      return syntax_msg(stderr, argv[0]);
    }
  }

  if (index_offset < 0) {
    index_offset = (flags & FLAGS_FLATS) ? FlatIndexOffset : MeshIndexOffset;
  }

  if ((first > last) && (last >= 0)) {
    fputs("First object number must not exceed last object number\n", stderr);
    return EXIT_FAILURE;
  }
  if (first == -1) {
    first = 0;
  }

  if ((flags & FLAGS_TRIANGLE_STRIPS) && (flags & FLAGS_TRIANGLE_FANS)) {
    fputs("Cannot split polygons into both triangle fans and strips\n", stderr);
    return EXIT_FAILURE;
  }

  if (batch) {
    if (output_file != NULL) {
      fputs("Cannot specify an output file in batch processing mode\n",
            stderr);
      return syntax_msg(stderr, argv[0]);
    }
    if (n >= argc) {
      fputs("Must specify file(s) in batch processing mode\n", stderr);
      return syntax_msg(stderr, argv[0]);
    }
  } else {
    /* If an input file was specified, it should follow the switches */
    if (n < argc) {
      in_file = argv[n++];
    }

    /* An output file name may follow the input file name, but only if not
       already specified */
    if (n < argc) {
      if (output_file != NULL) {
        fputs("Cannot specify more than one output file\n", stderr);
        return syntax_msg(stderr, argv[0]);
      }
      output_file = argv[n++];
    }

    if ((flags & FLAGS_LIST) && (output_file != NULL)) {
      fputs("Cannot specify an output file in list mode\n", stderr);
      return EXIT_FAILURE;
    }

    /* Ensure that OBJ output isn't mixed up with other text on stdout */
    if ((output_file == NULL) && !(flags & FLAGS_LIST) &&
        (time || (flags & FLAGS_VERBOSE))) {
      fputs("Must specify an output file in verbose/timer mode\n", stderr);
      return EXIT_FAILURE;
    }

    if (n < argc) {
      fputs("Too many arguments (did you intend -batch?)\n", stderr);
      return syntax_msg(stderr, argv[0]);
    }
  }

  if (flags & FLAGS_VERBOSE) {
    printf("Apocalypse to Wavefront obj convertor, "VERSION_STRING"\n"
           "Copyright (C) 2020, Christopher Bazley\n");
  }

  if (batch) {
    /* In batch processing mode, the remaining arguments are treated as a
       list of file names (output to default file names) */
    for (; n < argc && rtn == EXIT_SUCCESS; n++) {
      /* Invent an output file name */
      assert(argv[n] != NULL);
      StringBuffer default_output;
      stringbuffer_init(&default_output);
      if (!stringbuffer_append(&default_output, argv[n], SIZE_MAX) ||
          !stringbuffer_append_separated(&default_output, EXT_SEPARATOR,
                                         "obj")) {
        fprintf(stderr, "Failed to allocate memory for output file path\n");
        rtn = EXIT_FAILURE;
      } else if (!process_file(argv[n],
                               stringbuffer_get_pointer(&default_output),
                               first, last, name, index_offset,
                               mtl_file, flags, time)) {
        rtn = EXIT_FAILURE;
      }
      stringbuffer_destroy(&default_output);
    }
  } else if (!process_file(in_file, output_file, first, last, name,
                    index_offset, mtl_file, flags, time)) {
    rtn = EXIT_FAILURE;
  }

  return rtn;
}
