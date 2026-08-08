// 0x0043C170  ?timer_function@DesignWin@@QAAXH@Z  ->  ?timer_function@DesignWin@@QAAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class TutWin { public:
    void draw_arrow();
};

void __cdecl DesignWin::timer_function(int a1) {
    DesignWin *design = *reinterpret_cast<DesignWin **>(g_00733970);
    design->draw_vehicle(0);
    design = *reinterpret_cast<DesignWin **>(g_00733970);
    reinterpret_cast<GraphicWin *>(design)->update(
        reinterpret_cast<RECT *>(reinterpret_cast<char *>(design) + 0xa9c),
        0);
    reinterpret_cast<TutWin *>(g_008c6e68)->draw_arrow();
}
