// 0x0060E300  ?on_dialog_focus@RadioButton@@QAEXH@Z  ->  ?on_dialog_focus@RadioButton@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void RadioButton::on_dialog_focus(int a1) {
    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) - 0x18 +
        *reinterpret_cast<int *>(
            *reinterpret_cast<char **>(reinterpret_cast<char *>(this) - 0x18) + 4)
    )->slot062();
}
