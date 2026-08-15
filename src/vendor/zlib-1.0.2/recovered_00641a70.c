// ORIGINAL: 0x00641A70 BYTE_EXACT FILE
// name      _scan_tree
// CORRECTED from sub_641a70
//   zlib 1.0.2 trees.c, 231 bytes, byte-exact from upstream
// size      231 bytes
// spans     0x00641A70-0x00641B57
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// NOT WRITTEN HERE. zlib 1.0.2 `_scan_tree` from `trees.c`,
// 231 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_641a70' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
