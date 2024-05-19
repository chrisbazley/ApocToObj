# ApocToObj
Apocalypse to Wavefront convertor

(C) Christopher Bazley, 2020

Version 0.03 (19 May 2024)

-----------------------------------------------------------------------------
 1   Introduction and Purpose
-----------------------------

  'Apocalypse' is a single-player 3D shoot 'em up for Acorn Archimedes
computers, written by Gordon Key. It was published in 1990 by The Fourth
Dimension. The game world is drawn using a combination of flat-shaded
polygon meshes and sprites.

  This command-line program can be used to convert object models belonging to
'Apocalypse' from their original format into the simple object geometry
format developed by Wavefront for their Advanced Visualizer software.
Wavefront OBJ format is a de-facto standard for 3D computer graphics and it
has the advantage of being human-readable.

  The colour and other visual properties of objects are defined separately
from the object geometry (OBJ) file in a companion file known as a material
library (MTL) file. The supplied MTL file defines all of the colours in the
default RISC OS 256-colour palette as named materials, assuming a constant
colour illumination model (no texture map, ambient or specular reflectance).

-----------------------------------------------------------------------------
2   Requirements
----------------

  The supplied executable files will only work on RISC OS machines. They have
not been tested on any version of RISC OS earlier than 4, although they
should also work on earlier versions provided that a suitable version of the
'SharedCLibrary' module is active. They should be compatible with 32 bit
versions of RISC OS.

  You will obviously require a copy of the game 'Apocalypse' from which to
rip the graphics.

  The object model data for 'Apocalypse' can be found inside the !Apocalyps
application, in an untyped file named APCOD which also contains the
executable code for the game.
```
!Apocalyps
   `--APCOD
```
-----------------------------------------------------------------------------
3   Quick Guide
---------------

  The !Apocalyps application doesn't set a system variable to record its
location when it is first 'seen' by the Filer. Unfortunately that means
there is no convenient shorthand for referencing files inside the
application.

  Hold down the Shift key and double-click on the !Apocalyps application to
open it. Copy the APCOD file to the directory containing the 'ApocToObj'
  program.

  Set the current selected directory to that containing the 'ApocToObj'
program. On RISC OS 6, that can be done by opening the relevant directory
display and choosing 'Set work directory' from the menu (or giving the
directory display the input focus and then pressing F11).

  Press Ctrl-F12 to open a task window and then invoke the conversion program
using the following command (for example):
```
  *ApocToObj -list APCOD
```
  This should list all of the objects addressed by the index at the given
offset in file 'APCOD'. One of those should be named 'saucer_2', which is a
3D model of the Rakonan flying saucers which attack the player.

  To convert the flying saucer model into a Wavefront OBJ file named
'saucer/obj', use the following command:
```
  *ApocToObj -name saucer_2 -human APCOD saucer/obj
```
  The '-name' argument selects which object is converted and the '-human'
argument enables generation of human-readable material names such as
'maroon_0' in the output file.

-----------------------------------------------------------------------------
4   Usage information
---------------------

4.1 Command line syntax
-----------------------
  The basic syntax of the program is summarised below. It has two principle
modes of operation: single file mode and batch processing. All switches are
optional, but some are incompatible with each other (e.g. '-batch' with
'-outfile'). Switch names may be abbreviated provided that doing so does not
make the command ambiguous.
```
usage: ApocToObj [switches] [<input-file> [<output-file>]]
or     ApocToObj -batch [switches] <file1> [<file2> .. <fileN>]
```

4.2 Input and output
--------------------
Switches:
```
  -batch              Process a batch of files (see above)
  -flats              Convert or list flats instead of object models
  -offset N           Byte offset to object data address table in input
                      (default 0x10c6c)
  -outfile <file>     Write output to the named file instead of stdout
