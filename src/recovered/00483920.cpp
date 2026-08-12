// ORIGINAL: 0x00483920 BYTE_EXACT
// 0x00483920  ?info@NewTechWin@@QAAXPAD@Z  ->  ?info@NewTechWin@@QAAXPAD@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl info(char *a1) {
    int index = *g_00686964;
    reinterpret_cast<StringBox *>(reinterpret_cast<char *>(g_00814be4) + index * 11168)->add_special(a1);
}
