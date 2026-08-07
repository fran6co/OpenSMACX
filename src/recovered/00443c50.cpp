// 0x00443C50  ?hide@DiploWin@@QAEXXZ  ->  ?hide@DiploWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DiploWin::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<VCall *>(this)->slot058();
    }
}
