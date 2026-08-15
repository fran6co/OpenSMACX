// ORIGINAL: 0x005EDF00 BYTE_EXACT
// name      ?show_maximize@Win@@QAEHXZ
// size      72 bytes
// spans     0x005EDF00-0x005EDF48
// prototype int (__thiscall ?show_maximize@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EDF23 0x005EDF3D
// 0x005EDF00  ?show_maximize@Win@@QAEHXZ  ->  ?show_maximize@Win@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// 0x00669320 is the IAT slot for ShowWindow: the original calls through
// the SLOT's address (`call dword ptr [0x669320]`), not a direct relative
// call, so the declaration that makes this body compile carries
// `__declspec(dllimport)` - that is the difference between a memory-operand
// call and a direct one. It used to sit here; the scaffold supplies it now
// (emit_translation_unit.WIN32_IMPORTS), with the same attribute for the
// same reason.

int Win::show_maximize() {
    int active = *g_009b7abc;
    *g_009b7acc = 0;
    *g_009b7ad0 = 0;
    if (active == reinterpret_cast<int>(this)) {
        *g_009b7abc = 0;
        reinterpret_cast<VCall *>(this)->slot004();
    }
    if (*g_009b7ac0 == reinterpret_cast<int>(this)) {
        *g_009b7ac0 = 0;
    }
    ShowWindow(reinterpret_cast<void *>(*g_009b7b28), 3);
    return 0;
}
