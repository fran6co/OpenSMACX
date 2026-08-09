// ORIGINAL: 0x0045D140
// 0x0045D140  ?on_iface_selected@MainInterface@@QAEXHH@Z  ->  ?on_iface_selected@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MainInterface::on_iface_selected(int a1, int a2) {
    reinterpret_cast<MessageWin *>(g_007f67f8)->on_selected(a1, a2);
}
