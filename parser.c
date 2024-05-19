/*
 *  ApoctoObj - Converts Apocalypse graphics to Wavefront format
 *  Apocalypse object mesh parser
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
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>

/* StreamLib headers */
#include "Reader.h"

/* 3dObjLib headers */
#include "Vector.h"
#include "Coord.h"
#include "Vertex.h"
#include "Primitive.h"
#include "Group.h"
#include "Clip.h"
#include "ObjFile.h"

/* Local header files */
#include "flags.h"
#include "parser.h"
#include "version.h"
#include "names.h"
#include "colours.h"
#include "misc.h"

enum {
  MaxNumObjects = 200,
  MaxNumFlats = 26,
  MaxNumVertices = 256,
  MinNumSides = 3,
  MaxNumSides = 7,
  BytesPerPrimitive = 8,
  FlatColour = 0,
  NColours = 256,
  NTints = 1 << 2,
  LoadAddress = 0x8f00,
};

static void flip_backfacing(VertexArray * const varray,
                            Group * const group,
                            const unsigned int flags)
{
  assert(varray != NULL);
  assert(group != NULL);
  assert(flags & FLAGS_FLIP_BACKFACING);
  assert(!(flags & ~FLAGS_ALL));

  Coord norm[3] = {0,0,1};
  int const n = group_get_num_primitives(group);
  for (int p = 0; p < n; ++p) {
    Primitive *const pp = group_get_primitive(group, p);
    if (primitive_set_normal(pp, varray, &norm)) {
      if (flags & FLAGS_VERBOSE) {
        printf("Flipped ground polygon %d\n", p);
      }
    }
  }
}

static bool parse_flat(Reader * const r, const int object_count,
                       VertexArray * const varray,
                       const int nvertices,
                       Group * const group,
                       const unsigned int flags)
{
  assert(r != NULL);
  assert(!reader_ferror(r));
  assert(object_count >= 0);
  assert(varray != NULL);
  assert(nvertices > 0);
  assert(nvertices <= MaxNumVertices);
  assert(group != NULL);
  assert(!(flags & ~FLAGS_ALL));

  if (flags & FLAGS_VERBOSE) {
    const long int vertices_start = reader_ftell(r);
    printf("Found %d vertices at file position %ld (0x%lx)\n",
           nvertices, vertices_start, vertices_start);
  }

  Primitive *pp = NULL;

  if (!(flags & FLAGS_LIST)) {
    if (vertex_array_alloc_vertices(varray, nvertices) < nvertices) {
      fprintf(stderr, "Failed to allocate memory for %d vertices "
              "(object %d)\n", nvertices, object_count);
      return false;
    }

    pp = group_add_primitive(group);
    if (pp == NULL) {
      fprintf(stderr, "Failed to allocate primitive memory "
              "(object %d)\n", object_count);
      return false;
    }
    primitive_set_id(pp, group_get_num_primitives(group));
  }

  for (int v = 0; v < nvertices; ++v) {
    Coord pos[3] = {0.0, 0.0, 0.0};
    for (size_t dim = 0; dim < 2; ++dim) {
      int32_t coord;
      if (!reader_fread_int32(&coord, r)) {
        fprintf(stderr, "Failed to read vertex %d\n", v);
        return false;
      }
      pos[dim] = coord;
    } /* next dimension */

    if (flags & FLAGS_LIST) {
      continue;
    }

    if (vertex_array_add_vertex(varray, &pos) < 0) {
      fprintf(stderr, "Failed to allocate vertex memory "
              "(vertex %d of object %d)\n", v, object_count);
      return false;
    }

    if (flags & FLAGS_VERBOSE) {
      vertex_array_print_vertex(varray, v);
      puts("");
    }

    if (primitive_add_side(pp, v) < 0) {
      fprintf(stderr, "Failed to add side: too many sides? "
                      "(side %d of object %d)\n",
              v, object_count);
      return false;
    }
  } /* next side */

  if (flags & FLAGS_LIST) {
    return true;
  }

  if (flags & FLAGS_VERBOSE) {
    puts("Flat:");
    primitive_print(pp, varray);
    puts("");
  }

  return true;
}

