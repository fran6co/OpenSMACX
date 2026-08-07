// 0x004C3EA0  ?follow_center@WorldWin@@QAEXHH@Z  ->  ?follow_center@WorldWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void WorldWin::follow_center(int xCoord, int yCoord) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<VCall *>(self)->slot012();
    reinterpret_cast<GraphicWin *>(self)->soft_update();
}
