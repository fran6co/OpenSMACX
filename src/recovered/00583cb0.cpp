// ORIGINAL: 0x00583CB0
// 0x00583CB0  ?design_new_veh@@YAXHH@Z  ->  ?design_new_veh@@YAXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl design_new_veh(int a1, int a2) {
    reinterpret_cast<DesignWin *>(g_0071f2b0)->exec(a1, a2);
}
