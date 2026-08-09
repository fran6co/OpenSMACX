// ORIGINAL: 0x0050F630
// 0x0050F630  sub_50f630  ->  _sub_50f630
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __stdcall sub_50f630(int a1) {
    reinterpret_cast<StatusWin *>(g_008c5568)->redraw();
}
