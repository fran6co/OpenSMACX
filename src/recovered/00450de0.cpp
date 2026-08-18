// ORIGINAL: 0x00450DE0 sub_450de0 0x00450DE0-0x00450E2D BYTE_EXACT
// size      77 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005DE580 0x005E39A0
// 0x00450DE0  sub_450de0  ->  _sub_450de0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" int __cdecl sub_450de0(Sprite *spr, int xCoord, int yCoord) {
    Buffer *buf = reinterpret_cast<Buffer *>(g_00798668);
    buf->change_color(0x107, 0x109);
    buf->change_color(0x106, 0x109);
    return spr->extract(buf, 0x109, xCoord, yCoord, 100, 50, 0);
}
