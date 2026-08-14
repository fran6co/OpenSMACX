// ORIGINAL: 0x00640C30 BYTE_EXACT FILE
// name      sub_640c30
// size      529 bytes
// spans     0x00640C30-0x00640E41
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00640EC0 0x00642940
//
// NOT WRITTEN HERE. zlib 1.0.2 `_tr_static_init` from `trees.c`,
// 529 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_640c30' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/00640c30.cpp):
//
//   was: RULED-OUT: `_length_code[length-1] = 28` as a literal vs `(unsigned char)code`
//   was:            reusing the leaked (VC6) for-loop variable - the leaked-variable
//   was:            spelling moved the first divergence from #37 to #68 and raised
//   was:            mnemonic similarity 95.8%->96.4%, so it is the landed form.
//   was:            Swapping the `base_dist[code2]=dist<<7` vs `count3=1<<(...)`
//   was:            statement order in the third fill loop (#68 still 'jge' vs
//   was:            'mov') did not move it further; not chased past that. This is
//   was:            zlib `trees.c: tr_static_init()` (LENGTH_CODES/D_CODES table
//   was:            fills, the four static_ltree bit-length runs, static_dtree)
//   was:            plus its gen_codes/bi_reverse callees - identified by the
//   was:            gen_codes(static_ltree, 0x11f, bl_count) call matching zlib's
//   was:            `gen_codes((ct_data*)static_ltree, L_CODES+1, bl_count)` exactly.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "trees.c"
