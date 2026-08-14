// ORIGINAL: 0x00643FA0 BYTE_EXACT FILE
// name      sub_643fa0
// size      318 bytes
// spans     0x00643FA0-0x006440DE
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006439F0
//
// NOT WRITTEN HERE. zlib 1.0.2 `_inflate_trees_fixed` from `inftrees.c`,
// 318 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_643fa0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/00643fa0.cpp):
//
//   was: RULED-OUT: calling huft_build() directly by name - the catalogue records
//   was:            it unmangled (`_huft_build`) with no CRT_SIGNATURES entry, so
//   was:            declfix leaves it NULLARY and a real call is C2660, while
//   was:            redeclaring it locally is C2733 (second C linkage). Routed the
//   was:            call through a function-pointer cast instead, which compiles
//   was:            but is a source-form change the comparison sees: the callee's
//   was:            8 args are (b=code lengths, n, s, d, e, t, m, hn) matching a
//   was:            custom zlib inflate_trees_fixed()/huft_build() pair, hn shared
//   was:            across both calls at the same stack slot. Not reproduced: a
//   was:            self-referential pointer the original stores at [esp+0x30]
//   was:            before the first fill loop (role unknown - maybe part of an
//   was:            allocator-context struct passed alongside `hn`).
// LEVER: upstream-source the recovery IS zlib 1.0.2 `inftrees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "inftrees.c"
