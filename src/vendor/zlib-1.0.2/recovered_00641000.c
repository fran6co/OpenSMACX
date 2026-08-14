// ORIGINAL: 0x00641000 BYTE_EXACT FILE
// name      __tr_align
// size      627 bytes
// spans     0x00641000-0x00641273
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00642960
//
// NOT WRITTEN HERE. This function is zlib 1.0.2, which the image
// names itself - see PROVENANCE.md beside this file. The recovery
// is the upstream `trees.c`, unmodified, and this wrapper exists
// only to carry the annotation so the source stays byte-identical
// to the release. Compiled as C under `/c /O2 /Gy /GR- /GX`;
// `tr_align` is `local`, so it is selected out of the object among
// the STATIC symbols that `/Gy` gives their own COMDAT.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`, compiled as C
//        under `/c /O2 /Gy /GR- /GX`. Nothing here was written to match
//        the disassembly; the disassembly was compiled from this.
//
#include "trees.c"
