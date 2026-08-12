// ORIGINAL: 0x005EDE60 BYTE_EXACT
// name      ?maximize@Win@@QAEHXZ
// size      72 bytes
// spans     0x005EDE60-0x005EDEA8
// prototype int (__thiscall ?maximize@Win@@QAEHXZ)(Win* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EDE83 0x005EDE9D
// 0x005EDE60  ?maximize@Win@@QAEHXZ  ->  ?maximize@Win@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int(__stdcall *ShowWindowFn)(void *, int);

int Win::maximize() {
    void *active = *reinterpret_cast<void **>(g_009b7abc);
    *g_009b7acc = 0;
    *g_009b7ad0 = 0;
    if (active == this) {
        *g_009b7abc = 0;
        reinterpret_cast<VCall *>(this)->slot004();
    }
    if (*reinterpret_cast<void **>(g_009b7ac0) == this) {
        *g_009b7ac0 = 0;
    }
    (*reinterpret_cast<ShowWindowFn *>(g_00669320))(*reinterpret_cast<void **>(g_009b7b28), 3);
    return 0;
}
