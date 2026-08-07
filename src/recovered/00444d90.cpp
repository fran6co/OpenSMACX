// 0x00444D90  ?diplo@@YAXHH@Z  ->  ?diplo@@YAXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl diplo(int a1, int a2) {
    reinterpret_cast<DiploWin *>(g_0073acd8)->exec(a1, a2);
}
