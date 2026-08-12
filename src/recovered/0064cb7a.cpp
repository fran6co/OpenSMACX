// ORIGINAL: 0x0064CB7A BYTE_EXACT
// name      sub_64cb7a
// size      17 bytes
// spans     0x0064CB7A-0x0064CB8B
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0064CB7F
// 0x0064CB7A  sub_64cb7a  ->  _sub_64cb7a
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Install the top-level exception filter and remember the previous one.
Original Offset: 0064CB7A
Return Value: n/a
Status: Complete
*/
// Called through the import slot at 0x0066922C rather than by name: this is a
// raw indirect call through a fixed address, so it is not a call-graph edge
// and the scaffolding declares no callee for it.
extern "C" void __cdecl sub_64cb7a() {
    typedef int(__stdcall * FilterFn)(int);
    *g_009c06b4 = (reinterpret_cast<FilterFn>(*g_0066922c))(
        reinterpret_cast<int>(g_0064cb34));
}
