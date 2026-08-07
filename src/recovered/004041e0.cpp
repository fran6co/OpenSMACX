// 0x004041E0  ?close@AlphaMovie@@QAEXXZ  ->  ?close@AlphaMovie@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void AlphaMovie::close() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<MCIVideo *>(self + 0xa14)->close();
    reinterpret_cast<GraphicWin *>(self)->close();
}
