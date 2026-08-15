// ORIGINAL: 0x00642BB0 BYTE_EXACT FILE
// name      _inflate_blocks_new
// CORRECTED from sub_642bb0
//   zlib 1.0.2 infblock.c, 110 bytes, byte-exact from upstream
// size      110 bytes
// spans     0x00642BB0-0x00642C1E
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00642B10
// indirect  0x00642BBF 0x00642BDA 0x00642BEC
//
// NOT WRITTEN HERE. zlib 1.0.2 `_inflate_blocks_new` from `infblock.c`,
// 110 bytes, byte-exact. IDA did not recognise this function -
// it was catalogued 'sub_642bb0' - and it was found by
// COMPILING every vendored source and matching the emitted symbols
// against the image byte for byte, rather than by its name. See
// PROVENANCE.md beside this file.
//
// SUPERSEDED A HAND-WRITTEN BODY, whose reasoning is kept here
// rather than dropped with the file (was src/unrecovered/00642bb0.cpp):
//
//   was: RULED-OUT: zero-arg contract (disasm reads 3 stack args, so cdecl(int,int,int));
//   was:            redeclaring sub_642b10 with 3 params (C2733, second C linkage) -
//   was:            cast the existing 0-arg decl's address through a function-pointer
//   was:            typedef instead. Register allocation still MISMATCHes the prologue.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `infblock.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "infblock.c"