```
  The expected input is a file containing the executable code for the game.
By default, only object models are read from the input file. If the switch
'-flats' is used then flats are read instead. Flats are a separate array of
polygons that have no colour and are defined in only two dimensions.

  Flats and object models are indexed using separate tables of four-byte
little-endian addresses. When invoking ApocToObj, you may specify the offset
of one of those address tables within the input file. If no value is
specified then an appropriate default offset (dependant on the presence of
'-flats') is used.

  It's possible for several object numbers to alias the same flat or model
and for consecutively-numbered objects to be stored at arbitrary positions
within the file. Consequently reading more than one object from the same
file may fail if the input stream does not allow seeking backwards.

  Single file mode is the default mode of operation. Unlike batch mode, the
input and output files can be specified separately. An output file name can
be specified after the input file name, or before it using the '-outfile'
parameter.

  An output file name can be specified after the input file name or, by means
of the '-outfile' parameter, before it.

  If no input file is specified then input is read from 'stdin' (the
standard input stream; keyboard unless redirected). If no output file is
specified then OBJ-format output is written to 'stdout' (the standard output
stream; screen unless redirected).

  All of the following examples read input from a file named 'APCOD' and
write output to a file named 'apoc/obj':
```
  *ApocToObj APCOD apoc/obj
  *ApocToObj -outfile apoc/obj APCOD
  *ApocToObj -outfile apoc/obj <APCOD
  *ApocToObj APCOD >apoc/obj
```

  Under UNIX-like operating systems, output can be piped directly into
another program.

  Search for a named object in a file named 'APCOD':
```
  ApocToObj APCOD | grep -i 'saucer_2'
```

  Batch mode is enabled by the switch '-batch'. In this mode, multiple
files can be processed using a single command. The output is always saved to
a file with a name derived from the input file's name, which means that
programs cannot be chained using pipes. At least one file name must be
specified.

  Convert files named 'foo', 'bar' and 'baz' to Wavefront OBJ files named
'foo/obj', 'bar/obj' and 'baz/obj':
```
  *ApocToObj -batch foo bar baz
```

4.4 Object selection
--------------------
Switches:
```
  -index N     Object number to convert or list (default is all)
  -first N     First object number to convert or list
  -last N      Last object number to convert or list
  -name <name> Object name to convert or list (default is all)
```

  The object data address table can be filtered using the '-index' or
'-first' and '-last' parameters to select a single object or range of objects
to be processed. Using the '-index' parameter is equivalent to setting the
first and last numbers to the same value. The lowest model number is 0 and
the highest is 199; the lowest flat number is 0 and the highest is 25.

  The address table can also be filtered using the '-name' parameter to
select a single object to be processed. Any filter specified applies when
listing object definitions as well as when converting them.

  If no range of object numbers and no name is specified then all entries in
the address table are used.

  Convert the first object in file 'APCOD', outputting to the screen:
```
  *ApocToObj -index 0 APCOD
```

  If an object name is specified then the single object of the given name
is processed (provided that it falls within the specified range of object
numbers, if any).

  Convert a ground scanner object in file 'APCOD' (see below for object
naming):
```
  *ApocToObj -name ground_scanner APCOD
