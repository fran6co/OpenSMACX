// ORIGINAL: 0x006416F0 BYTE_EXACT FILE
// name      sub_6416f0
// size      209 bytes
// spans     0x006416F0-0x006417C1
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// NOT WRITTEN HERE. zlib 1.0.2 `_pqdownheap` from `trees.c`,
// 209 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_6416f0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
