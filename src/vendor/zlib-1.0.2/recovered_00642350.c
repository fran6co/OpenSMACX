// ORIGINAL: 0x00642350 BYTE_EXACT FILE
// name      __tr_tally
// CORRECTED from sub_642350
//   zlib 1.0.2 trees.c, 295 bytes, byte-exact from upstream
// size      295 bytes
// spans     0x00642350-0x00642477
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// NOT WRITTEN HERE. zlib 1.0.2 `__tr_tally` from `trees.c`,
// 295 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_642350' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/00642350.cpp):
//
//   was: RULED-OUT: no ecx receiver - all three arguments read from the stack
//   was:            before any push, matching IDA's three-int guess, so a plain
//   was:            `int a1, int a2, int a3` stays a free function over an
//   was:            opaque record pointer. Looks like a Huffman/LZ statistics
//   was:            update: records a (distance, length) or literal pair into a
//   was:            growing history table at 0x1694, bumps one of three
//   was:            frequency tables depending on whether the entry is a literal
//   was:            (distance 0) or a match, then every 4096 entries estimates
//   was:            whether the accumulated tree cost still fits the remaining
//   was:            output budget and signals an early flush. The end-pointer
//   was:            comparison in that scan (`piVar5 < g_00698f94`) uses the
//   was:            scaffold's own fixed-address global rather than a literal, per
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
