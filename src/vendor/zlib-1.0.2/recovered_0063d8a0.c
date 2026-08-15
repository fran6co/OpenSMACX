// ORIGINAL: 0x0063D8A0 BYTE_EXACT FILE
// name      _deflate_fast
// CORRECTED from sub_63d8a0
//   zlib 1.0.2 deflate.c, 559 bytes, byte-exact from upstream
// size      559 bytes
// spans     0x0063D8A0-0x0063DACF
// prototype 
// callers   0   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D450 0x0063D6F0 0x0063DAD0 0x00641280 0x00642350
//
// NOT WRITTEN HERE. zlib 1.0.2 `_deflate_fast` from `deflate.c`,
// 559 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63d8a0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
