// ORIGINAL: 0x00450E30 BYTE_EXACT
// name      sub_450e30
// size      77 bytes
// spans     0x00450E30-0x00450E7D
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005DE580 0x005E39A0
// 0x00450E30  sub_450e30  ->  _sub_450e30
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" int __cdecl sub_450e30(Sprite *a1, int a2, int a3) {
    reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
    reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
    return a1->extract(reinterpret_cast<Buffer *>(g_00798668), 0x109, a2, a3,
                       0x64, 0x3e, 0);
}