```

  Animated objects have a separate model for each frame of animation.
Objects which are animation frames are named according to the template
<name>_f<n>, where <n> is a decimal frame number. The 'f' part of the suffix
is to distinguish them from objects that exist in different versions (e.g.
there are three varieties of Simtaal Quak, none animated).

  Flying saucers are instead drawn using coordinates calculated at run time.
The model at index 22 in the address table is used for that purpose, which is
why it appears degenerate (all vertex coordinates are zero). Luckily a
complete saucer model can be found at index 31.

Named objects:

|  Number   | Name
|-----------|-----------------------
|  1        | guard_tower_1
|  4..5     | guard_tower_top_f0..1
|  6        | guard_tower_2
|  7        | obelisk_1_f1
|  14       | processing_factory
|  15       | obelisk_1_f0
|  22       | saucer_1
|  23       | simtaal_quak_1
|  24       | simtaal_quak_2
|  25       | simtaal_quak_3
|  26       | snail_rider
|  31       | saucer_2
|  35       | rakonan_barftub_f1
|  36       | rakonan_barftub_f0
|  40..42   | ground_wasp_f0..2
|  44..46   | tilexu_floater_f0..2
|  55       | silicon_vat
|  56       | static_release
|  57       | ground_scanner
|  58..61   | pumping_station_f0..3
|  62..65   | seismic_hammer_f0..3
|  66..69   | krypton_breather_f0..3
|  70       | ground_transport_1
|  71       | ground_transport_2
|  72..74   | wind_generator_1_f0..2
|  75..78   | wind_generator_2_f0..3
|  80..87   | rakonan_gomjabba_f0..7
|  88..95   | proton_flapper_f0..7
|  96..103  | electron_grinder_f0..7
|  104..111 | wave_generator_f0..7
|  112..119 | postal_teleport_f0..7
|  120..127 | climatic_ticker_f0..7
|  128..135 | weirding_flasher_f0..7
|  136..143 | aldebran_linkbat_f0..7
|  144..151 | argon_storehouse_f0..7
|  152..159 | thermal_riser_f0..7
|  166..167 | obelisk_2_f0..1
|  181..187 | snailherd_f0..6
|  192..199 | lhaktal_gourd_f0..7

4.5 Listing and summarizing objects
    -------------------------------
Switches:
```
  -list     List objects instead of converting them
```
  If the switch '-list' is used then ApocToObj lists object definitions
instead of converting them to Wavefront OBJ format. Only object definitions
matching any filter specified using the '-index', '-first', '-last', and
'-name' parameters are listed.

  No output file can be specified because no OBJ-format output is generated
and the object list is always sent to the standard output stream.

  List all object definitions indexed by file 'APCOD':
```
  *ApocToObj -list APCOD
```
  Output is a table with the following format:
```
Index  Name                  Verts  Prims      Offset        Size
    0  apocalypse_0             26     13       70548         437
```

4.6 Materials
-------------
Switches:
```
  -mtllib name   Specify a material library file (default sf3k.mtl)
  -human         Output readable material names
  -false         Assign false colours for visualization
```

  By default, ApocToObj emits 'usemtl' commands that refer to colours in
the standard RISC OS 256-colour palette by derived material names such as
'riscos_31'. (This naming convention is for compatibility with a similar
program, SF3KtoObj.)

  Convert the flying saucer, outputting colour numbers to the screen:
```
  *ApocToObj -index 31 APCOD
```

```
# 18 primitives
g saucer_2 saucer_2_0
usemtl riscos_119
f 7 13 8
usemtl riscos_115
f 8 13 9
...
```
  If the switch '-human' is used then ApocToObj instead generates human-
readable material names such as 'black_0'.

  Convert the flying saucer, outputting readable material names to the
screen:
```
  *ApocToObj -index 31 -human APCOD
```

```
# 18 primitives
g saucer_2 saucer_2_0
usemtl peridot_3
f 7 13 8
usemtl applegreen_3
f 8 13 9
...
```
  By default, ApocToObj emits a 'mtllib' command which references 'sf3k.mtl'
as the material library file to be used when drawing objects; this is the
same as the name of the supplied MTL file.

  An alternative material library file can be specified using the switch
'-mtllib'. The named file is not created, read or written by ApocToObj.

  False colours can be assigned to help visualise boundaries between
polygons, especially between coplanar polygons of the same colour. This
mode, which is mainly useful for debugging, is enabled by specifying the
switch '-false'.

  When false colours are enabled, the physical colours in the input file are
ignored.

4.7 Clipping
------------
Switches:
```
  -clip      Clip overlapping coplanar polygons
