// ORIGINAL: 0x00643990 BYTE_EXACT FILE
// name      _inflate_trees_bits
// CORRECTED from sub_643990
//   zlib 1.0.2 inftrees.c, 85 bytes, byte-exact from upstream
// size      85 bytes
// spans     0x00643990-0x006439E5
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006439F0 0x00644100
//
// NOT WRITTEN HERE. zlib 1.0.2 `_inflate_trees_bits` from `inftrees.c`,
// 85 bytes, byte-exact.
//
// THIS ONE ALREADY HAD A BYTE-EXACT HAND-WRITTEN BODY, and correcting
// the catalogued name from 'sub_643990' to '_inflate_trees_bits' is what
// broke it: the selector looks the subject up BY NAME, and that body
// defined the old one. Both spellings cannot be right, and the bytes
// say which is - so the recovery moves to the source it was always a
// transcription of. The old body was at src/recovered/00643990.cpp.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `inftrees.c`,
//        compiled as C under `/c /O2 /Gy /GR- /GX`.
//
#include "inftrees.c"
