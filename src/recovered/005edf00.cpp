// ORIGINAL: 0x005EDF00
// 0x005EDF00  ?show_maximize@Win@@QAEHXZ  ->  ?show_maximize@Win@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// 0x00669320 is the IAT slot for ShowWindow: the original calls through
// the SLOT's address (`call dword ptr [0x669320]`), not a direct relative
// call. `dllimport` tells the compiler the call goes through an import
// thunk, which is exactly this shape - a memory-operand call, no register
// load first.
extern "C" __declspec(dllimport) int __stdcall ShowWindow(void *hWnd,
                                                           int nCmdShow);

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
