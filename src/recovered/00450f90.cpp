// ORIGINAL: 0x00450F90 sub_450f90 0x00450F90-0x00450FB0 BYTE_EXACT
// size      32 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00619710
// 0x00450F90  sub_450f90  ->  _sub_450f90
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl sub_450f90(Texture *a1, int a2, int a3) {
    return a1->extract(reinterpret_cast<Buffer *>(g_00798668), a2, a3, 0x38, 0x38, 0);
}
