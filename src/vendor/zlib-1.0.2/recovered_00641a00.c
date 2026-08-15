// ORIGINAL: 0x00641A00 BYTE_EXACT FILE
// name      _build_bl_tree
// CORRECTED from sub_641a00
//   zlib 1.0.2 trees.c, 111 bytes, byte-exact from upstream
// size      111 bytes
// spans     0x00641A00-0x00641A6F
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006414B0 0x00641A70
//
// NOT WRITTEN HERE. zlib 1.0.2 `_build_bl_tree` from `trees.c`,
// 111 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_641a00' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
