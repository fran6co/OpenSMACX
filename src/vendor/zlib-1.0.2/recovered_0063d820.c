// ORIGINAL: 0x0063D820 BYTE_EXACT FILE
// name      _read_buf
// CORRECTED from sub_63d820
//   zlib 1.0.2 deflate.c, 113 bytes, byte-exact from upstream
// size      113 bytes
// spans     0x0063D820-0x0063D891
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00640A80
//
// NOT WRITTEN HERE. zlib 1.0.2 `_read_buf` from `deflate.c`,
// 113 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63d820' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/0063d820.cpp):
//
//   was: RULED-OUT: signature changed to (int*, void*, unsigned int) per disasm
//   was:            (3 cdecl stack params); mismatch is in the prologue (#2, push
//   was:            vs mov) - regalloc/param-order difference, not tried further.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
