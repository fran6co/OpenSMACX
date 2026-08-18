// ORIGINAL: 0x005F04E0 ?close_class@Win@@QAAXXZ 0x005F04E0-0x005F0520 BYTE_EXACT
// size      64 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D7470
// indirect  0x005F04EA 0x005F050F
// 0x005F04E0  ?close_class@Win@@QAAXXZ  ->  ?close_class@Win@@QAAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl Win::close_class() {
    if (*g_009b7b28 != 0) {
        typedef int (__stdcall *DestroyWindowFn)(int);
        (*reinterpret_cast<DestroyWindowFn *>(g_0066930c))(*g_009b7b28);
        *g_009b7b28 = 0;
    }
    reinterpret_cast<Buffer *>(g_009b7490)->close();
    typedef int (__stdcall *UnregisterClassFn)(int, int);
    (*reinterpret_cast<UnregisterClassFn *>(g_006692a4))(
        reinterpret_cast<int>(g_00696dec), *g_009b7b14);
    *g_009b7b14 = 0;
}