```
  Some objects are liable to suffer from a phenomenon known as "Z-fighting"
if they are part of a scene rendered using a depth (Z) buffer. It is caused
by overlapping faces in the same geometric plane and typically manifests as
a shimmering pattern in a rendered image. Essentially two or more polygons
occupy the same points in 3D space.

  The game uses painter's algorithm to ensure that overlapping objects are
drawn correctly (drawing more distant objects before nearer ones), instead
of using a depth buffer. It also draws the polygons of each object in a
predictable order, which allows overlapping polygons to be used as decals
(e.g. as doors and windows on buildings).

  If the switch '-clip' is used then the rearmost of two overlapping polygons
is split by dividing it along the line obtained by extending one edge of the
front polygon to infinity in both directions. This process is repeated until
no two edges intersect. Any polygons that are fully hidden (typically behind
decals) are deleted.

  The following diagrams illustrate how one polygon (B: 1 2 3 4) overlapped
by another (A: 5 6 7 8) is split into five polygons (B..F) during the
clipping process. The last polygon (F) is then deleted because it duplicates
the overlapping polygon (A).
```
     Original         First split       Second split
  (A overlaps B)     (A overlaps C)    (A overlaps D)
                               :
  3_____________2   3__________9__2   3__________9__2
  |      B      |   |          |  |   |      C   |  |
  |  7_______6  |   |          |  | ..|__________6..|..
  |  |   A   |  |   |      C   |B | 11|          |B |
  |  |_______|  |   |          |  |   |      D   |  |
  |  8       5  |   |          |  |   |          |  |
  |_____________|   |__________|__|   |__________|__|
  4             1   4          10 1   4          10 1
                               :
    Third split       Fourth split     Final clipped
  (A overlaps E)     (A overlaps F)     (F deleted)
  3__:_______9__2   3__________9__2   3__________9__2
  |  :   C   |  |   |      C   |  |   |      C   |  |
11|__7_______6  | 11|__7_______6  | 11|__7_______6  |
  |  |       |B |   |D |   F   |B |   |D |   A   |B |
  |D |   E   |  | ..|..|_______|..|.. |  |_______|  |
  |  |       |  |   |  8   E   5  |   |  8   E   5  |
  |__|_______|__|   |__|_______|__|   |__|_______|__|
  4  12      10 1   4  12      10 1   4  12      10 1
     :
```
4.8 Normal correction
---------------------
Switches:
```
  -flip      Flip back-facing flats
```
  The order in which vertices are specified in a primitive definition
determines the direction of a polygon's normal vector and therefore whether
or not it is facing the camera in any given scene.

  The vertices for flat_11 are specified in anti-clockwise order (when
viewed from the positive Z direction), therefore this flat is the only one
that faces towards the camera. The switch '-flip' reverses the order of
vertices for all the other flats so that they face the camera too.

4.10 Output of faces
--------------------
Switches:
```
  -fans      Split complex polygons into triangle fans
  -strips    Split complex polygons into triangle strips
  -negative  Use negative vertex indices
```
  The Wavefront OBJ format specification does not restrict the maximum
number of vertices in a face element. Nevertheless, some programs cannot
correctly display faces with more than three vertices. Two switches, '-fans'
and '-strips', are provided to split complex polygons into triangles.

```
       Original          Triangle fans       Triangle strips
     3_________2          3_________2          3_________2
     /         \          / `-._    \          /\`-._    \
    /           \        /      `-._ \        /  `\  `-._ \
 4 /             \ 1  4 /___________`-\ 1  4 /     \.    `-\ 1
   \             /      \         _.-`/      \`-._   \     /
    \           /        \    _.-`   /        \   `-. \.  /
     \_________/          \.-`______/          \_____`-.\/
     5         6          5         6          5         6

 f 1 2 3 4 5 6        f 1 2 3              f 1 2 3
                      f 1 3 4              f 6 1 3
                      f 1 4 5              f 6 3 4
                      f 1 5 6              f 5 6 4
```
  Vertices in a face element are normally indexed by their position in the
