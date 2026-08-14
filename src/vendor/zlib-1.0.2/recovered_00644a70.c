// ORIGINAL: 0x00644A70 BYTE_EXACT FILE
// name      _inflate_fast
// size      834 bytes
// spans     0x00644A70-0x00644DB2
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// NOT WRITTEN HERE. This function is zlib 1.0.2, which the image
// names itself - see PROVENANCE.md beside this file. The recovery
// is the upstream `inffast.c`, unmodified, and this wrapper exists
// only to carry the annotation so the source stays byte-identical
// to the release. Compiled as C under `/c /O2 /Gy /GR- /GX`;
// `inflate_fast` is `local`, so it is selected out of the object among
// the STATIC symbols that `/Gy` gives their own COMDAT.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `inffast.c`, compiled as C
//        under `/c /O2 /Gy /GR- /GX`. Nothing here was written to match
//        the disassembly; the disassembly was compiled from this.
//
#include "inffast.c"
