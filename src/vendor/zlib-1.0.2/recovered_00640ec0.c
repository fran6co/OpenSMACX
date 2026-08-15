// ORIGINAL: 0x00640EC0 BYTE_EXACT FILE
// name      _gen_codes
// CORRECTED from sub_640ec0
//   zlib 1.0.2 trees.c, 115 bytes, byte-exact from upstream
// size      115 bytes
// spans     0x00640EC0-0x00640F33
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00642940
//
// NOT WRITTEN HERE. zlib 1.0.2 `_gen_codes` from `trees.c`,
// 115 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_640ec0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
