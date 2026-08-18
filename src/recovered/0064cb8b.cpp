// ORIGINAL: 0x0064CB8B sub_64cb8b 0x0064CB8B-0x0064CB98 BYTE_EXACT
// size      13 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0064CB91
// 0x0064CB8B  sub_64cb8b  ->  _sub_64cb8b
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Restore the previously installed top-level exception filter.
Original Offset: 0064CB8B
Return Value: n/a
Status: Complete
*/
// The mirror of sub_64cb7a, through the same import slot. The result is
// discarded, which is why this returns void where the catalogue said int.
extern "C" void __cdecl sub_64cb8b() {
    typedef int(__stdcall * FilterFn)(int);
    (reinterpret_cast<FilterFn>(*g_0066922c))(*g_009c06b4);
}
