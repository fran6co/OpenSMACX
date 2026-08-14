// ORIGINAL: 0x00643950 BYTE_EXACT FILE
// name      sub_643950
// size      49 bytes
// spans     0x00643950-0x00643981
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00642B10
// indirect  0x0064396E 0x00643976
//
// NOT WRITTEN HERE. zlib 1.0.2 `_inflate_blocks_free` from `infblock.c`,
// 49 bytes, byte-exact.
//
// THIS ONE ALREADY HAD A BYTE-EXACT HAND-WRITTEN BODY, and correcting
// the catalogued name from 'sub_643950' to '_inflate_blocks_free' is what
// broke it: the selector looks the subject up BY NAME, and that body
// defined the old one. Both spellings cannot be right, and the bytes
// say which is - so the recovery moves to the source it was always a
// transcription of. The old body was at src/recovered/00643950.cpp.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `infblock.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "infblock.c"
