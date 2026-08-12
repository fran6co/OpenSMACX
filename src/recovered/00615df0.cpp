// ORIGINAL: 0x00615DF0 BYTE_EXACT
// name      ?on_dialog_focus@EditBox@@QAEXH@Z
// size      65 bytes
// spans     0x00615DF0-0x00615E31
// prototype void (__thiscall ?on_dialog_focus@EditBox@@QAEXH@Z)(EditBox* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616650 0x00616730
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
