// ORIGINAL: 0x0048D650 sub_48d650 0x0048D650-0x0048D66E BYTE_EXACT
// size      30 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0048D650  sub_48d650  ->  _sub_48d650
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl sub_48d650() {
    *g_008578a8 = *g_008578ac - *g_00686b74 * 3 - *g_00686b84;
}
