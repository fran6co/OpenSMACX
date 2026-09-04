// ORIGINAL: 0x0045D140 ?on_iface_selected@MainInterface@@QAEXHH@Z 0x0045D140-0x0045D159 BYTE_EXACT
// HANDOFF 2026-09-04: the body is one line - MessageWin::on_selected(a1, a2)
//         on the object at 0x007F67F8 - and messagewin.h declares no
//         on_selected. Add `void on_selected(int index, int a2);` there and
//         home this as IfaceRegistryEntry::on_iface_selected
//         (maininterface.h): the receiver is the SubInterface subobject,
//         which this body never touches, so it is byte-exact either way.
// size      25 bytes
// prototype void (__thiscall ?on_iface_selected@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004718C0
// 0x0045D140  ?on_iface_selected@MainInterface@@QAEXHH@Z  ->  ?on_iface_selected@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MainInterface::on_iface_selected(int a1, int a2) {
    reinterpret_cast<MessageWin *>(g_007f67f8)->on_selected(a1, a2);
}
