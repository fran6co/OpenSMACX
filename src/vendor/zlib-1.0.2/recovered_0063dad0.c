// ORIGINAL: 0x0063DAD0 BYTE_EXACT FILE
// name      _longest_match
// CORRECTED from sub_63dad0
//   zlib 1.0.2 deflate.c, 367 bytes, byte-exact from upstream
// size      367 bytes
// spans     0x0063DAD0-0x0063DC3F
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// NOT WRITTEN HERE. zlib 1.0.2 `_longest_match` from `deflate.c`,
// 367 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_63dad0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/0063dad0.cpp):
//
//   was: RULED-OUT: this is zlib's `longest_match` (deflate.c) - the field
//   was:            offsets match exactly (0x70 max_chain_length, 0x60 strstart,
//   was:            0x6c prev_length, 0x2c window, 0x20 w_size, 0x80 good_match,
//   was:            0x68 nice_match, 0x84 lookahead, 0x64 match_start, 0x34 prev,
//   was:            0x28 w_mask), but the compiled shape differs from the
//   was:            textbook source in two places kept literal here: the loop's
//   was:            break threshold uses `min(lookahead, nice_match)` while the
//   was:            FINAL return clamps against the UNCLAMPED `nice_match` field
//   was:            (not `lookahead`, unlike the well-known zlib source), and the
//   was:            return value itself is a plain match-length int (Ghidra's
//   was:            `char *` typing of the scan/match locals is a mistype from
//   was:            the pointer-arithmetic-as-integer idiom, not a real pointer
// LEVER: upstream-source the recovery IS zlib 1.0.2 `deflate.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "deflate.c"
