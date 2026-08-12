// ORIGINAL: 0x0041E8B0 BYTE_EXACT
// name      ?hide@BaseWin@@QAEXXZ
// size      25 bytes
// spans     0x0041E8B0-0x0041E8C9
// prototype void (__thiscall ?hide@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   2
// 0x0041E8B0  ?hide@BaseWin@@QAEXXZ  ->  ?hide@BaseWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// `sub_interface_` was a member of a BaseWin layout the emitter no longer
// produces - it emits `pad_0_[0x40B10]` and one field - so the name had gone
// undeclared and this scored NO_COMPILE. Reaching the sub-object by its
// offset says the same thing without depending on a layout that moved:
// BaseWin embeds the SubInterface at +0xA14, which src/basewin.h declares as
// `subIFace_` and commit beb8e72 measured byte-exact for five classes.
void BaseWin::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(
            reinterpret_cast<char *>(this) + 0xA14)->release_iface_mode();
    }
}
