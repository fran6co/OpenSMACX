// 0x0051D7C0  ?on_sys_close@Console@@QAEXXZ  ->  ?on_sys_close@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Record that the console was closed from the system menu.
Original Offset: 0051D7C0
Return Value: n/a
Status: Complete
*/
void Console::on_sys_close() {
    *g_009b2068 = 1;
}
