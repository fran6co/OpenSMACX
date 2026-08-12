// ORIGINAL: 0x0054DC90 BYTE_EXACT
// 0x0054DC90  sub_54dc90  ->  _sub_54dc90
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

static char *const g_0097d040 = (char *)0x0097D040;
static char *const g_0097d042 = (char *)0x0097D042;

extern "C" void __cdecl sub_54dc90(int a1) {
    reinterpret_cast<PlanWin *>(g_00834d70)->clear_lines();
    int offset = a1 * 0x134;
    reinterpret_cast<PlanWin *>(g_00834d70)->monocenter(
        *reinterpret_cast<short *>(g_0097d040 + offset),
        *reinterpret_cast<short *>(g_0097d042 + offset),
        0);
    reinterpret_cast<PlanWin *>(g_00834d70)->add_line(a1, a1, 0);
    reinterpret_cast<PlanWin *>(g_00856dc0)->on_redraw();
}
