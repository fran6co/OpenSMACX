// ORIGINAL: 0x0045D2A0 BYTE_EXACT
// name      ?set_alert@MainInterface@@QAEXH@Z
// size      49 bytes
// spans     0x0045D2A0-0x0045D2D1
// prototype void (__thiscall ?set_alert@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   1   call targets   1
// 0x0045D2A0  ?set_alert@MainInterface@@QAEXH@Z  ->  ?set_alert@MainInterface@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MainInterface::set_alert(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 != 0) {
        *reinterpret_cast<int *>(self + 0x216f0) = 0;
        reinterpret_cast<Time *>(self + 0x25068)->start();
        return;
    }
    *reinterpret_cast<int *>(self + 0x216f0) = -1;
}
