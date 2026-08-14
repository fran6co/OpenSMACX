// ORIGINAL: 0x0063D1A0 BYTE_EXACT FILE
// name      sub_63d1a0
// size      627 bytes
// spans     0x0063D1A0-0x0063D413
// prototype 
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D420 0x0063D450 0x00640F40 0x00641000
// indirect  0x0063D328
//
// NOT WRITTEN HERE. zlib 1.0.2 `_deflate` from `deflate.c`,
// 627 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63d1a0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/0063d1a0.cpp):
//
//   was: RULED-OUT: this is zlib 1.0.2 deflate() (src/vendor/zlib-1.0.2/deflate.c) -
//   was:            deflate_state field offsets (status 0x4, w_bits 0x24, strstart
//   was:            0x60, level 0x78, hash_size 0x40, head 0x38, lookahead 0x68) and
//   was:            state constants (0x2a/0x71/0x29a = INIT/BUSY/FINISH_STATE) match
//   was:            offsetof()/#define exactly. Transcribed source-form-faithfully
//   was:            with goto's mirroring the two-way FINISH_STATE/avail_in branch;
//   was:            reaches MISMATCH #27 (an early BUF_ERROR return's epilogue pops
//   was:            registers in a different order than the original's
//   was:            mov/pop/pop/mov/pop/mov/pop/ret - a register-allocation detail
//   was:            of that one early exit, not a logic difference) out of ~180
//   was:            instructions. Not chased further - CLEAR_HASH's rep stosd/stosb
//   was:            reproduced cleanly via memset(head,0,(hashSize-1)*sizeof(short)).
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
