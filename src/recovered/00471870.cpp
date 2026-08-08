// 0x00471870  ?listbox_draw@MessageWin@@QAAHPAUGraphicWin@@HHHHH@Z  ->  ?listbox_draw@MessageWin@@QAAHPAUGraphicWin@@HHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl MessageWin::listbox_draw(GraphicWin * a1, int a2, int a3, int a4, int a5, int a6) {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = (self != 0) ? reinterpret_cast<Buffer *>(self + 0x444) : 0;
    return reinterpret_cast<Sprite *>(g_007f67c8)->draw(
        buf, *g_007f67d0 & 0xff, a3 + 4, (a5 - *g_007f67e4) / 2 + a4, 1, 1);
}
