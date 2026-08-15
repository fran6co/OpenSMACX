// ORIGINAL: 0x00642A70 BYTE_EXACT FILE
// name      _copy_block
// CORRECTED from sub_642a70
//   zlib 1.0.2 trees.c, 151 bytes, byte-exact from upstream
// size      151 bytes
// spans     0x00642A70-0x00642B07
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006429F0
//
// NOT WRITTEN HERE. zlib 1.0.2 `_copy_block` from `trees.c`,
// 151 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_642a70' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
