// 0x004412D0  ?UNK1@DiploWin@@QAEXXZ  ->  ?UNK1@DiploWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DiploWin::UNK1() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<StringBox *>(self + 0xa388)->clear();
    reinterpret_cast<EditBox *>(self + 0xda7c)->set_text(0);
}