static bool parse_vertices(Reader * const r, const int object_count,
                           VertexArray * const varray,
                           const int nvertices,
                           const unsigned int flags)
{
  assert(r != NULL);
  assert(!reader_ferror(r));
  assert(object_count >= 0);
  assert(varray != NULL);
  assert(nvertices > 0);
  assert(nvertices <= MaxNumVertices);
  assert(!(flags & ~FLAGS_ALL));

  const long int vertices_start = reader_ftell(r);

  if (flags & FLAGS_VERBOSE) {
    printf("Found %d vertices at file position %ld (0x%lx)\n",
           nvertices, vertices_start, vertices_start);
  }

  if (!(flags & FLAGS_LIST) &&
      (vertex_array_alloc_vertices(varray, nvertices) < nvertices)) {
    fprintf(stderr, "Failed to allocate memory for %d vertices "
            "(object %d)\n", nvertices, object_count);
    return false;
  }

  for (int v = 0; v < nvertices; ++v) {
    Coord pos[3] = {0.0, 0.0, 0.0};
    for (size_t dim = 0; dim < ARRAY_SIZE(pos); ++dim) {
      int32_t coord;
      if (!reader_fread_int32(&coord, r)) {
        fprintf(stderr, "Failed to read vertex %d\n", v);
        return false;
      }
      pos[dim] = coord;
    } /* next dimension */

    if (flags & FLAGS_LIST) {
      continue;
    }

    if (vertex_array_add_vertex(varray, &pos) < 0) {
      fprintf(stderr, "Failed to allocate vertex memory "
              "(vertex %d of object %d)\n", v, object_count);
      return false;
    }

    if (flags & FLAGS_VERBOSE) {
      vertex_array_print_vertex(varray, v);
      puts("");
    }
  } /* next vertex */

  return true;
}

static bool parse_primitives(Reader * const r, const int object_count,
                             VertexArray * const varray,
                             Group * const group,
                             const int nprimitives,
                             const unsigned int flags)
{
  assert(r != NULL);
  assert(object_count >= 0);
  assert(!reader_ferror(r));
  assert(group != NULL);
  assert(nprimitives > 0);
  assert(!(flags & ~FLAGS_ALL));

  if (flags & FLAGS_VERBOSE) {
    const long int primitives_start = reader_ftell(r);
    printf("Found %d primitives at file position %ld (0x%lx)\n",
           nprimitives, primitives_start, primitives_start);
  }

  for (int p = 0; p < nprimitives; ++p) {
    const long int primitive_start = reader_ftell(r);
    if (flags & FLAGS_VERBOSE) {
      printf("Found primitive %d at file position %ld (0x%lx)\n",
             p, primitive_start, primitive_start);
    }

    if (!(flags & FLAGS_LIST)) {
      Primitive * const pp = group_add_primitive(group);
      if (pp == NULL) {
        fprintf(stderr, "Failed to allocate primitive memory "
                "(primitive %d of object %d)\n", p, object_count);
        return false;
      }
      primitive_set_id(pp, group_get_num_primitives(group));

      const int nsides = reader_fgetc(r);
      if (nsides == EOF) {
        fprintf(stderr, "Failed to read no. of sides (primitive %d of object %d)\n",
                p, object_count);
        return false;
      }

      if (nsides < MinNumSides || nsides > MaxNumSides) {
        fprintf(stderr, "Bad side count %d (primitive %d of object %d)\n",
                nsides, p, object_count);
        return false;
      }

      const int nvertices = vertex_array_get_num_vertices(varray);

      for (int s = 0; s < nsides; ++s) {
        int const v = reader_fgetc(r);
        if (v == EOF) {
          fprintf(stderr, "Failed to read side %d of primitive %d "
                  "of object %d\n", s, p, object_count);
          return false;
        }

        /* Validate the vertex indices */
        if (v < 0 || v >= nvertices) {
          fprintf(stderr, "Bad vertex %lld (side %d of primitive %d "
                  "of object %d)\n", (long long signed)v, s, p,
                  object_count);
          return false;
        }

        if (primitive_add_side(pp, v) < 0) {
          fprintf(stderr, "Failed to add side: too many sides? "
                          "(side %d of primitive %d of object %d)\n",
                  s, p, object_count);
          return false;
        }
      }

      int const side = primitive_get_skew_side(pp, varray);
      if (side >= 0) {
        fprintf(stderr, "Warning: skew polygon detected "
                        "(side %d of primitive %d of object %d)\n",
                side, p, object_count);
      }

      if (flags & FLAGS_VERBOSE) {
        printf("Primitive %d:\n",
               group_get_num_primitives(group) - 1);
        primitive_print(pp, varray);
        puts("");
      }
    }

    /* Skip any unused vertex indices */
    if (reader_fseek(r, primitive_start + BytesPerPrimitive, SEEK_SET)) {
      fprintf(stderr, "Failed to seek end of primitive "
              "(primitive %d of object %d)\n", p, object_count);
      return false;
    }
  } /* next primitive */

  /* Read primitive colours */
  if (flags & FLAGS_VERBOSE) {
    const long int colours_start = reader_ftell(r);
    printf("Found %d colours at file position %ld (0x%lx)\n",
           nprimitives, colours_start, colours_start);
  }

  for (int p = 0; p < nprimitives; ++p) {
    const int colour = reader_fgetc(r);
    if (colour == EOF) {
      fprintf(stderr, "Failed to read colour (primitive %d of object %d)\n",
              p, object_count);
      return false;
    }

    if (flags & FLAGS_LIST) {
      continue;
    }
    primitive_set_colour(group_get_primitive(group, p), colour);
  }

  return true;
}

