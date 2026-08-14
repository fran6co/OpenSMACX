// ORIGINAL: 0x006429F0 BYTE_EXACT FILE
// name      sub_6429f0
// size      124 bytes
// spans     0x006429F0-0x00642A6C
// prototype 
// callers   2   call targets   0
// calls     (none)
//
// NOT WRITTEN HERE. zlib 1.0.2 `_bi_windup` from `trees.c`,
// 124 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_6429f0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/006429f0.cpp):
//
//   was: RULED-OUT: catalogued as nullary returning int; the disassembly reads a
//   was:            real stack argument at [esp+4], so re-signatured as one
//   was:            `int param_1` parameter, `void` return (see also
//   was:            0x0063D450). Tried: caching the byte-write index in a local
//   was:            (`int idx = *(int*)(param_1+0x10)+1;`, matching Ghidra's
//   was:            iVar1 literally) - worse (119B); no caching at all, every
//   was:            field re-read fresh through nested `*(int*)(...)` each time -
//   was:            best so far (121B vs 124); caching the buffer base pointer
//   was:            once as `unsigned char *buf` - worse (116B, divergence moves
//   was:            earlier); caching `count = *(int*)(param_1+0x16b4)` once and
//   was:            reusing it in both the `8 <` and `0 <` guards, matching the
//   was:            original's single register load - no change; two named
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