output file, counting upwards from 1. If the output comprises more than one
object definition then it can be more useful to count backwards from the
most recent vertex definition, which is assigned index -1. The '-negative'
switch enables this output mode, which allows object definitions to be
separated, extracted or rearranged later.

  Convert the flying saucer with positive vertex indices:
```
  *ApocToObj -index 31 APCOD
```

```
# 18 primitives
g saucer_2 saucer_2_0
usemtl riscos_119
f 7 13 8
usemtl riscos_115
f 8 13 9
...
```

  Convert the same object with negative vertex indices:
```
  *ApocToObj -index 31 -neg APCOD
```

```
# 18 primitives
g saucer_2 saucer_2_0
usemtl riscos_119
f -8 -2 -7
usemtl riscos_115
f -7 -2 -6
...
```

4.11 Hidden data
----------------
Switches:
```
  -unused     Include unused vertices in the output
  -duplicate  Include duplicate vertices in the output
```
  It's common for model data to include vertex definitions that are not
referenced by any primitive definition. An example is apocalypse_163. Such
vertices are not included in the output unless the '-unused' switch is used.

  Some object definitions also include two or more vertices with the same
coordinates, where both vertices are referenced by primitives. An example is
saucer_1. Duplicate vertices are automatically merged unless the '-duplicate'
switch is specified.

4.12 Getting diagnostic information
-----------------------------------
Switches:
```
  -time               Show the total time for each file processed
  -verbose or -debug  Emit debug information (and keep bad output)
```
  If either of the switches '-verbose' and '-debug' is used then the program
emits information about its internal operation on the standard output
stream. However, this makes it slower and prevents output being piped to
another program.

  If the switch '-time' is used then the total time for each file processed
(to centisecond precision) is printed. This can be used independently of
'-verbose' and '-debug'.

  When debugging output or the timer is enabled, you must specify an output
file name. This is to prevent OBJ-format output being sent to the standard
output stream and becoming mixed up with the diagnostic information.

-----------------------------------------------------------------------------
5   Colour names
----------------

  The colour names were taken from a variety of online sources including
Wikipedia articles, W3C standards, the X Window System, and the results of
the web comic XKCD's colour naming survey.

