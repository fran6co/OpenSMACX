// ORIGINAL: 0x0050DD90 BYTE_EXACT
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
