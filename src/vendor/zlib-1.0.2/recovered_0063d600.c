// ORIGINAL: 0x0063D600 BYTE_EXACT FILE
// name      sub_63d600
// size      239 bytes
// spans     0x0063D600-0x0063D6EF
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D450 0x0063D6F0 0x00641280
//
// NOT WRITTEN HERE. zlib 1.0.2 `_deflate_stored` from `deflate.c`,
// 239 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63d600' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/0063d600.cpp):
//
//   was: RULED-OUT: nothing yet - MISMATCH #1 mov/push (74% mnemonic sim). Register
//   was:            pre-load order (param_2 into a saved reg before param_1) shifts
//   was:            the prologue; not chased further. LEVER: calling the arity-0
//   was:            scaffold declarations of the three unnamed callees through a
//   was:            reinterpret-cast function-pointer typedef compiles and passes
//   was:            real arguments, where calling them directly is locked to 0 args.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
