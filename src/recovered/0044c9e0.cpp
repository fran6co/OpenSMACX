// ORIGINAL: 0x0044C9E0 ?help_project@@YAXH@Z 0x0044C9E0-0x0044C9F8 BYTE_EXACT
// size      24 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
// 0x0044C9E0  ?help_project@@YAXH@Z  ->  ?help_project@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl help_project(int a1) {
    reinterpret_cast<Datalink *>(g_00703ea0)->exec(0xb, a1 + 0x46);
}
