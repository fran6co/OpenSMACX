// ORIGINAL: 0x0050DD90 BYTE_EXACT
// name      ?clock_wait@@YAXH@Z
// size      42 bytes
// spans     0x0050DD90-0x0050DDBA
// prototype 
// callers   7   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FCA30 0x005FCBB0
// indirect  0x0050DD9C 0x0050DDAD
// 0x0050DD90  ?clock_wait@@YAXH@Z  ->  ?clock_wait@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl clock_wait(int a1) {
    uint32_t (__cdecl *clock_fn)() =
        reinterpret_cast<uint32_t (__cdecl *)()>(*g_00669368);
    uint32_t started = clock_fn();
    do {
        do_all_draws();
        do_non_input();
    } while (clock_fn() - started < static_cast<uint32_t>(a1));
}
