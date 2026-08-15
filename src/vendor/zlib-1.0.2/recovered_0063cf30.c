// ORIGINAL: 0x0063CF30 BYTE_EXACT FILE
// name      _deflateInit2_
// CORRECTED from sub_63cf30
//   zlib 1.0.2 deflate.c, 492 bytes, byte-exact from upstream
// size      492 bytes
// spans     0x0063CF30-0x0063D11C
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D120 0x0063D4D0
// indirect  0x0063CFF9 0x0063D05A 0x0063D06A 0x0063D07A 0x0063D097
//
// NOT WRITTEN HERE. zlib 1.0.2 `_deflateInit2_` from `deflate.c`,
// 492 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63cf30' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/0063cf30.cpp):
//
//   was: RULED-OUT: (none - lands MISMATCH; a param-7/param-3 reuse near the end
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
