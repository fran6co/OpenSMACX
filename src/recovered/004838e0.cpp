// ORIGINAL: 0x004838E0 BYTE_EXACT
// 0x004838E0  ?on_link_clicked@NewTechWin@@QAEHHPAD@Z  ->  ?on_link_clicked@NewTechWin@@QAEHHPAD@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int NewTechWin::on_link_clicked(int a1, char * a2) {
    if (*g_0093f660 == 0 && a2 != 0) {
        *g_00686960 = reinterpret_cast<int (__cdecl *)(char *)>(atoi)(a2);
        reinterpret_cast<VCall *>(this)->slot058();
        return 0;
    }
    return 1;
}
