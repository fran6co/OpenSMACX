// ORIGINAL: 0x00640F40 BYTE_EXACT FILE
// name      sub_640f40
// size      179 bytes
// spans     0x00640F40-0x00640FF3
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00642A70
//
// NOT WRITTEN HERE. zlib 1.0.2 `__tr_stored_block` from `trees.c`,
// 179 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_640f40' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/00640f40.cpp):
//
//   was: RULED-OUT: differently-named extern fn_00642a70(int,int,int,int) resolves fine
//   was:            (not a link failure); mismatch is #1 mov-vs-push, whole-function
//   was:            register-allocation order, not a logic difference
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
