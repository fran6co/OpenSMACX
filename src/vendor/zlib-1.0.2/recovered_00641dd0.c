// ORIGINAL: 0x00641DD0 BYTE_EXACT FILE
// name      _send_tree
// size      1397 bytes
// spans     0x00641DD0-0x00642345
// prototype 
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
//
// NOT WRITTEN HERE. This function is zlib 1.0.2, which the image
// names itself - see PROVENANCE.md beside this file. The recovery
// is the upstream `trees.c`, unmodified, and this wrapper exists
// only to carry the annotation so the source stays byte-identical
// to the release. Compiled as C under `/c /O2 /Gy /GR- /GX`;
// `send_tree` is `local`, so it is selected out of the object among
// the STATIC symbols that `/Gy` gives their own COMDAT.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`, compiled as C
//        under `/c /O2 /Gy /GR- /GX`. Nothing here was written to match
//        the disassembly; the disassembly was compiled from this.
//
#include "trees.c"
