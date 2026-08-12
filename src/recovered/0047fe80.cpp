// ORIGINAL: 0x0047FE80 BYTE_EXACT
// name      ?UNK3@NetWin@@QAEXXZ
// size      70 bytes
// spans     0x0047FE80-0x0047FEC6
// prototype void (__thiscall ?UNK3@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   1
// 0x0047FE80  ?UNK3@NetWin@@QAEXXZ  ->  ?UNK3@NetWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void NetWin::UNK3() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    buf->box_sprite(reinterpret_cast<RECT *>(self + 0xce4),
                    reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));
    buf->box_sprite(reinterpret_cast<RECT *>(self + 0xcd4),
                    reinterpret_cast<BoxSpriteParams *>(g_0078d528));
    buf->box_sprite(reinterpret_cast<RECT *>(self + 0xcc4),
                    reinterpret_cast<BoxSpriteParams *>(g_0078d528));
}
