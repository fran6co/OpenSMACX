// ORIGINAL: 0x00615DF0 BYTE_EXACT
// 0x00615DF0  ?on_dialog_focus@EditBox@@QAEXH@Z  ->  ?on_dialog_focus@EditBox@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void EditBox::on_dialog_focus(int a1) {
    if (a1) {
        reinterpret_cast<Time *>(reinterpret_cast<char *>(this) + 0xB4C)->start();
    } else {
        reinterpret_cast<Time *>(reinterpret_cast<char *>(this) + 0xB4C)->stop();
        field_b48_ = 0;
    }
    reinterpret_cast<VCall *>(this)->slot062();
}
