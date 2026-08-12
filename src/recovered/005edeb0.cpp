// ORIGINAL: 0x005EDEB0 BYTE_EXACT
// name      ?minimize@Win@@QAEHXZ
// size      72 bytes
// spans     0x005EDEB0-0x005EDEF8
// prototype int (__thiscall ?minimize@Win@@QAEHXZ)(Win* this)
// callers   1   call targets   0
// 0x005EDEB0  ?minimize@Win@@QAEHXZ  ->  ?minimize@Win@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *ShowWindowFn)(int, int);

int Win::minimize() {
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
    (*reinterpret_cast<ShowWindowFn *>(g_00669320))(*g_009b7b28, 6);
    return 0;
}
