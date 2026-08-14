// ORIGINAL: 0x00644180 BYTE_EXACT FILE
// name      _inflate_codes
// size      1896 bytes
// spans     0x00644180-0x006448E8
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644930 0x00644A70
//
// NOT WRITTEN HERE. This function is zlib 1.0.2, which the image
// names itself - see PROVENANCE.md beside this file. The recovery
// is the upstream `infcodes.c`, unmodified, and this wrapper exists
// only to carry the annotation so the source stays byte-identical
// to the release. Compiled as C under `/c /O2 /Gy /GR- /GX`;
// `inflate_codes` is `local`, so it is selected out of the object among
// the STATIC symbols that `/Gy` gives their own COMDAT.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `infcodes.c`, compiled as C
//        under `/c /O2 /Gy /GR- /GX`. Nothing here was written to match
//        the disassembly; the disassembly was compiled from this.
//
#include "infcodes.c"
