// 0x0051D740  ?menu_update@Console@@QAEXXZ  ->  ?menu_update@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Console::menu_update() {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0x23bdc);
    reinterpret_cast<MainMenu *>(self + 0x22a2c)->check(v);
}
