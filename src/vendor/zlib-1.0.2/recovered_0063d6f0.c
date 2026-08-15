// ORIGINAL: 0x0063D6F0 BYTE_EXACT FILE
// name      _fill_window
// CORRECTED from sub_63d6f0
//   zlib 1.0.2 deflate.c, 290 bytes, byte-exact from upstream
// size      290 bytes
// spans     0x0063D6F0-0x0063D812
// prototype
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D820
//
// NOT WRITTEN HERE. zlib 1.0.2 `_fill_window` from `deflate.c`,
// 290 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63d6f0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/0063d6f0.cpp):
//
//   was: RULED-OUT: no ecx receiver - the first stack slot at entry is the sole
//   was:            argument, matching IDA's one-argument guess exactly, so this
//   was:            stays a plain int* over the parameter. This looks like a
//   was:            zlib-style sliding-window deflate fill: fields at 0x18/0x1a/
//   was:            0xc read as strstart/lookahead/window_size, 0xb/0xe/0xd as
//   was:            window/head/prev base pointers, 0x10/0x14/0x19 as further
//   was:            counters, shifted down by the lookahead whenever the window
//   was:            fills to within 0x106 bytes of its end. The one callee
//   was:            (sub_63d820) is declared nullary by the scaffold; calling it
//   was:            through a cast function pointer compiles but turns the
//   was:            original's direct call into an indirect one at that site.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
