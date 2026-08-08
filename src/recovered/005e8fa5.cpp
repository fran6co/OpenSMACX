// 0x005E8FA5  ?draw_multi_table_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z  ->  ?draw_multi_table_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The 7-arg entry point stashes a5/a6 into two globals (evidently a
// clip-rect or scroll-offset pair some inner drawing path reads), forwards
// (a1, a2, a3, a4, a7) to the class's OWN narrower 5-arg overload of the
// same name, then restores the globals - a save/set/call/restore wrapper.
int Sprite::draw_multi_table_dest(Buffer* a1, int a2, int a3, int a4, int a5, int a6, unsigned char* a7) {
    int save18 = *g_00696d18;
    int save1c = *g_00696d1c;
    *g_00696d18 = a5;
    *g_00696d1c = a6;
    int result = draw_multi_table_dest(a1, a2, a3, a4, a7);
    *g_00696d1c = save1c;
    *g_00696d18 = save18;
    return result;
}
