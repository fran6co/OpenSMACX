// ORIGINAL: 0x00642960 BYTE_EXACT FILE
// name      sub_642960
// size      139 bytes
// spans     0x00642960-0x006429EB
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// NOT WRITTEN HERE. zlib 1.0.2 `_bi_flush` from `trees.c`,
// 139 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_642960' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/00642960.cpp):
//
//   was: RULED-OUT: the given nullary `int __cdecl sub_642960()` head - the body
//   was:            reads a single stack arg at [esp+4] with no ebp frame (a
//   was:            struct pointer) and never sets eax for a return; changed to
//   was:            `void __cdecl sub_642960(void *a1)`.
//   was:            MISMATCH #8 'push' vs 'mov' remains open.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