static void mark_vertices(VertexArray * const varray,
                          Group (* const group),
                          const int object_count,
                          const unsigned int flags)
{
  assert(group != NULL);
  assert(object_count >= 0);
  assert(!(flags & ~FLAGS_ALL));

  if (flags & FLAGS_UNUSED) {
    /* We're keeping all vertices */
    vertex_array_set_all_used(varray);
  } else {
    /* Mark only the used vertices */
    group_set_used(group, varray);

    /* Report the unused vertices */
    if (flags & FLAGS_VERBOSE) {
      int count = 0;
      const int nvertices = vertex_array_get_num_vertices(varray);
      for (int v = 0; v < nvertices; ++v) {
        if (!vertex_array_is_used(varray, v)) {
          Coord (*coords)[3] = vertex_array_get_coords(varray, v);
          printf("Vertex %d {%g,%g,%g} is unused (object %d)\n", v,
                 (*coords)[0], (*coords)[1], (*coords)[2], object_count);
          ++count;
        }
      }
      printf("Object %d has %d unused vertices\n", object_count, count);
    }
  }
}

static int get_false_colour(const Primitive *pp, void *arg)
{
  NOT_USED(pp);
  NOT_USED(arg);

  static int p;
  const int colour = (p * NTints) % NColours;
  ++p;
  return colour;
}

static int get_human_material(char *buf, size_t buf_size,
                              int const colour, void *arg)
{
  NOT_USED(arg);
  return snprintf(buf, buf_size, "%s_%d",
                  get_colour_name(colour / NTints), colour % NTints);
}

static int get_material(char *const buf, size_t const buf_size,
                        int const colour, void *arg)
{
  NOT_USED(arg);
  return snprintf(buf, buf_size, "riscos_%d", colour);
}

