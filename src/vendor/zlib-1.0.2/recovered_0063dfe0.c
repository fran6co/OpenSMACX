// ORIGINAL: 0x0063DFE0 BYTE_EXACT FILE
// name      sub_63dfe0
// size      270 bytes
// spans     0x0063DFE0-0x0063E0EE
// prototype
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063DF40 0x0063DF90 0x00642BB0
// indirect  0x0063E043
//
// NOT WRITTEN HERE. zlib 1.0.2 `_inflateInit2_` from `inflate.c`,
// 270 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63dfe0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/0063dfe0.cpp):
//
//   was: RULED-OUT: not chased to byte-exact - 86.4% mnemonic agreement (262 vs
//   was:            270 bytes), diverges right after the validation checks. Landed
//   was:            for the field layout: `param_1` is treated as an `int*` record
//   was:            with a function-pointer "allocator" at index 8 (offset 0x20)
//   was:            called cdecl as `(ctx, count, size)`, matching the disassembly
//   was:            exactly; the allocated 0x18-byte record's sign/magnitude/shift
//   was:            fields (indices 3/4/5) match Ghidra's `piVar1[3]`, `[4]`, `[5]`.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `inflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "inflate.c"
