// ORIGINAL: 0x0063D560 BYTE_EXACT FILE
// name      sub_63d560
// size      147 bytes
// spans     0x0063D560-0x0063D5F3
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// NOT WRITTEN HERE. zlib 1.0.2 `_lm_init` from `deflate.c`,
// 147 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63d560' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