| Index | Web RGB | Name                 | Notes                         |
|-------|---------|----------------------|-------------------------------|
|     0 | #000000 | black              * | Identical to HTML 'black'     |
|     4 | #440000 | darkmaroon           | By analogy with 'darkred'     |
|     8 | #000044 | darknavy             | By analogy with 'darkblue'    |
|    12 | #440044 | darkpurple           | By analogy with 'darkmagenta' |
|    16 | #880000 | maroon             * | Like HTML colour #800000      |
|    20 | #cc0000 | mediumred            | By analogy with 'mediumblue'  |
|    24 | #880044 | tyrianpurple         | Like Wikipedia colour #66023c |
|    28 | #cc0044 | crimson            * | Like X11 colour #dc143c       |
|    32 | #004400 | darkgreen          + | Like X11 colour #006400       |
|    36 | #444400 | darkolive            | Like XKCD colour #373e02      |
|    40 | #004444 | darkteal             | Like XKCD colour #014d4e      |
|    44 | #444444 | darkgrey           + | Darker than X11 'dimgray'     |
|    48 | #884400 | brown              * | Like Wikipedia colour #964b00 |
|    52 | #cc4400 | mahogany             | Like Wikipedia colour #c04000 |
|    56 | #884444 | cordovan             | Like Wikipedia colour #893f45 |
|    60 | #cc4444 | brickred             | Like Wikipedia colour #cb4154 |
|    64 | #008800 | green              * | Like HTML colour #008000      |
|    68 | #448800 | avocado              | Like Wikipedia colour #568203 |
|    72 | #008844 | pigmentgreen         | Like Wikipedia colour #00a550 |
|    76 | #448844 | ferngreen            | Like Wikipedia colour #4f7942 |
|    80 | #888800 | olive              * | Like HTML colour #808000      |
|    84 | #cc8800 | harvestgold          | Like Wikipedia colour #da9100 |
|    88 | #888844 | darktan              | Like Wikipedia colour #918151 |
|    92 | #cc8844 | peru               * | Like X11 colour #cd853f       |
|    96 | #00cc00 | mediumgreen          | By analogy with 'mediumblue'  |
|   100 | #44cc00 | napiergreen          | Like Wikipedia colour #2a8000 |
|   104 | #00cc44 | darkpastelgreen      | Like Wikipedia colour #03c03c |
|   108 | #44cc44 | limegreen          * | Like X11 colour #32cd32       |
|   112 | #88cc00 | applegreen           | Like Wikipedia colour #8db600 |
|   116 | #cccc00 | peridot              | Like Wikipedia colour #e6e200 |
|   120 | #88cc44 | yellowgreen        * | Like X11 colour #9acd32       |
|   124 | #cccc44 | oldgold              | Like Wikipedia colour #cfb53b |
|   128 | #000088 | navy               * | Like HTML colour #000080      |
|   132 | #440088 | indigo             * | Like X11 colour #4b0082       |
|   136 | #0000cc | mediumblue         * | Like X11 colour #0000cd       |
|   140 | #4400cc | violetblue           | Like XKCD colour #510ac9      |
|   144 | #880088 | purple             * | Like HTML colour #800080      |
|   148 | #cc0088 | mediumvioletred    * | Like X11 colour #c71585       |
|   152 | #8800cc | darkviolet         * | Like X11 colour #9400d3       |
|   156 | #cc00cc | deepmagenta          | Like Wikipedia colour #cc00cc |
|   160 | #004488 | mediumelectricblue   | Like Wikipedia colour #035096 |
|   164 | #444488 | darkslateblue      * | Like X11 colour #483d8b       |
|   168 | #0044cc | royalazure           | Like Wikipedia colour #0038a8 |
|   172 | #4444cc | pigmentblue          | Like Wikipedia colour #333399 |
|   176 | #884488 | plum               + | Like Wikipedia colour #8e4585 |
|   180 | #cc4488 | mulberry             | Like Wikipedia colour #c54b8c |
|   184 | #8844cc | lavenderindigo       | Like Wikipedia colour #9457eb |
|   188 | #cc44cc | deepfuchsia          | Like Wikipedia colour #c154c1 |
|   192 | #008888 | teal               * | Like HTML colour #008080      |
|   196 | #448888 | dustyteal            | Like XKCD colour #4c9085      |
|   200 | #0088cc | honolulublue         | Like Wikipedia colour #007fbf |
|   204 | #4488cc | celestialblue        | Like Wikipedia colour #4997d0 |
|   208 | #888888 | grey               * | Like HTML colour #808080      |
|   212 | #cc8888 | oldrose              | Like Wikipedia colour #c08081 |
|   216 | #8888cc | ube                  | Like Wikipedia colour #8878c3 |
|   220 | #cc88cc | pastelviolet         | Like Wikipedia colour #cb99c9 |
|   224 | #00cc88 | caribbeangreen       | Like Wikipedia colour #00cc99 |
|   228 | #44cc88 | mint                 | Like Wikipedia colour #3eb479 |
|   232 | #00cccc | darkturquoise      * | Like X11 colour #00ced1       |
|   236 | #44cccc | mediumturquoise    * | Like X11 colour #48d1cc       |
|   240 | #88cc88 | darkseagreen       * | Like X11 colour #8fbc8f       |
|   244 | #cccc88 | lightbeige           | Like Ford colour #d2d08e      |
|   248 | #88cccc | pearlaqua            | Like Wikipedia colour #88d8c0 |
|   252 | #cccccc | lightgrey          * | Like X11 colour #d3d3d3       |

 '*' means that a colour has the same name as one of the standard X11 colours
  and closely resembles it.
  
 '+' means that a colour has the same name as one of the standard X11 colours
  but is significantly darker.

