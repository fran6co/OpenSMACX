// 0x00402F30  ??__Fg_ALPHAMENU_WAVE@@YAXXZ  ->  ??__Fg_ALPHAMENU_WAVE@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class Wave {
public:
    ~Wave();
};

static Wave *const g_ALPHAMENU_WAVE = (Wave *)0x006A7090;

void __cdecl fn_00402f30() {
    g_ALPHAMENU_WAVE->~Wave();
}
