// 0x004DA740  ?on_selected@DipEdit@@QAEXH@Z  ->  ?on_selected@DipEdit@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DipEdit::on_selected(int a1) {
    read_check();
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xa20) = a1;
    do_check();
}
