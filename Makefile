# Project:   ApocToObj

# Tools
CC = cc
Link = link
AS = objasm

# Toolflags:
CCCommonFlags = -c -depend !Depend -IC: -throwback -fahi -apcs 3/32/fpe2/swst/fp/nofpr -memaccess -L22-S22-L41
CCflags = $(CCCommonFlags) -DNDEBUG -Otime
CCDebugFlags = $(CCCommonFlags) -g -DUSE_CBDEBUG -DFORTIFY -DDEBUG_OUTPUT
Linkflags = -aif -c++
LinkDebugFlags = $(Linkflags) -d

include MakeCommon

DebugObjectsApoc = $(addprefix debug.,$(ObjectList))
ReleaseObjectsApoc = $(addprefix o.,$(ObjectList))
DebugLibs = C:o.Stubs C:o.Fortify C:o.CBDebugLib C:debug.CBUtilLib \
            C:debug.StreamLib C:debug.3dObjLib
ReleaseLibs = C:o.StubsG C:o.CBUtilLib C:o.StreamLib C:o.3dObjLib

# Final targets:
all: ApocToObj ApocToObjD 

ApocToObj: $(ReleaseObjectsApoc)
	$(Link) $(LinkFlags) -o $@ $(ReleaseObjectsApoc) $(ReleaseLibs)

ApocToObjD: $(DebugObjectsApoc)
	$(Link) $(LinkDebugFlags) -o $@ $(DebugObjectsApoc) $(DebugLibs)

# User-editable dependencies:
.SUFFIXES: .o .c .debug
.c.o:; cc $(CCflags) -o $@ $<
.c.debug:; cc $(CCDebugFlags) -o $@ $<

# Static dependencies:

# Dynamic dependencies:
debug.findnorm:	c.findnorm
debug.findnorm:	C:h.Vertex
debug.findnorm:	C:h.Vector
debug.findnorm:	C:h.Coord
debug.findnorm:	C:h.Coord
debug.findnorm:	C:h.Primitive
debug.findnorm:	C:h.Vertex
debug.findnorm:	C:h.Vector
debug.findnorm:	C:h.Coord
debug.findnorm:	C:h.Group
debug.findnorm:	C:h.Primitive
debug.findnorm:	C:h.Vertex
debug.findnorm:	h.findnorm
debug.findnorm:	C:h.Vertex
debug.findnorm:	C:h.Coord
debug.findnorm:	C:h.Group
debug.findnorm:	h.misc
debug.findnorm:	C:h.Fortify
debug.findnorm:	C:h.ufortify
debug.findnorm:	C:h.Debug
debug.findnorm:	C:h.PseudoIO
debug.colours:	c.colours
debug.colours:	h.colours
debug.colours:	h.misc
debug.colours:	C:h.Fortify
debug.colours:	C:h.ufortify
debug.colours:	C:h.Debug
debug.colours:	C:h.PseudoIO
debug.apoctoobj:	c.apoctoobj
debug.apoctoobj:	C:h.StrExtra
debug.apoctoobj:	C:h.ArgUtils
debug.apoctoobj:	C:h.Reader
debug.apoctoobj:	C:h.ReaderGKey
debug.apoctoobj:	C:h.Reader
debug.apoctoobj:	C:h.ReaderRaw
debug.apoctoobj:	C:h.Reader
debug.apoctoobj:	h.flags
debug.apoctoobj:	h.parser
debug.apoctoobj:	C:h.Reader
debug.apoctoobj:	h.version
debug.apoctoobj:	h.misc
debug.apoctoobj:	C:h.Fortify
debug.apoctoobj:	C:h.ufortify
debug.apoctoobj:	C:h.Debug
debug.apoctoobj:	C:h.PseudoIO
debug.parser:	c.parser
debug.parser:	C:h.Reader
debug.parser:	C:h.Vector
debug.parser:	C:h.Coord
debug.parser:	C:h.Coord
debug.parser:	C:h.Vertex
debug.parser:	C:h.Vector
debug.parser:	C:h.Coord
debug.parser:	C:h.Primitive
debug.parser:	C:h.Vertex
debug.parser:	C:h.Group
debug.parser:	C:h.Primitive
debug.parser:	C:h.Vertex
debug.parser:	C:h.Clip
debug.parser:	C:h.Vertex
debug.parser:	C:h.Group
debug.parser:	C:h.ObjFile
debug.parser:	C:h.Primitive
debug.parser:	C:h.Vertex
debug.parser:	C:h.Group
debug.parser:	h.flags
debug.parser:	h.parser
debug.parser:	C:h.Reader
debug.parser:	h.version
debug.parser:	h.names
debug.parser:	h.findnorm
debug.parser:	C:h.Vertex
debug.parser:	C:h.Coord
debug.parser:	C:h.Group
debug.parser:	h.colours
debug.parser:	h.misc
debug.parser:	C:h.Fortify
debug.parser:	C:h.ufortify
debug.parser:	C:h.Debug
debug.parser:	C:h.PseudoIO
debug.names:	c.names
debug.names:	h.names
debug.names:	h.misc
debug.names:	C:h.Fortify
debug.names:	C:h.ufortify
debug.names:	C:h.Debug
debug.names:	C:h.PseudoIO
