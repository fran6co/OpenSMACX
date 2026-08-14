// ORIGINAL: 0x0063DC40 BYTE_EXACT FILE
// name      sub_63dc40
// size      755 bytes
// spans     0x0063DC40-0x0063DF33
// prototype 
// callers   0   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D450 0x0063D6F0 0x0063DAD0 0x00641280 0x00642350
//
// NOT WRITTEN HERE. zlib 1.0.2 `_deflate_slow` from `deflate.c`,
// 755 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63dc40' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