static bool process_object(Reader * const r, FILE * const out,
                           const char * const object_name,
                           const int object_count,
                           VertexArray * const varray,
                           Group * const group,
                           int *const vtotal, bool *const list_title,
                           const unsigned int flags)
{
  long int obj_start = 0;

  assert(r != NULL);
  assert(!reader_ferror(r));
  assert(object_name != NULL);
  assert(*object_name != '\0');
  assert(object_count >= 0);
  assert(varray != NULL);
  assert(group != NULL);
  assert(vtotal != NULL);
  assert(*vtotal >= 0);
  assert(!(flags & ~FLAGS_ALL));

  if (flags & FLAGS_LIST) {
    obj_start = reader_ftell(r);
  }

  vertex_array_clear(varray);
  group_delete_all(group);

  int32_t nvertices, nprimitives = 1;

  if (!reader_fread_int32(&nvertices, r)) {
    fprintf(stderr, "Failed to read number of vertices (object %d)\n",
            object_count);
    return false;
  }

  if ((nvertices < 1) || (nvertices > MaxNumVertices)) {
    fprintf(stderr, "Bad number of vertices, %lld (object %d)\n",
            (long long signed int)nvertices, object_count);
    return false;
  }

  if (flags & FLAGS_FLATS) {
    if (!parse_flat(r, object_count, varray, nvertices, group, flags)) {
      return false;
    }

    if (flags & FLAGS_FLIP_BACKFACING) {
      flip_backfacing(varray, group, flags);
    }
  } else {
    if (!parse_vertices(r, object_count, varray, nvertices, flags)) {
      return false;
    }

    if (!reader_fread_int32(&nprimitives, r)) {
      fprintf(stderr, "Failed to read number of primitives (object %d)\n",
              object_count);
      return false;
    }

    if (nprimitives < 1) {
      fprintf(stderr, "Bad number of primitives, %lld (object %d)\n",
              (long long signed int)nprimitives, object_count);
      return false;
    }

    if (!parse_primitives(r, object_count, varray, group,
                          nprimitives, flags)) {
      return false;
    }
  }

  if (out != NULL) {
    /* In cases of overlapping coplanar polygons,
       split the underlying polygon */
    if (flags & FLAGS_CLIP_POLYGONS) {
      const int group_order[] = {0};
      if (!clip_polygons(varray, group, group_order,
                         ARRAY_SIZE(group_order),
                         (flags & FLAGS_VERBOSE) != 0)) {
        fprintf(stderr,
                "Clipping of overlapping coplanar polygons failed\n");
        return false;
      }
    }

    /* Mark the vertices in preparation for culling unused ones. */
    mark_vertices(varray, group, object_count, flags);

    if (!(flags & FLAGS_DUPLICATE)) {
      /* Unmark duplicate vertices in preparation for culling them. */
      if (vertex_array_find_duplicates(varray,
                                       (flags & FLAGS_VERBOSE) != 0) < 0) {
        fprintf(stderr, "Detection of duplicate vertices failed\n");
        return false;
      }
    }

    int vobject;
    if (!(flags & FLAGS_UNUSED) || !(flags & FLAGS_DUPLICATE)) {
      /* Cull unused and/or duplicate vertices */
      vobject = vertex_array_renumber(varray, (flags & FLAGS_VERBOSE) != 0);
      DEBUGF("Renumbered %d vertices\n", vobject);
    } else {
      vobject = vertex_array_get_num_vertices(varray);
      DEBUGF("No need to renumber %d vertices\n", vobject);
    }

    if (fprintf(out, "\no %s\n", object_name) < 0) {
      fprintf(stderr,
              "Failed writing to output file: %s\n",
              strerror(errno));
      return false;
    }

    VertexStyle vstyle = VertexStyle_Positive;
    if (flags & FLAGS_NEGATIVE_INDICES) {
      vstyle = VertexStyle_Negative;
    }

    MeshStyle mstyle = MeshStyle_NoChange;
    if (flags & FLAGS_TRIANGLE_FANS) {
      mstyle = MeshStyle_TriangleFan;
    } else if (flags & FLAGS_TRIANGLE_STRIPS) {
      mstyle = MeshStyle_TriangleStrip;
    }

    if (!output_vertices(out, vobject, varray, -1) ||
        !output_primitives(out, object_name, *vtotal, vobject,
                           varray, group, 1,
                           (flags & FLAGS_FALSE_COLOUR) ?
                             get_false_colour : NULL,
                           (flags & FLAGS_HUMAN_READABLE) ?
                             get_human_material : get_material,
                           NULL, vstyle, mstyle)) {
      fprintf(stderr, "Failed writing to output file: %s\n",
              strerror(errno));
      return false;
    }

    *vtotal += vobject;
  }

  if (flags & FLAGS_LIST) {
    if (!*list_title) {
      puts("\nIndex  Name                  Verts  Prims      Offset        Size");
      *list_title = true;
    }

    const long int obj_size = reader_ftell(r) - obj_start;
    printf("%5d  %-20.20s  %5d  %5d  %10ld  %10ld\n",
           object_count, object_name, nvertices, nprimitives,
           obj_start, obj_size);
  }

  return true;
}