-----------------------------------------------------------------------------
6   File formats
----------------

6.1 Executable file
-------------------

  The game's assembled code and data is loaded at address &8F00. Flats are
accessed via a table of 26 addresses located at address &18B64. Object
models are accessed via a table of 200 addresses located at address &19B6C.
Each address occupies 4 bytes, stored in little-endian byte order (i.e.
least-significant bits first). This mechanism allows fast random access to
data by the game.

6.2  Model data
---------------
  In summary, the layout of the data for a single model is as follows:

|   Offset | Size | Data
|----------|------|--------------------------
|        0 |    4 | Number of vertices (v)
|        4 |  12v | Vertex definitions
|    4+12v |    4 | Number of primitives (p)
|    8+12v |   8p | Primitive definitions
| 8+12v+8p |    p | Primitive colours

6.2.1 Vertices
--------------
  An array of vertex definitions follows the vertex count. Each vertex
definition is 12 bytes long, comprising three coordinate values. Vertices
are not shared between models.

  The order in which vertices are defined is significant because their
position in the array is used to refer to them in primitive definitions. It
would be pointless to define more than 256 vertices in a single model because
each vertex index is encoded as one byte in a primitive definition.

  A right-handed coordinate system is used to specify vertex positions.
Ground level is the xy plane with z=0 and all positive z coordinate values
are above ground. Coordinates are stored as a signed two's complement little-
endian number occupying four bytes.

Vertex data format:

|  Offset | Size | Data
|---------|------|--------------
|       0 |    4 | X coordinate
|       4 |    4 | Y coordinate
|       8 |    4 | Z coordinate

6.2.2 Primitives
----------------
  An array of primitive definitions follows the primitive count. Each
primitive definition is 8 bytes long, most of which is used to store vertex
indices. Primitives are not shared between models.

  Each vertex index occupies one byte. Vertices are indexed by their
position in the preceding vertex array, starting at 0. A primitive must have
at least three vertices and can have up to seven (a heptagon). Polygons with
vertices specified in anti-clockwise direction face the camera.

Primitive data format:

|  Offset | Size | Data
|---------|------|-----------------
|       0 |    1 | Number of edges
|       1 |    1 | 1st vertex index
|       2 |    1 | 2nd vertex index
|       3 |    1 | 3rd vertex index
|       4 |    1 | 4th vertex index
|       5 |    1 | 5th vertex index
|       6 |    1 | 6th vertex index
|       7 |    1 | 7th vertex index

6.2.3 Primitive colours
-----------------------
  An array of colours numbers follows the primitive definitions. Each byte
specifies the colour of the primitive with the corresponding array index.

  Colours are encoded using an additive RGB model with 4 bits per component.
However, the 2 least-significant bits of each component (the 'tint' bits)
must be equal for all three components.

Bit  | 7      | 6      | 5      | 4      | 3      | 2      | 1      | 0
-----|--------|--------|--------|--------|--------|--------|--------|-------
Role | B high | G high | G low  | R high | B low  | R low  | T high | T low
```
Red component:   (R high << 3) + (R low << 2) + (T high << 1) + T low
Green component: (G high << 3) + (G low << 2) + (T high << 1) + T low
Blue component:  (B high << 3) + (B low << 2) + (T high << 1) + T low
```
('<< n' means 'left-shift by n binary places'.)

For example, 42 (binary 00101010):
```
  B high 0, G high 0, G low 1, R high 0, B low 1, R low 0, T high 1, T low 0
  Red:   0010 = 2/15 = 0.13
  Green: 0110 = 6/15 = 0.4
  Blue:  0110 = 6/15 = 0.4
```
This is similar to the SVG/HTML/CSS colour named 'Teal' (0.0, 0.50, 0.50).

