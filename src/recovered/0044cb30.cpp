// ORIGINAL: 0x0044CB30 BYTE_EXACT
// name      ?help_any@@YAXXZ
// size      15 bytes
// spans     0x0044CB30-0x0044CB3F
// prototype 
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00429180
// 0x0044CB30  ?help_any@@YAXXZ  ->  ?help_any@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl help_any() {
    reinterpret_cast<Datalink *>(g_00703ea0)->exec(0, 0);
}
