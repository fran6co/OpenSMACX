// ORIGINAL: 0x00642C20 BYTE_EXACT FILE
// name      _inflate_blocks
// size      3368 bytes
// spans     0x00642C20-0x00643948
// prototype 
// callers   1   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00643990 0x00643EB0 0x00643FA0 0x00644100 0x00644140 0x00644180 0x00644910 0x00644930
// indirect  0x00642F98 0x006432E1
//
// NOT WRITTEN HERE. This function is zlib 1.0.2, which the image
// names itself - see PROVENANCE.md beside this file. The recovery
// is the upstream `infblock.c`, unmodified, and this wrapper exists
// only to carry the annotation so the source stays byte-identical
// to the release. Compiled as C under `/c /O2 /Gy /GR- /GX`;
// `inflate_blocks` is `local`, so it is selected out of the object among
// the STATIC symbols that `/Gy` gives their own COMDAT.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `infblock.c`, compiled as C
//        under `/c /O2 /Gy /GR- /GX`. Nothing here was written to match
//        the disassembly; the disassembly was compiled from this.
//
#include "infblock.c"
