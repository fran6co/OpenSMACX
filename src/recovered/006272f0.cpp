// ORIGINAL: 0x006272F0 BYTE_EXACT
// name      ?pop_close@@YAXXZ
// size      22 bytes
// spans     0x006272F0-0x00627306
// prototype 
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00600F00
// 0x006272F0  ?pop_close@@YAXXZ  ->  ?pop_close@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl pop_close() {
    reinterpret_cast<BasePop *>(*g_009bc074)->close();
    reinterpret_cast<BasePop *>(*g_009bc078)->close();
}
