// ORIGINAL: 0x0043D0F0 BYTE_EXACT
// name      ?hide@DesignWin@@QAEXXZ
// size      38 bytes
// spans     0x0043D0F0-0x0043D116
// prototype void (__thiscall ?hide@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00446B40 0x0045D380 0x005F7E90
// 0x0043D0F0  ?hide@DesignWin@@QAEXXZ  ->  ?hide@DesignWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DesignWin::hide() {
    char *self = reinterpret_cast<char *>(this);
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(self + 0xa14)->release_iface_mode();
        ambience(0xd4);
    }
}