6.3 Flat data
-------------
  In summary, the layout of the data for a single flat is as follows:

|  Offset | Size | Data
|---------|------|-----------------------
|       0 |    4 | Number of vertices (v)
|       4 |   8v | Vertex definitions

  A flat is drawn by connecting every pair of vertices in its definition
with an edge. There are no explicit primitive definitions for a flat.

6.3.1 Vertices
--------------
  An array of vertex definitions follows the vertex count. Each vertex
definition is 8 bytes long, comprising two coordinate values. Vertices
are not shared between flats.

  Coordinates are stored as a signed two's complement little-endian number
occupying four bytes.

Vertex data format:

|  Offset |  Size | Data
|---------|-------|--------------
|       0 |    4  | X coordinate
|       4 |    4  | Y coordinate

-----------------------------------------------------------------------------
7  Program history
------------------

0.01 (21 Apr 2020)
- First public release.

0.02 (22 Apr 2020)
- Converted flats now face in the positive Z direction (towards the camera).

0.03 (19 May 2024)
- Added a new makefile for use on Linux.
- Improved the README.md file for Linux users.

-----------------------------------------------------------------------------
8  Compiling the software
-------------------------

  Source code is only supplied for the command-line program. To compile
and link the code you will also require an ISO 9899:1999 standard 'C'
library and three of my own libraries: 3dObjLib, CBUtilLib and StreamLib.
These are available separately from https://github.com/chrisbazley/

  Three make files are supplied:

1. 'Makefile' is intended for use with GNU Make and the GNU C Compiler on Linux.

2. 'NMakefile' is intended for use with Acorn Make Utility (AMU) and the
   Norcroft C compiler supplied with the Acorn C/C++ Development Suite.

3. 'GMakefile' is intended for use with GNU Make and the GNU C Compiler on RISC OS.

  The APCS variant specified for the Norcroft compiler is 32 bit for
compatibility with ARMv5 and fpe2 for compatibility with older versions of
the floating point emulator. Generation of unaligned data loads/stores is
disabled for compatibility with ARMv6. When building the code for release,
it is linked with RISCOS Ltd's generic C library stubs ('StubsG').

  Before compiling the library for RISC OS, move the C source and header files
with .c and .h suffixes into subdirectories named 'c' and 'h' and remove those
suffixes from their names. You probably also need to create 'o', 'd' and 'debug'
subdirectories for compiler output.

  The only platform-specific code is the EXT_SEPARATOR and PATH_SEPARATOR
macro definitions in misc.h. These must be defined according to the file name
convention on the target platform (e.g. '.' and '\\' for DOS or Windows).

-----------------------------------------------------------------------------
9  Licence and Disclaimer
-------------------------

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public Licence as published by the Free
Software Foundation; either version 2 of the Licence, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public Licence for
more details.

  You should have received a copy of the GNU General Public Licence along
with this program; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.

-----------------------------------------------------------------------------
10  Credits
-----------

  ApocToObj was designed and programmed by Christopher Bazley.

  My information on the Wavefront Object file format came from Paul Bourke's
copy of the file format specifications for the Advanced Visualizer software
(Copyright 1995 Alias|Wavefront, Inc.):
  http://paulbourke.net/dataformats/obj/

  Some colour names were taken from a survey run by the web comic XKCD. The
data is in the public domain, available under a Creative Commons licence:
https://blog.xkcd.com/2010/05/03/color-survey-results/
Thanks to Keith McKillop for suggesting this source.

  The game Apocalypse is (C) 1990, The Fourth Dimension.

-----------------------------------------------------------------------------
11  Contact details
-------------------

  Feel free to contact me with any bug reports, suggestions or anything else.

  Email: mailto:cs99cjb@gmail.com

  WWW:   http://starfighter.acornarcade.com/mysite/