static bool read_index(Reader * const in, const int first, const int last,
  const long int index_offset, long int *const index, const unsigned int flags)
{
  assert(in != NULL);
  assert(!reader_ferror(in));
  assert(first >= 0);
  assert(last >= first);
  assert(index_offset >= 0);
  assert(index != NULL);
  assert(!(flags & ~FLAGS_ALL));

  int const err = reader_fseek(in, index_offset +
                               (sizeof(int32_t) * first), SEEK_SET);
  if (err) {
    fprintf(stderr, "Failed to seek objects index at "
                    "file position %ld (0x%lx)\n",
            index_offset, index_offset);
    return NULL;
  }

  bool success = true;
  const long int index_addr = LoadAddress + index_offset;

  for (int object_count = first; object_count <= last; ++object_count) {
    int32_t address;
    if (!reader_fread_int32(&address, in)) {
      fprintf(stderr, "Failed to read address from input file (object %d)\n",
              object_count);
      success = false;
      break;
    }

    if (address < index_addr) {
      fprintf(stderr, "Bad address %" PRId32 " (0x%" PRIx32 ") "
              "for object %d in index\n", address, address, object_count);
      success = false;
      break;
    }

    long int const fpos = index_offset + (address - index_addr);
    if (flags & FLAGS_VERBOSE) {
      printf("Object %d has address 0x%" PRIx32 ", "
             "file position %ld (0x%lx)\n",
             object_count, address, fpos, fpos);
    }
    index[object_count] = fpos;
  }
  return success;
}

static bool process_objects(Reader * const in, FILE * const out,
        int const first, int const last, const char * const name,
        const long int *const index, const unsigned int flags)
{
  assert(in != NULL);
  assert(!reader_ferror(in));
  assert(index != NULL);
  assert(first >= 0);
  assert(last >= first);
  assert(!(flags & ~FLAGS_ALL));

  Group group;
  group_init(&group);

  VertexArray varray;
  vertex_array_init(&varray);

  bool success = true;
  int vtotal = 0;
  bool list_title = false, stop = false;

  /* Read each file position from the index in turn. */
  for (int object_count = first;
       success && !stop && (object_count <= last);
       ++object_count) {

    /* Is this object the named one? */
    const char *object_name;
    if (flags & FLAGS_FLATS) {
      object_name = get_flat_name(object_count);
    } else {
      object_name = get_obj_name(object_count);
    }

    if (name != NULL) {
      if (!strcmp(name, object_name)) {
        /* Stop after finding the named object (assuming there are
           no others of the same name) */
        stop = true;
      } else {
        continue;
      }
    }

    long int const file_pos = index[object_count];
    int err = reader_fseek(in, file_pos, SEEK_SET);
    if (!err) {
      /* fseek doesn't return an error when seeking beyond the end
         of a file. */
      const int c = reader_fgetc(in);
      if (c == EOF) {
        err = 1;
      } else {
        if (reader_ungetc(c, in) == EOF) {
          fprintf(stderr, "Failed to push back first byte of object %d\n",
                  object_count);
          success = false;
          break;
        }
      }
    }

    if (err) {
      fprintf(stderr, "Failed to seek object %d at "
                      "file position %ld (0x%lx)\n",
              object_count, file_pos, file_pos);
      success = false;
      break;
    }

    if (flags & FLAGS_VERBOSE) {
      printf("Found object %d at file position %ld (0x%lx)\n",
             object_count, file_pos, file_pos);
    }

    success = process_object(in, out, object_name, object_count,
                             &varray, &group, &vtotal, &list_title,
                             flags);
  }

  group_free(&group);
  vertex_array_free(&varray);

  return success;
}

bool apoc_to_obj(Reader * const in, FILE * const out,
                 int first, int last, const char * const name,
                 const long int index_offset, const char * const mtl_file,
                 const unsigned int flags)
{
  assert(in != NULL);
  assert(!reader_ferror(in));
  assert(first >= 0);
  assert(index_offset >= 0);
  assert(last == -1 || last >= first);
  assert(mtl_file != NULL);
  assert(!(flags & ~FLAGS_ALL));

  if (out != NULL &&
      fprintf(out, "# Apocalypse graphics\n"
                   "# Converted by ApoctoObj "VERSION_STRING"\n"
                   "mtllib %s\n", mtl_file) < 0) {
    fprintf(stderr, "Failed writing to output file: %s\n",
            strerror(errno));
    return false;
  }

  if (first == -1) {
    first = 0;
  }

  int const max = (flags & FLAGS_FLATS) ? MaxNumFlats : MaxNumObjects;
  if (last == -1 || last >= max) {
    last = max - 1;
  }

  bool success = false;
  long int index[MaxNumObjects > MaxNumFlats ? MaxNumObjects : MaxNumFlats];

  if (read_index(in, first, last, index_offset, index, flags)) {
    success = process_objects(in, out, first, last, name, index, flags);
  }

  return success;
}
