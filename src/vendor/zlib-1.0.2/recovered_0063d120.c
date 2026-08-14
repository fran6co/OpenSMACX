// ORIGINAL: 0x0063D120 BYTE_EXACT FILE
// name      sub_63d120
// size      115 bytes
// spans     0x0063D120-0x0063D193
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D560 0x00640BB0
//
// NOT WRITTEN HERE. zlib 1.0.2 `_deflateReset` from `deflate.c`,
// 115 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63d120' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
