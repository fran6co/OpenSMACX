// 0x005F0540  ?adjust_menus@Win@@QAGHPAX@Z  ->  ?adjust_menus@Win@@QAGHPAX@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef long (__stdcall *GetWindowLongProc)(void *, int);

int __stdcall Win::adjust_menus(void *a1) {
    Win *obj = reinterpret_cast<Win *>(
        (*reinterpret_cast<GetWindowLongProc *>(g_0066934c))(
            reinterpret_cast<void *>(this), -0x15));
    if (obj != 0) {
        reinterpret_cast<VCall *>(obj)->slot066();
        Menu *menu = obj->menu_;
        if (menu != 0) {
            reinterpret_cast<VCall *>(menu)->slot093();
        }
    }
    return 1;
}
