// ORIGINAL: 0x004243D0 BYTE_EXACT
// name      ?clear_council_notify@@YAXH@Z
// size      24 bytes
// spans     0x004243D0-0x004243E8
// prototype 
// callers   1   call targets   0
// 0x004243D0  ?clear_council_notify@@YAXH@Z  ->  ?clear_council_notify@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl clear_council_notify(int a1) {
    g_00703dec[a1] = 0;
    g_00703e0c[a1] = 0;
}
